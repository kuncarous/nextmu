#include "mu_precompiled.h"
#include "mu_charactersmanager.h"
#include "mu_textureattachments.h"
#include "mu_resourcesmanager.h"

namespace MUCharactersManager
{
	std::map<mu_utf8string, NCharacterType> TypeMap;
	std::map<mu_uint32, NCharacterClass> ClassesMap;

	const mu_uint32 GetCharacterSexFromString(const mu_utf8string value)
	{
		return static_cast<mu_uint32>(value.compare("male") == 0 ? NCharacterSex::Male : NCharacterSex::Female);
	}

	const mu_boolean Load()
	{
		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, "data/characters.json", "rb") == false)
		{
			return false;
		}

		mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));
		std::unique_ptr<mu_char[]> jsonBuffer(new_nothrow mu_char[fileLength]);
		SDL_RWread(fp, jsonBuffer.get(), fileLength, 1);
		SDL_RWclose(fp);

		const mu_utf8string inputBuffer = JsonStripComments(jsonBuffer.get(), static_cast<mu_uint32>(fileLength));
		jsonBuffer.reset();
		auto document = nlohmann::json::parse(inputBuffer.c_str());
		if (document.is_discarded() == true || document.is_array() == false)
		{
			return false;
		}

		for (const auto &jcharacter : document)
		{
			NCharacterClass character;

			const auto id = jcharacter["id"].get<mu_uint32>();
			character.Id = id;

			const auto sex = GetCharacterSexFromString(jcharacter["sex"].get<mu_utf8string>());
			const auto &jsubClasses = jcharacter["subclass"];
			const mu_uint32 subClassesCount = static_cast<mu_uint32>(jsubClasses.size());
			for (mu_uint32 n = 0; n < subClassesCount; ++n)
			{
				const auto &jsubClass = jsubClasses[n];
				NCharacterConfiguration config;
				config.Sex = sex;

				const auto &jattachments = jsubClass["attachments"];
				for (const auto &jattachment : jattachments)
				{
					const auto id = jattachment["id"].get<mu_utf8string>();
					const auto type = MUTextureAttachments::GetAttachmentTypeFromString(id);
					if (type == NInvalidAttachment) continue;

					const auto texture = MUResourcesManager::GetTexture(jattachment["texture"].get<mu_utf8string>());
					if (texture == nullptr) continue;

					config.Attachments.push_back(
						NCharacterAttachment{
							.Type = type,
							.Texture = texture,
						}
					);
				}

				const auto &jbody = jsubClass["body"];
				if (jbody.contains("head")) config.Parts[static_cast<mu_uint32>(CharacterBodyPart::Head)] = MUResourcesManager::GetModel(jbody["head"].get<mu_utf8string>());
				if (jbody.contains("chest")) config.Parts[static_cast<mu_uint32>(CharacterBodyPart::Chest)] = MUResourcesManager::GetModel(jbody["chest"].get<mu_utf8string>());
				if (jbody.contains("lower")) config.Parts[static_cast<mu_uint32>(CharacterBodyPart::Lower)] = MUResourcesManager::GetModel(jbody["lower"].get<mu_utf8string>());
				if (jbody.contains("arms")) config.Parts[static_cast<mu_uint32>(CharacterBodyPart::Arms)] = MUResourcesManager::GetModel(jbody["arms"].get<mu_utf8string>());
				if (jbody.contains("legs")) config.Parts[static_cast<mu_uint32>(CharacterBodyPart::Legs)] = MUResourcesManager::GetModel(jbody["legs"].get<mu_utf8string>());

				TypeMap.insert(
					std::make_pair(
						jsubClass["id"].get<mu_utf8string>(),
						NCharacterType{
							.Class = static_cast<mu_uint16>(character.Id),
							.SubClass = static_cast<mu_uint16>(n)
						}
					)
				);
				character.SubClasses.push_back(config);
			}

			ClassesMap.insert(std::make_pair(id, character));
		}

		return true;
	}

	void Destroy()
	{
		ClassesMap.clear();
	}

	const NCharacterType GetTypeFromString(const mu_utf8string id)
	{
		auto iter = TypeMap.find(id);
		if (iter == TypeMap.end()) return NCharacterType();
		return iter->second;
	}

	const NCharacterConfiguration *GetConfiguration(const mu_uint32 classId, const mu_uint32 subClassId)
	{
		auto iter = ClassesMap.find(classId);
		if (iter == ClassesMap.end()) return nullptr;
		auto &character = iter->second;
		if (subClassId >= static_cast<mu_uint32>(character.SubClasses.size())) return nullptr;
		return &character.SubClasses[subClassId];
	}
}