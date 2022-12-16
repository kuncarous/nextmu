#include "stdafx.h"
#include "mu_textures.h"
#include "mu_texture.h"

namespace MUTextures
{
	const mu_boolean LoadRaw(mu_utf8string path, FIBITMAP **texture, TextureInfo &info)
	{
		NormalizePath(path);

		auto ext = path.substr(path.find_last_of('.') + 1);
		std::transform(ext.begin(), ext.end(), ext.begin(), mu_utf8tolower);

		SDL_RWops *fp = nullptr;
		if (mu_rwfromfile<EGameDirectoryType::eSupport>(&fp, path, "rb") == false)
		{
			mu_error("texture not found ({})", path);
			return false;
		}

		mu_isize fileLength = static_cast<mu_isize>(SDL_RWsize(fp));

		if (ext == "ozj")
		{
			fileLength -= 24;
			SDL_RWseek(fp, 24, RW_SEEK_CUR);
		}
		else if (ext == "ozt")
		{
			fileLength -= 4;
			SDL_RWseek(fp, 4, RW_SEEK_CUR);
		}

		std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileLength]);
		SDL_RWread(fp, buffer.get(), fileLength, 1);
		SDL_RWclose(fp);

		mu_boolean alpha = false;
		mu_uint64 fflags = 0;
		FREE_IMAGE_FORMAT format;
		if (ext == "ozj" || ext == "jpg" || ext == "jpeg")
		{
			format = FREE_IMAGE_FORMAT::FIF_JPEG;
		}
		else if (ext == "ozt" || ext == "tga")
		{
			alpha = true;
			format = FREE_IMAGE_FORMAT::FIF_TARGA;
		}
		else if (ext == "png")
		{
			fflags = PNG_IGNOREGAMMA;
			format = FREE_IMAGE_FORMAT::FIF_PNG;
		}
		else
		{
			mu_assert(!"unsupported texture format");
		}

		FIMEMORY *memory = FreeImage_OpenMemory(buffer.get(), static_cast<DWORD>(fileLength));
		if (memory == nullptr)
		{
			return false;
		}

		FIBITMAP *bitmap = FreeImage_LoadFromMemory(format, memory, fflags);
		buffer.reset();
		FreeImage_CloseMemory(memory);

		if (bitmap == nullptr)
		{
			return false;
		}

		const FREE_IMAGE_TYPE imageType = FreeImage_GetImageType(bitmap);
		if (imageType != FIT_BITMAP)
		{
			FreeImage_Unload(bitmap);
			return false;
		}

		const mu_uint32 bpp = FreeImage_GetBPP(bitmap);
		if (bpp != 32)
		{
			FIBITMAP *newBitmap = FreeImage_ConvertTo32Bits(bitmap);
			FreeImage_Unload(bitmap);

			if (newBitmap == nullptr)
			{
				return false;
			}

			bitmap = newBitmap;
		}

		info.Width = FreeImage_GetWidth(bitmap);
		info.Height = FreeImage_GetHeight(bitmap);
		info.Alpha = alpha;
		*texture = bitmap;

		return true;
	}

	std::unique_ptr<NTexture> Load(mu_utf8string path, const mu_uint64 samplerFlags)
	{
		TextureInfo info;
		FIBITMAP *bitmap = nullptr;
		if (LoadRaw(path, &bitmap, info) == false)
		{
			return nullptr;
		}

		const mu_uint32 width = info.Width;
		const mu_uint32 height = info.Height;
		const mu_uint32 bpp = FreeImage_GetBPP(bitmap);
		const mu_uint32 bitmapSize = width * height * (bpp / 8);
		const mu_uint8 *bitmapBuffer = FreeImage_GetBits(bitmap);
		const bgfx::Memory *mem = bgfx::copy(bitmapBuffer, bitmapSize);
		bgfx::TextureHandle texture = bgfx::createTexture2D(
			width,
			height,
			false,
			1,
			bgfx::TextureFormat::RGBA8,
			samplerFlags,
			mem
		);
		FreeImage_Unload(bitmap);

		if (bgfx::isValid(texture) == false)
		{
			return nullptr;
		}

		return std::make_unique<NTexture>(texture, width, height, info.Alpha);
	}

	struct TextureFlags
	{
		const mu_utf8string name;
		const mu_uint64 flags;
	};

	const std::array<TextureFlags, 2> filters = { {
		{
			.name = "linear",
			.flags = 0,
		},
		{
			.name = "nearest",
			.flags = BGFX_SAMPLER_POINT,
		},
	} };
	const std::array<TextureFlags, 3> wraps = { {
		{
			.name = "repeat",
			.flags = 0,
		},
		{
			.name = "clamp",
			.flags = BGFX_SAMPLER_UVW_CLAMP,
		},
		{
			.name = "mirror",
			.flags = BGFX_SAMPLER_UVW_MIRROR,
		},
	} };

	const mu_boolean IsValidFilter(const mu_utf8string value)
	{
		for (mu_uint32 index = 0; index < filters.size(); ++index)
		{
			const auto &f = filters[index];
			if (value == f.name) return true;
		}

		return false;
	}

	const mu_boolean IsValidWrap(const mu_utf8string value)
	{
		for (mu_uint32 index = 0; index < filters.size(); ++index)
		{
			const auto &f = wraps[index];
			if (value == f.name) return true;
		}

		return false;
	}

	const mu_uint64 CalculateSamplerFlags(const mu_utf8string filter, const mu_utf8string wrap)
	{
		mu_uint64 flags = 0;

		for (mu_uint32 index = 0; index < filters.size(); ++index)
		{
			const auto &f = filters[index];
			if (filter == f.name)
			{
				flags |= f.flags;
				break;
			}
		}

		for (mu_uint32 index = 0; index < wraps.size(); ++index)
		{
			const auto &f = wraps[index];
			if (wrap == f.name)
			{
				flags |= f.flags;
				break;
			}
		}

		return flags;
	}

	const mu_uint32 CalculateComponentsCount(FIBITMAP *bitmap)
	{
		const FREE_IMAGE_TYPE format = FreeImage_GetImageType(bitmap);
		const mu_uint32 bpp = FreeImage_GetBPP(bitmap);

		switch (format)
		{
		case FIT_BITMAP: return bpp / 8;
		case FIT_UINT16:
		case FIT_INT16: return (bpp / 8) / sizeof(mu_uint16);
		case FIT_UINT32:
		case FIT_INT32: return (bpp / 8) / sizeof(mu_uint32);
		case FIT_FLOAT: return (bpp / 8) / sizeof(mu_float);
		case FIT_DOUBLE: return (bpp / 8) / sizeof(mu_double);
		case FIT_COMPLEX: return (bpp / 8) / (sizeof(mu_double) * 2);
		case FIT_RGB16: return 3;
		case FIT_RGBA16: return 4;
		case FIT_RGBF: return 3;
		case FIT_RGBAF: return 4;
		default: return 4;
		}
	}
};