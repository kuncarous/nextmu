#include "mu_precompiled.h"
#include "res_renders.h"
#include "res_render.h"
#include "mu_resourcesmanager.h"

std::map<mu_utf8string, NPartType> PartTypeIds({
	std::pair("helm", NPartType::Helm),
	std::pair("armor", NPartType::Armor),
	std::pair("pants", NPartType::Pants),
	std::pair("gloves", NPartType::Gloves),
	std::pair("boots", NPartType::Boots),
	std::pair("item_left", NPartType::ItemLeft),
	std::pair("item_right", NPartType::ItemRight),
	std::pair("wings", NPartType::Wings),
	std::pair("helper", NPartType::Helper),
});

namespace MURendersManager
{
	std::map<mu_utf8string, NRender> Renders;

	const mu_boolean Initialize()
	{
		const mu_utf8string path = "data/";
		const mu_utf8string filename = path + "renders.json";

		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
		{
			mu_error("renders.json missing ({})", filename);
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
			mu_error("resources malformed ({})", filename);
			return false;
		}

		for (const auto &jrender : document)
		{
			const auto id = jrender["id"].get<mu_utf8string>();
			const auto model = jrender["model"].get<mu_utf8string>();

			NRender render;
			render.Id = id;
			render.Model = MUResourcesManager::GetModel(model);
			render.IsLinked = jrender["is_linked"].get<mu_boolean>();

			if (jrender.contains("animations"))
			{
				for (const auto &janimation : jrender["animations"])
				{
					const auto ids = janimation["id"];

					NRenderAnimation animation;
					animation.Visible = janimation["visible"].get<mu_boolean>();

					if (janimation.contains("attachments"))
					{
						for (const auto &jattachment : janimation["attachments"])
						{
							const auto id = jattachment["id"].get<mu_utf8string>();
							const auto partType = GetPartTypeById(id);

							NRenderAttachment attachment;
							attachment.Bone = jattachment["bone"].get<mu_utf8string>();
							const auto &jposition = jattachment["position"];
							attachment.Position = glm::vec3(jposition[0].get<mu_float>(), jposition[1].get<mu_float>(), jposition[2].get<mu_float>());
							const auto &jangle = jattachment["angle"];
							attachment.Angle = glm::vec3(jangle[0].get<mu_float>(), jangle[1].get<mu_float>(), jangle[2].get<mu_float>());
							attachment.Scale = jattachment["scale"].get<mu_float>();

							if (partType == NPartType::Max)
							{
								animation.Attachments.Default = attachment;
							}
							else
							{
								animation.Attachments.Customs.insert(std::make_pair(partType, attachment));
							}
						}
					}

					for (const auto &jid : ids)
					{
						const auto id = jid.get<mu_utf8string>();
						if (id.compare("default") == 0)
						{
							render.Animations.Default = animation;
						}
						else
						{
							render.Animations.Customs.insert(std::make_pair(id, animation));
						}
					}
				}
			}

			Renders.insert(std::make_pair(id, render));
		}

		return true;
	}

	void Destroy()
	{
		Renders.clear();
	}

	NRender *GetRender(const mu_utf8string id)
	{
		auto iter = Renders.find(id);
		if (iter == Renders.end()) return nullptr;
		return &iter->second;
	}
}