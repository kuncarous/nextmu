#ifndef __MU_TEXTURES_H__
#define __MU_TEXTURES_H__

#pragma once

namespace MUTextures
{
	const mu_boolean LoadRaw(mu_utf8string path, FIBITMAP **texture);
	const mu_boolean Load(mu_utf8string path, bgfx::TextureHandle *texture, const mu_uint64 samplerFlags);

	const mu_boolean IsValidFilter(const mu_utf8string value);
	const mu_boolean IsValidWrap(const mu_utf8string value);
	const mu_uint64 CalculateSamplerFlags(const mu_utf8string filter, const mu_utf8string wrap);

	const mu_uint32 CalculateComponentsCount(FIBITMAP *bitmap);

	NEXTMU_INLINE void NormalizeFilter(mu_utf8string &path)
	{
		std::transform(path.begin(), path.end(), path.begin(), mu_utf8tolower);
	}
};

#endif