#include "mu_precompiled.h"
#include "mu_updatemanager.h"
#include "upd_retrieveservers.h"
#include "mu_webmanager.h"
#include "web_filedownload.h"
#include "mu_graphics.h"

#include <boost/regex.hpp>
#include <glm/gtc/random.hpp>

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
#include "ui_noesisgui.h"
#include "ngui_context.h"
#endif

#ifndef NEXTMU_UPDATE_ENABLED
#define NEXTMU_UPDATE_ENABLED 1
#endif

#ifndef NEXTMU_UPDATE_SERVER_PROTOCOL
#define NEXTMU_UPDATE_SERVER_PROTOCOL "http"
#endif

#ifndef NEXTMU_UPDATE_SERVER_URL
#define NEXTMU_UPDATE_SERVER_URL "localhost:8110"
#endif

#include <fifo_map.hpp>
template<class K, class V, class dummy_compare, class A>
using wafifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using ujson = nlohmann::basic_json<wafifo_map>;

namespace EFileBitset
{
	enum : mu_uint32
	{
		eSkipped = (1 << 0),
		eDownloading = (1 << 1),
		eFinished = (1 << 2),
	};
};

class NFileData
{
public:
	NFileData()
	{
		Clear();
	}

	NEXTMU_INLINE void Clear()
	{
		UrlPath.clear();
		Filename.clear();
		Extension.clear();
		LocalPath.clear();
		DownloadSize = 0;
		FileSize = 0;
		CRC32 = 0;
		Bitset = 0;
	}

public:
	mu_utf8string UrlPath;
	mu_utf8string Filename;
	mu_utf8string Extension;
	mu_utf8string LocalPath;
	mu_uint32 DownloadSize;
	mu_uint32 FileSize;
	mu_uint32 CRC32;
	mu_uint32 Bitset;
};

namespace MUUpdateManager
{
	constexpr mu_uint32 VerifyingThreadsCount = 2u;
	const mu_char *GameVersionFilename = "assets.ver";
	const mu_char *SizeTypes[] =
	{
		"B",
		"KB",
		"MB",
		"GB",
	};

	mu_atomic_bool Terminate = false;
	mu_boolean Paused = false;
	NUpdateState State = NUpdateState::Initialize;
	std::vector<WEBRequestBasePtr> Requests;
	std::vector<mu_utf8string> ServersList;
	mu_utf8string UpdateServer;
	mu_utf8string UpdateGameVersion;
	std::vector<std::thread> Threads;
	mu_atomic_uint32_t FinishedThreadsCount;
	mu_atomic_uint32_t VerifiedCount;
	mu_atomic_uint32_t ValidFilesCount(0u);
	mu_uint32 MaxDownloadFiles = 0u;
	mu_size TotalDownloadSize = 0;
	mu_atomic_uint64_t TotalDownloadedSize = 0;
	mu_uint32 TotalDownloadCount = 0u;
	mu_uint32 TotalCompletedCount = 0u;
	mu_double TotalDownloadSizeFormatted = 0.0;
	mu_uint32 TotalDownloadSizeType = 0u;

	typedef std::shared_ptr<NFileData> NFileDataPtr;
	std::vector<NFileDataPtr> FilesList;
	std::vector<NFileDataPtr> FilesToDownloadList;

	void DestroyThreads()
	{
		for (auto &thread : Threads)
		{
			if (thread.joinable()) thread.join();
		}
		Threads.clear();
	}

	const mu_boolean Initialize()
	{
		constexpr mu_int32 MinParallelDownloadFiles = 4;
		constexpr mu_int32 MaxParallelDownloadFiles = 64;

		try
		{
			const mu_int32 systemRam = SDL_GetSystemRAM();
			MaxDownloadFiles = glm::max(
				glm::min(
					static_cast<mu_size>((systemRam / 2) / 32), // Half of memory in MB divided by 32MB
					static_cast<mu_size>(MaxParallelDownloadFiles)
				),
				static_cast<mu_size>(MaxParallelDownloadFiles)
			);
		}
		catch (const std::exception &ex)
		{
			MaxDownloadFiles = static_cast<mu_size>(MinParallelDownloadFiles);
		}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		auto *context = UINoesis::GetContext()->GetUpdate();
		context->SetProgress(0.0f);
		context->SetState("Initialize");
#endif

		return true;
	}

	void Destroy()
	{
		Terminate = true;
		for (auto &request : Requests)
		{
			MUWebManager::AbortRequest(request);
		}
		Requests.clear();
		ServersList.clear();
		DestroyThreads();
	}

	enum class NUpdateTextureFormat
	{
		eUncompressed,
		eBC3,
		eBC7,
		eETC2,
		eASTC,
	};

	const NUpdateTextureFormat GetCompatibleTextureFormat()
	{
		auto device = MUGraphics::GetDevice();

		// Not supported yet
		/*if (device->GetTextureFormatInfo(Diligent::TEXTURE_FORMAT::TEX_FORMAT_ASTC_UNORM).Supported)
			return NUpdateTextureFormat::eASTC;*/
		
		// Not supported yet
		/*if (device->GetTextureFormatInfo(Diligent::TEXTURE_FORMAT::TEX_FORMAT_ETC2_UNORM).Supported)
			return NUpdateTextureFormat::eETC2;*/

		if (device->GetTextureFormatInfo(Diligent::TEXTURE_FORMAT::TEX_FORMAT_BC7_UNORM).Supported)
			return NUpdateTextureFormat::eBC7;

		if (device->GetTextureFormatInfo(Diligent::TEXTURE_FORMAT::TEX_FORMAT_BC3_UNORM).Supported)
			return NUpdateTextureFormat::eBC3;

		return NUpdateTextureFormat::eUncompressed;
	}

	void VerifyFilesWorker(const mu_uint32 threadIndex)
	{
		const mu_size filesCount = FilesList.size();

		constexpr mu_int64 maxChunkSize = 512 * 1024; // 512KB
		EReadMemoryBuffer memoryBuffer;
		memoryBuffer.Reset(maxChunkSize);

		for (mu_size j = threadIndex; j < filesCount && !Terminate; j += VerifyingThreadsCount)
		{
			NFileDataPtr &fileData = FilesList[j];
			SDL_RWops *file = nullptr;

			if (mu_rwfromfile<EGameDirectoryType::eSupport>(&file, fileData->LocalPath.c_str(), "rb") == true)
			{
				const mu_int64 fileLength = SDL_RWsize(file);
				mu_uint32 fileCRC32 = 0;
				if (fileLength > 0)
				{
					mu_int64 dataLength = fileLength;
					do
					{
						const mu_int64 chunkSize = dataLength > maxChunkSize ? maxChunkSize : dataLength;
						const mu_int64 readSize = SDL_RWread(file, memoryBuffer.GetBuffer(), chunkSize, 1);
						if (readSize > 0)
						{
							fileCRC32 = scalopus::crcdetail::compute(reinterpret_cast<mu_char*>(memoryBuffer.GetBuffer()), static_cast<mu_uint32>(chunkSize), fileCRC32);
							dataLength -= chunkSize;
						}
					} while (dataLength > 0);

				}

				SDL_RWclose(file);

				if (fileData->CRC32 == fileCRC32)
				{
					++ValidFilesCount;
					fileData->Bitset |= EFileBitset::eSkipped;
				}
			}

			++VerifiedCount;
		}

		++FinishedThreadsCount;
	}

	enum class EProcessResult
	{
		eSuccess = 0,
		eNoMemoryAvailable,
		eDecompressFailed,
		eStorageFull,
		eCorruptedFile,
		eUnknown,
	};
	const EProcessResult ProcessDownloadedFile(WEBFileDownloadRequest *request, NFileData *fileData)
	{
		const mu_size bufferSize = request->GetBufferSize();
		mu_uint8 *buffer = request->GetBuffer();

		if (bufferSize != fileData->DownloadSize)
		{
			mu_error("Downlaoded file size is wrong");
			return EProcessResult::eCorruptedFile;
		}

		if (bufferSize == 0) return EProcessResult::eSuccess;

		mu_boolean isCompressed = false;
		if (fileData->Extension.compare(".eupdz") == 0)
		{
			isCompressed = true;
		}

		if (isCompressed == true)
		{
			std::unique_ptr<mu_uint8[]> decompressBuffer(new_nothrow mu_uint8[fileData->FileSize]);

			if (DecompressData(buffer, decompressBuffer.get(), bufferSize, fileData->FileSize) == false)
			{
				return EProcessResult::eDecompressFailed;
			}

			const mu_size slashIndex = fileData->LocalPath.find_last_of('/');
			if (slashIndex != mu_utf8string::npos)
			{
				MakeDirectory<EGameDirectoryType::eSupport>(fileData->LocalPath.substr(0, slashIndex));
			}

			SDL_RWops *file = nullptr;
			if (mu_rwfromfile<EGameDirectoryType::eSupport>(&file, fileData->LocalPath.c_str(), "wb") == false)
			{
				mu_error("Failed to open file to update");
				return EProcessResult::eUnknown;
			}

			const mu_size writtenCount = SDL_RWwrite(file, decompressBuffer.get(), fileData->FileSize, 1);
			SDL_RWclose(file);

			if (writtenCount == 0)
			{
				return EProcessResult::eStorageFull;
			}
		}
		else
		{
			const mu_size slashIndex = fileData->LocalPath.find_last_of('/');
			if (slashIndex != mu_utf8string::npos)
			{
				MakeDirectory<EGameDirectoryType::eSupport>(fileData->LocalPath.substr(0, slashIndex));
			}

			SDL_RWops *file = nullptr;
			if (mu_rwfromfile<EGameDirectoryType::eSupport>(&file, fileData->LocalPath.c_str(), "wb") == false)
			{
				mu_error("Failed to open file to update");
				return EProcessResult::eUnknown;
			}

			const mu_size writtenCount = SDL_RWwrite(file, buffer, bufferSize, 1);
			SDL_RWclose(file);

			if (writtenCount == 0)
			{
				return EProcessResult::eStorageFull;
			}
		}

		return EProcessResult::eSuccess;
	}

	void RequestFailedCallback(const WEBFileDownloadRequest *request)
	{
		NFileData *fileData = reinterpret_cast<NFileData *>(request->GetUserData());
		fileData->Bitset &= ~EFileBitset::eDownloading;
	}

	void RequestAbortedCallback(const WEBFileDownloadRequest *request)
	{
		NFileData *fileData = reinterpret_cast<NFileData *>(request->GetUserData());
		fileData->Bitset &= ~EFileBitset::eDownloading;
	}

	void ReceivedDataCallback(const mu_size dataLength)
	{
		TotalDownloadedSize += dataLength;
	}

	void WriteVersion(const mu_boolean requireVerify)
	{
		ujson document;

		document["GameVersion"] = UpdateGameVersion;
		document["TextureType"] = static_cast<mu_uint32>(GetCompatibleTextureFormat());
		document["RequireVerify"] = requireVerify;

		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, GameVersionFilename, "wb") == false)
		{
			return;
		}

		const mu_utf8string jsonData = document.dump(1, '\t');
		SDL_RWwrite(fp, jsonData.data(), jsonData.size(), 1);
		SDL_RWclose(fp);
	}

	mu_boolean Run()
	{
		if (Paused) return false;

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
		auto *context = UINoesis::GetContext()->GetUpdate();
#endif

		switch (State)
		{
		case NUpdateState::Initialize:
			{
#if NEXTMU_UPDATE_ENABLED == 1
				State = NUpdateState::WaitingServers;
#else
				State = NUpdateState::Finished;
#endif
			}
			break;

		case NUpdateState::WaitingServers:
			{
				if (Requests.empty() == true)
				{
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
					context->SetState("WaitingServers");
#endif

					auto systemTime = std::chrono::system_clock::now();
					auto request = std::make_shared<WEBFileDownloadRequest>(fmt::format("{}://{}/api/v1/updates/servers/list/{}", NEXTMU_UPDATE_SERVER_PROTOCOL, NEXTMU_UPDATE_SERVER_URL, std::chrono::system_clock::to_time_t(systemTime)));
					if (MUWebManager::AddRequest(request) != CURLM_OK)
					{
						// UIMessageSystem::RequestMessage<EMessageBoxID::eWebManagerRequestFailed>();
						Pause();
						break;
					}

					Requests.push_back(std::move(request));
				}
				else
				{
					WEBFileDownloadRequest *request = static_cast<WEBFileDownloadRequest*>(Requests[0].get());
					auto requestState = request->GetRequestState();
					auto responseCode = request->GetResponseCode();

					switch (requestState)
					{
					case WEBRequestState::Completed:
						{
							if (IsSuccessful(responseCode) == false)
							{
								mu_debug_error("retrieve server list invalid response code : {}", responseCode);
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateServerListFailed>();
								Pause();
								break;
							}

							const mu_size bufferSize = request->GetBufferSize();
							mu_uint8 *buffer = request->GetBuffer();
							auto document = nlohmann::json::parse(buffer, buffer + bufferSize);

							if (document.is_discarded() == true)
							{
								mu_debug_error("Server list is corrupted");
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateFileCorrupted>();
								Pause();
								break;
							}

							if (document.contains("servers") == false)
							{
								mu_debug_error("Server list is empty");
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateFileCorrupted>();
								Pause();
								break;
							}

							const auto servers = document["servers"];
							const auto serversCount = servers.size();
							if (serversCount < 1)
							{
								mu_debug_error("Server list is empty");
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateNoServerAvailable>();
								Pause();
								break;
							}

							ServersList.clear();
							for (mu_uint32 n = 0; n < serversCount; ++n)
							{
								ServersList.push_back(servers[n].get<mu_utf8string>());
							}

							State = NUpdateState::WaitingFilesList;

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
							context->SetProgress(1.0f);
#endif
						}
						break;

					case WEBRequestState::Failed:
						{
							mu_debug_error("failed to retrieve server list");
							// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateServerListFailed>();
							Pause();
						}
						break;

					case WEBRequestState::Aborted:
						{
							Pause();
						}
						break;
					}

					switch (requestState)
					{
					case WEBRequestState::Completed:
					case WEBRequestState::Failed:
					case WEBRequestState::Aborted:
						Requests.clear();
						break;
					}
				}
			}
			break;

		case NUpdateState::WaitingFilesList:
			{
				static mu_utf8string GameVersion;
				static mu_uint32 TextureCompatible;

				if (GameVersion.size() == 0)
				{
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
					context->SetProgress(0.0f);
					context->SetState("WaitingFilesList");
#endif

					UpdateServer = ServersList[glm::linearRand(static_cast<mu_int32>(0), glm::max(static_cast<mu_int32>(ServersList.size()) - 1, 0))];
					GameVersion = "0.0.0";

					SDL_RWops *file = nullptr;
					if (mu_rwfromfile<EGameDirectoryType::eSupport>(&file, GameVersionFilename, "rb") == true)
					{
						const mu_int64 fileLength = SDL_RWsize(file);
						EReadMemoryBuffer memoryBuffer;
						memoryBuffer.Reset(fileLength);
						if (memoryBuffer.GetBuffer() == nullptr)
						{
							mu_debug_error("Files list failed to allocate memory [memoryBuffer]");
							// UIMessageSystem::RequestMessage<EMessageBoxID::eSystemError>();
							Pause();
							return false;
						}

						SDL_RWread(file, memoryBuffer.GetBuffer(), fileLength, 1);
						SDL_RWclose(file);

						auto document = nlohmann::json::parse(memoryBuffer.GetBuffer(), memoryBuffer.GetBuffer() + fileLength);
						if (document.is_discarded() == false)
						{
							if (document.contains("GameVersion") == true &&
								document.contains("TextureType") == true)
							{
								TextureCompatible = document["TextureType"].get<mu_uint32>();

								mu_boolean requireVerify = false;
								if (document.contains("RequireVerify") == true)
								{
									requireVerify = document["RequireVerify"].get<mu_boolean>();
								}

								if (requireVerify == false &&
									TextureCompatible == static_cast<mu_uint32>(GetCompatibleTextureFormat()))
								{
									const mu_utf8string version = document["GameVersion"].get<mu_utf8string>();
									boost::regex versionRegex("^(\\d){1,2}\\.(\\d){1,2}\\.(\\d){1,2}$");
									if (boost::regex_match(version, versionRegex) == true)
									{
										GameVersion = version;
									}
								}
							}
						}
					}
				}

				if (Requests.empty() == true)
				{
					auto systemTime = std::chrono::system_clock::now();
					auto request = std::make_shared<WEBFileDownloadRequest>(fmt::format("{}://{}/api/v1/updates/list/{}/{}/{}/{}", NEXTMU_UPDATE_SERVER_PROTOCOL, NEXTMU_UPDATE_SERVER_URL, GameVersion, NEXTMU_OPERATING_SYSTEM, static_cast<mu_uint32>(GetCompatibleTextureFormat()), std::chrono::system_clock::to_time_t(systemTime)));
					if (MUWebManager::AddRequest(request) != CURLM_OK)
					{
						// UIMessageSystem::RequestMessage<EMessageBoxID::eWebManagerRequestFailed>();
						Pause();
						break;
					}

					Requests.push_back(std::move(request));
				}
				else
				{
					WEBFileDownloadRequest *request = static_cast<WEBFileDownloadRequest *>(Requests[0].get());
					auto requestState = request->GetRequestState();
					auto responseCode = request->GetResponseCode();

					switch (requestState)
					{
					case WEBRequestState::Completed:
						{
							if (IsSuccessful(responseCode) == false)
							{
								mu_debug_error("retrieve files list invalid response code : {}", responseCode);
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateServerListFailed>();
								Pause();
								break;
							}

							const mu_size bufferSize = request->GetBufferSize();
							mu_uint8 *buffer = request->GetBuffer();
							auto document = nlohmann::json::parse(buffer, buffer + bufferSize);

							if (document.is_discarded() == true)
							{
								mu_debug_error("File list is corrupted");
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateFileCorrupted>();
								Pause();
								return false;
							}

							if (document.contains("version") == false)
							{
								mu_debug_error("File list does't contain version");
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateFileCorrupted>();
								Pause();
								return false;
							}

							UpdateGameVersion = document["version"].get<mu_utf8string>();

							if (document.contains("files") == true)
							{
								const auto &files = document["files"];
								const mu_size filesCount = files.size();
								FilesList.resize(filesCount);
								for (mu_size n = 0; n < filesCount; ++n)
								{
									const auto &fileInfo = files[n];
									auto &fileData = FilesList[n];
									fileData.reset(new_nothrow NFileData());

									fileData->UrlPath = fileInfo["UrlPath"].get<mu_utf8string>();
									fileData->Filename = fileInfo["Filename"].get<mu_utf8string>();
									fileData->Extension = fileInfo["Extension"].get<mu_utf8string>();
									fileData->LocalPath = fileInfo["LocalPath"].get<mu_utf8string>();
									std::replace(fileData->LocalPath.begin(), fileData->LocalPath.end(), '\\', '/');
									fileData->DownloadSize = fileInfo["PackedSize"].get<mu_uint32>();
									fileData->FileSize = fileInfo["OriginalSize"].get<mu_uint32>();
									fileData->CRC32 = std::strtoul(fileInfo["CRC32"].get<mu_utf8string>().c_str(), nullptr, 16);
									fileData->Bitset = 0;
								}
							}

							if (FilesList.size() > 0)
							{
								State = NUpdateState::VerifyingFiles;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
								context->SetState("VerifyingFiles");
#endif
							}
							else if (GameVersion.compare(UpdateGameVersion) == 0)
							{
								State = NUpdateState::Finished;
							}
							else
							{
								State = NUpdateState::WritingVersion;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
								context->SetState("WritingVersion");
#endif
							}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
							context->SetProgress(1.0f);
#endif
						}
						break;

					case WEBRequestState::Failed:
						{
							mu_debug_error("failed to retrieve files list");
							// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateServerListFailed>();
							Pause();
						}
						break;

					case WEBRequestState::Aborted:
						{
							Pause();
						}
						break;
					}

					switch (requestState)
					{
					case WEBRequestState::Completed:
					case WEBRequestState::Failed:
					case WEBRequestState::Aborted:
						Requests.clear();
						break;
					}
				}
			}
			break;

		case NUpdateState::VerifyingFiles:
			{
				const mu_size filesCount = FilesList.size();
				if (Threads.empty() == true)
				{
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
					context->SetState("VerifyingFiles");
					context->SetVerifyCount(filesCount);
					context->SetVerifiedCount(0u);
#endif

					FinishedThreadsCount.store(0, std::memory_order_relaxed);
					VerifiedCount.store(0, std::memory_order_relaxed);
					Threads.resize(VerifyingThreadsCount);
					for (mu_uint32 n = 0; n < VerifyingThreadsCount; ++n)
					{
						Threads[n] = std::thread(VerifyFilesWorker, n);
					}
				}
				else if (FinishedThreadsCount >= VerifyingThreadsCount)
				{
					DestroyThreads();

					TotalDownloadSize = 0;
					mu_size downloadCount = FilesList.size();
					for (auto iter = FilesList.begin(); iter != FilesList.end(); ++iter)
					{
						auto &fileData = *iter;
						if ((fileData->Bitset & EFileBitset::eSkipped) != 0)
						{
							--downloadCount;
						}
						else
						{
							FilesToDownloadList.push_back(fileData);
							TotalDownloadSize += fileData->DownloadSize;
						}
					}

					TotalDownloadSizeFormatted = TotalDownloadSize;
					while (TotalDownloadSizeFormatted > 1024.0 &&
						TotalDownloadSizeType < mu_countof(SizeTypes) - 1)
					{
						TotalDownloadSizeFormatted /= 1024.0;
						++TotalDownloadSizeType;
					}

					if (downloadCount > 0)
					{
						TotalDownloadedSize = 0;
						TotalDownloadCount = downloadCount;
						TotalCompletedCount = 0;
						State = NUpdateState::UpdatingFiles;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
						context->SetState("UpdatingFiles");
#endif
					}
					else
					{
						State = NUpdateState::WritingVersion;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
						context->SetState("WritingVersion");
#endif
					}
				}

				const mu_uint32 verifiedCount = VerifiedCount;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
				if (State == NUpdateState::VerifyingFiles)
				{
					context->SetProgress(static_cast<mu_float>(verifiedCount) / static_cast<mu_float>(filesCount));
					context->SetVerifiedCount(verifiedCount);
				}
				else
				{
					context->SetProgress(0.0f);
					context->SetDownloadSizeType(SizeTypes[TotalDownloadSizeType]);
					context->SetDownloadSize(TotalDownloadSizeFormatted);
				}
#endif
			}
			break;

		case NUpdateState::UpdatingFiles:
			{
				enum class EProcessingState
				{
					eSuccess,
					eAborted,
					eCurlFailed,
					eCurlMultiFailed,
					eServerFailed,
					eProcessFailed,
				};
				EProcessingState state = EProcessingState::eSuccess;
				union
				{
					CURLcode curlCode;
					CURLMcode curlmCode;
					EProcessResult processResult;
					mu_long errorCode;
				};

				for (auto iter = Requests.begin(); iter != Requests.end(); ++iter)
				{
					WEBFileDownloadRequest *request = static_cast<WEBFileDownloadRequest *>(iter->get());
					NFileData *fileData = reinterpret_cast<NFileData*>(request->GetUserData());
					auto requestState = request->GetRequestState();
					auto responseCode = request->GetResponseCode();

					switch (requestState)
					{
					case WEBRequestState::Completed:
						{
							if (IsSuccessful(responseCode) == false)
							{
								state = EProcessingState::eServerFailed;
								errorCode = responseCode;
								break;
							}
							
							const auto result = ProcessDownloadedFile(request, fileData);
							if (result == EProcessResult::eSuccess)
							{
								fileData->Bitset |= EFileBitset::eFinished;
								++TotalCompletedCount;
							}
							else
							{
								fileData->Bitset &= ~EFileBitset::eDownloading;
								state = EProcessingState::eProcessFailed;
								processResult = result;
							}
						}
						break;

					case WEBRequestState::Failed:
						{
							state = EProcessingState::eCurlFailed;
							curlCode = request->GetErrorCode();
							mu_debug_error("failed to retrieve file");
							// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateServerListFailed>();
							Pause();
						}
						break;

					case WEBRequestState::Aborted:
						{
							state = EProcessingState::eCurlFailed;
							Pause();
						}
						break;
					}

					if (state != EProcessingState::eSuccess) break;
				}

				switch (state)
				{
				case EProcessingState::eAborted:
					{
						// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateAborted>();
					}
					break;

				case EProcessingState::eCurlFailed:
					{
						// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateFileDownloadFailed>(curlCode);
					}
					break;

				case EProcessingState::eServerFailed:
					{
						// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateFileDownloadFailed>(errorCode);
					}
					break;

				case EProcessingState::eProcessFailed:
					{
						switch (processResult)
						{
						case EProcessResult::eNoMemoryAvailable:
							{
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateNoMemory>();
							}
							break;

						case EProcessResult::eDecompressFailed:
							{
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateDecompressFailed>();
							}
							break;

						default:
						case EProcessResult::eStorageFull:
							{
								// UIMessageSystem::RequestMessage<EMessageBoxID::eUpdateStorageFull>();
							}
							break;

						case EProcessResult::eCorruptedFile:
							{
								// UIMessageSystem::RequestMessage<EMessageBoxID::eCorruptedFile>();
							}
							break;

						case EProcessResult::eUnknown:
							{
								// UIMessageSystem::RequestMessage<EMessageBoxID::eSystemError>();
							}
							break;
						}
					}
					break;
				}

				for (auto iter = Requests.begin(); iter != Requests.end(); )
				{
					WEBFileDownloadRequest *request = static_cast<WEBFileDownloadRequest *>(iter->get());
					if (request->GetRequestState() >= WEBRequestState::Completed)
					{
						iter = Requests.erase(iter);
					}
					else
					{
						++iter;
					}
				}

				for (auto iter = FilesToDownloadList.begin(); iter != FilesToDownloadList.end(); )
				{
					auto &fileData = *iter;
					if ((fileData->Bitset & EFileBitset::eFinished) != 0)
					{
						iter = FilesToDownloadList.erase(iter);
					}
					else
					{
						++iter;
					}
				}

				if (state == EProcessingState::eSuccess)
				{
					auto requestsCount = Requests.size();
					if (requestsCount < MaxDownloadFiles && requestsCount < FilesToDownloadList.size())
					{
						Requests.reserve(MaxDownloadFiles);
						for (auto iter = FilesToDownloadList.begin(); iter != FilesToDownloadList.end(); ++iter)
						{
							auto &fileData = *iter;
							if ((fileData->Bitset & EFileBitset::eDownloading) != 0) continue;

							auto request = std::make_shared<WEBFileDownloadRequest>(fmt::format("{}{}/{}{}", UpdateServer, fileData->UrlPath, fileData->Filename, fileData->Extension), fileData.get());\
							request->SetRequestFailedCallback(RequestFailedCallback);
							request->SetRequestAbortedCallback(RequestAbortedCallback);
							request->SetReceivedDataCallback(ReceivedDataCallback);

							auto result = MUWebManager::AddRequest(request);
							if (result != CURLM_OK)
							{
								state = EProcessingState::eCurlFailed;
								curlmCode = result;
								// UIMessageSystem::RequestMessage<EMessageBoxID::eWebManagerRequestFailed>();
								break;
							}

							fileData->Bitset |= EFileBitset::eDownloading;
							Requests.push_back(request);
							if (Requests.size() >= MaxDownloadFiles) break;
						}
					}
				}

				switch (state)
				{
				case EProcessingState::eAborted:
				case EProcessingState::eCurlFailed:
				case EProcessingState::eCurlMultiFailed:
				case EProcessingState::eServerFailed:
				case EProcessingState::eProcessFailed:
					{
						Pause();
						for (auto &request : Requests)
						{
							MUWebManager::AbortRequest(request);
						}
						Requests.clear();
					}
					return false;
				}

				if (FilesToDownloadList.empty() == true)
				{
					State = NUpdateState::WritingVersion;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
					context->SetState("WritingVersion");
#endif
				}

				mu_double downloadedSize = static_cast<mu_double>(TotalDownloadedSize.load());
				for (mu_uint32 n = 0; n < TotalDownloadSizeType; ++n)
				{
					downloadedSize /= 1024.0;
				}

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
				context->SetProgress(static_cast<mu_float>(TotalDownloadedSize.load()) / static_cast<mu_float>(TotalDownloadSize));
				context->SetDownloadedSize(downloadedSize);
#endif
			}
			break;

		case NUpdateState::WritingVersion:
			{
				WriteVersion();
				State = NUpdateState::Finished;
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
				context->SetState("StartingGame");
#endif
			}
			break;

		case NUpdateState::Finished:
#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
			context->SetState("LoadingGame");
			context->SetProgress(0.0f);
#endif
			return true;
		}

		return false;
	}

	void Resume()
	{
		Paused = false;
	}

	void Pause()
	{
		Paused = true;
	}
};