#ifndef __MU_RENDERSTATE_ENUMS_H__
#define __MU_RENDERSTATE_ENUMS_H__

#pragma once

namespace TextureAttachment
{
	typedef mu_uint32 Type;
	enum : Type
	{
		Normal,
		Skin,
		Hide,
		Hair,
		Count,
	};
};

#endif