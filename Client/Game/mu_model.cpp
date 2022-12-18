#include "stdafx.h"
#include "mu_model.h"
#include "mu_crypt.h"
#include "shared_binaryreader.h"
#include "mu_textures.h"
#include "mu_texture.h"
#include <glm/gtc/type_ptr.hpp>

#define NEXTMU_COMPRESSED_VERTEX (0)

static bgfx::VertexLayout VertexLayout;

static void InitializeVertexLayout()
{
	static mu_boolean initialized = false;
	if (initialized) return;
	initialized = true;
	VertexLayout
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
#if NEXTMU_COMPRESSED_VERTEX
		.add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Int16, true, true)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
#else
		.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
#endif
		.add(bgfx::Attrib::TexCoord1, 2, bgfx::AttribType::Uint8)
		.skip(2)
		.end();
}

struct MeshVertex
{
	glm::vec3 Position;
#if NEXTMU_COMPRESSED_VERTEX
	mu_uint32 Normal[2];
	mu_uint32 TexCoords;
#else
	glm::vec3 Normal;
	glm::vec2 TexCoords;
#endif
	mu_uint8 Bone[2];
};

NModel::~NModel()
{
	if (bgfx::isValid(VertexBuffer))
	{
		bgfx::destroy(VertexBuffer);
		VertexBuffer = BGFX_INVALID_HANDLE;
	}
}

const mu_boolean NModel::Load(const mu_utf8string id, mu_utf8string path)
{
	Id = id;
	NormalizePath<true>(path);

	const mu_utf8string filename = path + "model.json";
	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
	{
		mu_error("model json missing ({})", filename);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
	SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
	jsonBuffer.reset();
	auto document = nlohmann::json::parse(inputBuffer.c_str());
	if (document.is_discarded() == true)
	{
		mu_error("model json malformed ({})", filename);
		return false;
	}

	const auto model = document["model"].get<mu_utf8string>();
	if (LoadModel(path + model) == false)
	{
		mu_error("failed to load model ({})", filename);
		return false;
	}

	if (LoadTextures(path, document) == false)
	{
		mu_error("failed to load model textures ({})", filename);
		return false;
	}

	if (document.contains("boneHead"))
	{
		BoneHead = document["boneHead"].get<mu_int16>();
	}

	if (document.contains("bodyHeight"))
	{
		BodyHeight = document["bodyHeight"].get<mu_float>();
	}

	if (document.contains("playSpeed"))
	{
		PlaySpeed = document["playSpeed"].get<mu_float>();
	}

	if (GenerateBuffers() == false)
	{
		mu_error("failed to generate model buffers ({})", filename);
		return false;
	}

	return true;
}

const mu_boolean NModel::LoadModel(mu_utf8string path)
{
	NormalizePath(path);

	auto ext = path.substr(path.find_last_of('.') + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

	SDL_RWops *fp = nullptr;
	if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
	{
		mu_error("model not found ({})", path);
		return false;
	}

	mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
	std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
	SDL_RWread(fp, buffer.get(), fileLength, 1);
	SDL_RWclose(fp);

	NBinaryReader reader(buffer.get(), static_cast<mu_uint32>(fileLength));

	reader.Skip(3); // BMD bytes
	const mu_uint8 version = reader.Read<mu_uint8>();
	if (version == 12)
	{
		mu_int32 encryptedSize = reader.Read<mu_int32>();
		std::unique_ptr<mu_uint8[]> decryptedBuffer(new_nothrow mu_uint8[encryptedSize]);
		XorDecrypt(decryptedBuffer.get(), reader.GetPointer(), encryptedSize);
		buffer.swap(decryptedBuffer);
		reader.Replace(buffer.get(), encryptedSize);
	}
	else if (version == 14)
	{
		mu_int32 encryptedSize = reader.Read<mu_int32>();
		mu_int64 decryptedSize = CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, nullptr);
		std::unique_ptr<mu_uint8[]> decryptedBuffer(new_nothrow mu_uint8[decryptedSize]);
		CryptoModulusDecrypt(reader.GetPointer(), encryptedSize, decryptedBuffer.get());
		buffer.swap(decryptedBuffer);
		reader.Replace(buffer.get(), static_cast<mu_uint32>(decryptedSize));
	}

	reader.Skip(32); // Name bytes

	const mu_uint32 numMeshes = static_cast<mu_uint32>(reader.Read<mu_int16>());
	const mu_uint32 numBones = static_cast<mu_uint32>(reader.Read<mu_int16>());
	const mu_uint32 numActions = static_cast<mu_uint32>(reader.Read<mu_int16>());

	if (numBones >= static_cast<mu_int16>(MaxBones))
	{
		mu_error("model exceed max bones limit ({})", path);
		return false;
	}

	Meshes.resize(numMeshes);
	BoneName.resize(numBones);
	BoneInfo.resize(numBones);
	Animations.resize(numActions);
	BoundingBoxes.resize(numBones);

	mu_char filename[32 + 1] = {};
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		auto &mesh = Meshes[m];
		const mu_uint32 numVertices = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numNormals = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numTexCoords = static_cast<mu_uint32>(reader.Read<mu_int16>());
		const mu_uint32 numTriangles = static_cast<mu_uint32>(reader.Read<mu_int16>());

		const mu_uint32 texture = static_cast<mu_uint32>(reader.Read<mu_int16>());
		mu_assert(texture == m); // Why would this variable even exists?

		mesh.Vertices.resize(numVertices);
		mesh.Normals.resize(numNormals);
		mesh.TexCoords.resize(numTexCoords);
		mesh.Triangles.resize(numTriangles);

		for (mu_uint32 v = 0; v < numVertices; ++v)
		{
			auto &vertex = mesh.Vertices[v];
			vertex.Node = reader.Read<mu_int16>();
			reader.Skip(2);
			vertex.Position[0] = reader.Read<mu_float>();
			vertex.Position[1] = reader.Read<mu_float>();
			vertex.Position[2] = reader.Read<mu_float>();
		}

		for (mu_uint32 n = 0; n < numNormals; ++n)
		{
			auto &normal = mesh.Normals[n];
			normal.Node = reader.Read<mu_int16>();
			reader.Skip(2);
			normal.Normal[0] = reader.Read<mu_float>();
			normal.Normal[1] = reader.Read<mu_float>();
			normal.Normal[2] = reader.Read<mu_float>();
			reader.Skip(4);
		}

		for (mu_uint32 t = 0; t < numTexCoords; ++t)
		{
			auto &texCoord = mesh.TexCoords[t];
			texCoord.UV[0] = reader.Read<mu_float>();
			texCoord.UV[1] = reader.Read<mu_float>();
		}

		for (mu_uint32 t = 0; t < numTriangles; ++t)
		{
			auto &triangle = mesh.Triangles[t];

			const mu_uint8 polygon = reader.Read<mu_uint8>();
			reader.Skip(1);
			mu_assert(polygon == 3);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.Vertices[n] = reader.Read<mu_int16>();
			}
			reader.Skip(2);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.Normals[n] = reader.Read<mu_int16>();
			}
			reader.Skip(2);

			for (mu_uint32 n = 0; n < 3; ++n)
			{
				triangle.TexCoords[n] = reader.Read<mu_int16>();
			}
			reader.Skip(40);
		}

		reader.ReadLine(filename, 32);
		mesh.Texture.Filename = filename;

		// TODO: Texture Script Parsing, really, didn't have a better idea?
	}

	std::vector<mu_uint32> AnimationKeys(numActions, 0);
	for (mu_uint32 a = 0; a < numActions; ++a)
	{
		auto &animation = Animations[a];
		animation.Loop = false;
		const mu_uint32 numKeys = static_cast<mu_uint32>(reader.Read<mu_int16>());
		AnimationKeys[a] = numKeys;
		animation.LockPositions = reader.Read<mu_boolean>();

		if (animation.LockPositions)
		{
			reader.Skip(sizeof(glm::vec3) * numKeys); // Useless data
		}
	}

	for (mu_uint32 b = 0; b < numBones; ++b)
	{
		auto &info = BoneInfo[b];
		info.Dummy = reader.Read<mu_boolean>();
		if (info.Dummy) continue;
		
		reader.ReadLine(filename, 32);
		BoneName[b] = filename;

		info.Parent = reader.Read<mu_int16>();

		for (mu_uint32 a = 0; a < numActions; ++a)
		{
			auto &animation = Animations[a];

			const mu_uint32 numKeys = AnimationKeys[a];
			animation.Keys.resize(numKeys);

			/*
				Why I decided to structure it this way?
				to benefit from cache when processing animations, slower loading but faster performance later.
				Hopefully the bones are sorted.
			*/
			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				animationKey.Bones.resize(numBones);
			}

			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				reader.ReadLine(&animationKey.Bones[b].Position, sizeof(glm::vec3));
			}

			for (mu_uint32 k = 0; k < numKeys; ++k)
			{
				auto &animationKey = animation.Keys[k];
				glm::vec3 rotation;
				reader.ReadLine(&rotation, sizeof(glm::vec3));
				animationKey.Bones[b].Rotation = glm::quat(rotation);
			}
		}
	}

	CalculateBoundingBoxes();

	return true;
}

const mu_boolean NModel::LoadTextures(const mu_utf8string path, const nlohmann::json &document)
{
	mu_utf8string subPath = "textures/";

	if (document.contains("textures"))
	{
		const auto &textures = document["textures"];

		mu_utf8string filter = textures.contains("filter") ? textures["filter"].get<mu_utf8string>() : "";
		mu_utf8string wrap = textures.contains("wrap") ? textures["wrap"].get<mu_utf8string>() : "";
		const mu_boolean isValidFilter = MUTextures::IsValidFilter(filter);
		const mu_boolean isValidWrap = MUTextures::IsValidWrap(wrap);

		if (isValidFilter || isValidWrap)
		{
			for (auto &mesh : Meshes)
			{
				if (isValidFilter) mesh.Texture.Filter = filter;
				if (isValidWrap) mesh.Texture.Wrap = wrap;
			}
		}

		if (textures.contains("meshes"))
		{
			const auto &meshes = textures["meshes"];
			for (const auto &texture : meshes)
			{
				const mu_uint32 index = texture["mesh"].get<mu_uint32>();
				if (index >= Meshes.size()) continue;

				auto &mesh = Meshes[index];

				mu_utf8string filter = texture.contains("filter") ? texture["filter"].get<mu_utf8string>() : "";
				mu_utf8string wrap = texture.contains("wrap") ? texture["wrap"].get<mu_utf8string>() : "";

				MUTextures::NormalizeFilter(filter);
				MUTextures::NormalizeFilter(wrap);

				if (MUTextures::IsValidFilter(filter))
				{
					mesh.Texture.ForceFilter = true;
					mesh.Texture.Filter = filter;
				}

				if (MUTextures::IsValidWrap(wrap))
				{
					mesh.Texture.ForceWrap = true;
					mesh.Texture.Wrap = wrap;
				}
			}
		}
	}

	const mu_uint32 numMeshes = static_cast<mu_uint32>(Meshes.size());
	Textures.resize(numMeshes);
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		const auto &mesh = Meshes[m];

		mu_utf8string filename = mesh.Texture.Filename;
		if (filename.starts_with("ski") || filename.starts_with("level"))
		{
			Textures[m] = {
				.Type = TextureAttachment::Skin,
			};
		}
		else if (filename.starts_with("hid"))
		{
			Textures[m] = {
				.Type = TextureAttachment::Hide,
			};
		}
		else if (filename.starts_with("hair"))
		{
			Textures[m] = {
				.Type = TextureAttachment::Hair,
			};
		}
		else
		{
			const mu_size extAt = filename.find_last_of('.');
			mu_utf8string ext = extAt != mu_utf8string::npos ? filename.substr(extAt) : "";
			std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

			mu_utf8string filter = mesh.Texture.Filter;
			mu_utf8string wrap = mesh.Texture.Wrap;

			// Force nearest filter in TGA files
			if (
				mesh.Texture.ForceFilter == false &&
				ext.starts_with(".t")
			)
			{
				filter = "nearest";
			}

			mu_boolean hasAlpha = false;
			if (ext.starts_with(".t"))
			{
				filename = filename.substr(0, extAt) + ".ozt";
				hasAlpha = true;
			}
			else if (ext.starts_with(".j"))
			{
				filename = filename.substr(0, extAt) + ".ozj";
			}

			filename = path + subPath + filename;
			std::unique_ptr<NTexture> texture = MUTextures::Load(
				filename,
				MUTextures::CalculateSamplerFlags(filter, wrap)
			);
			if (!texture)
			{
				mu_error("texture not found ({})", filename);
				return false;
			}

			Textures[m] = {
				.Type = TextureAttachment::Normal,
				.Texture = std::move(texture),
			};
		}
	}

	return true;
}

const mu_boolean NModel::GenerateBuffers()
{
	InitializeVertexLayout();

	mu_uint32 verticesCount = 0;
	for (auto &mesh : Meshes)
	{
		mesh.VertexBuffer.Offset = verticesCount;
		mesh.VertexBuffer.Count = static_cast<mu_uint32>(mesh.Triangles.size()) * 3u;
		verticesCount += mesh.VertexBuffer.Count;
	}

	if (verticesCount == 0) return true;

	const bgfx::Memory *mem = bgfx::alloc(verticesCount * sizeof(MeshVertex));
	MeshVertex *dest = reinterpret_cast<MeshVertex *>(mem->data);

	for (const auto &mesh : Meshes)
	{
		for (const auto &triangle : mesh.Triangles)
		{
			for (mu_uint32 n = 0; n < 3; ++n)
			{
				const auto &vertex = mesh.Vertices[triangle.Vertices[n]];
				const auto &normal = mesh.Normals[triangle.Normals[n]];
				const auto &texCoord = mesh.TexCoords[triangle.TexCoords[n]];

				mu_assert(vertex.Node == normal.Node);

				dest->Position = vertex.Position;

				/*
					V is being flipped because bgfx flips the image, it follows DirectX 9 functionality.
				*/
#if NEXTMU_COMPRESSED_VERTEX
				glm::vec4 Normal(normal.Normal, 0.0f);
				bgfx::vertexPack(glm::value_ptr(Normal), true, bgfx::Attrib::Normal, VertexLayout, dest);
				glm::vec2 UV(texCoord.UV.x, 1.0f - texCoord.UV.y);
				bgfx::vertexPack(glm::value_ptr(UV), true, bgfx::Attrib::TexCoord0, VertexLayout, dest);
#else
				dest->Normal = normal.Normal;
				dest->TexCoords = glm::vec2(texCoord.UV.x, 1.0f - texCoord.UV.y);
#endif

				dest->Bone[0] = static_cast<mu_uint8>(vertex.Node);
				dest->Bone[1] = static_cast<mu_uint8>(normal.Node);

				++dest;
			}
		}
	}

	VertexBuffer = bgfx::createVertexBuffer(mem, VertexLayout);
	if (!bgfx::isValid(VertexBuffer))
	{
		return false;
	}

	return true;
}

void NModel::CalculateBoundingBoxes()
{
	const mu_uint32 numMeshes = static_cast<mu_uint32>(Meshes.size());
	for (mu_uint32 m = 0; m < numMeshes; ++m)
	{
		auto &mesh = Meshes[m];

		const mu_uint32 numVertices = static_cast<mu_uint32>(mesh.Vertices.size());
		for (mu_uint32 v = 0; v < numVertices; ++v)
		{
			auto &vertex = mesh.Vertices[v];
			if (vertex.Node == NInvalidInt16) continue;

			auto &boundingBox = BoundingBoxes[vertex.Node];
			for (mu_uint32 n = 0; n < 3; ++n)
			{
				if (vertex.Position[n] < boundingBox.Min[n]) boundingBox.Min[n] = vertex.Position[n];
				if (vertex.Position[n] > boundingBox.Max[n]) boundingBox.Max[n] = vertex.Position[n];
			}
		}
	}
}