#include "stdafx.h"
#include "res_items.h"
#include "res_renders.h"

namespace MUItemsManager
{
	std::map<mu_uint32, NItem> Items;

	NEXTMU_INLINE const mu_uint32 GetItemId(const mu_uint16 category, const mu_uint16 index)
	{
		return static_cast<mu_uint32>(category) << 16 | static_cast<mu_uint32>(index);
	}

	const mu_boolean Initialize()
	{
		const mu_utf8string path = "data/";
		const mu_utf8string filename = path + "items.json";

		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, filename, "rb") == false)
		{
			mu_error("items.json missing ({})", filename);
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

		for (const auto &jitem : document)
		{
			const auto category = jitem["category"].get<mu_uint16>();
			const auto index = jitem["index"].get<mu_uint16>();
			const auto render = jitem["render"].get<mu_utf8string>();

			NItem item;
			item.Category = category;
			item.Index = index;
			item.Render = MURendersManager::GetRender(render);

			Items.insert(std::make_pair(GetItemId(category, index), item));
		}

		return true;
	}

	void Destroy()
	{

	}

	const NItem *GetItem(const mu_uint16 category, const mu_uint16 index)
	{
		const auto id = GetItemId(category, index);
		auto iter = Items.find(id);
		if (iter == Items.end()) return nullptr;
		return &iter->second;
	}
}