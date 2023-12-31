#include "stdafx.h"
#include "mu_textureattachments.h"

namespace MUTextureAttachments
{
	std::map<mu_utf8string, NTextureAttachmentType> AttachmentsMap;

	const mu_boolean Initialize()
	{
		return true;
	}

	void Destroy()
	{
		AttachmentsMap.clear();
	}

	const mu_size GetAttachmentsCount()
	{
		return AttachmentsMap.size();
	}

	NTextureAttachmentType CreateAttachmentTypeByString(const mu_utf8string id)
	{
		auto iter = AttachmentsMap.find(id);
		if (iter != AttachmentsMap.end()) return NInvalidAttachment;
		const auto type = static_cast<NTextureAttachmentType>(AttachmentsMap.size());
		AttachmentsMap.insert(std::make_pair(id, type));
		return type;
	}

	const NTextureAttachmentType GetAttachmentTypeFromString(const mu_utf8string id)
	{
		auto iter = AttachmentsMap.find(id);
		if (iter == AttachmentsMap.end()) return NInvalidAttachment;
		return iter->second;
	}
}