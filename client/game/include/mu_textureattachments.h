#ifndef __MU_TEXTUREATTACHMENTS_H__
#define __MU_TEXTUREATTACHMENTS_H__

#pragma once

#include "t_textureattachments.h"

namespace MUTextureAttachments
{
	const mu_boolean Initialize();
	void Destroy();

	const mu_size GetAttachmentsCount();
	NTextureAttachmentType CreateAttachmentTypeByString(const mu_utf8string id);
	const NTextureAttachmentType GetAttachmentTypeFromString(const mu_utf8string id);
}

#endif