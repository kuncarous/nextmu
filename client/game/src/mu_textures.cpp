#include "mu_precompiled.h"
#include "mu_textures.h"
#include "mu_graphics.h"

namespace MUTextures
{
	mu_atomic_uint32_t TextureIdGenerator = 0;

	template <class T> void SwapValue(T& a, T& b) {
		T tmp = a;
		a = b;
		b = tmp;
	}

	const mu_uint32 GenerateTextureId()
	{
		return TextureIdGenerator++;
	}

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
		mu_int32 fflags = 0;
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
			return false;
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

		constexpr mu_uint32 bitsPerPixel = 32u;
		constexpr mu_uint32 bytesPerPixel = bitsPerPixel / 8u;

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
		{
			const mu_uint32 width = FreeImage_GetWidth(bitmap);
			const mu_uint32 height = FreeImage_GetHeight(bitmap);
			for (mu_uint32 y = 0; y < height; y++) {
				mu_uint8 *pixels = FreeImage_GetScanLine(bitmap, y);
				for (mu_uint32 x = 0; x < width; x++) {
					SwapValue(pixels[0], pixels[2]);
					pixels += bytesPerPixel;
				}
			}
		}
#endif

		info.Width = FreeImage_GetWidth(bitmap);
		info.Height = FreeImage_GetHeight(bitmap);
		info.Alpha = alpha;
		*texture = bitmap;

		return true;
	}

	NGraphicsTexturePtr Load(mu_utf8string path, const Diligent::SamplerDesc &samplerDesc)
	{
		TextureInfo info;
		FIBITMAP *bitmap = nullptr;
		if (LoadRaw(path, &bitmap, info) == false)
		{
			return nullptr;
		}

		auto device = MUGraphics::GetDevice();

		const mu_uint32 width = info.Width;
		const mu_uint32 height = info.Height;
		const mu_uint32 bpp = FreeImage_GetBPP(bitmap);
		const mu_uint32 bitmapSize = width * height * (bpp / 8);
		const mu_uint8 *bitmapBuffer = FreeImage_GetBits(bitmap);

		std::vector<Diligent::TextureSubResData> subresources;
		Diligent::TextureSubResData subresource;
		subresource.pData = bitmapBuffer;
		subresource.Stride = width * (bpp / 8);
		subresources.push_back(subresource);

		Diligent::TextureDesc textureDesc;
#if NEXTMU_COMPILE_DEBUG == 1
		textureDesc.Name = path.c_str();
#endif
		textureDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
		textureDesc.Usage = Diligent::USAGE_IMMUTABLE;
		textureDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE;

		Diligent::TextureData textureData(subresources.data(), static_cast<mu_uint32>(subresources.size()));
		Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
		device->CreateTexture(textureDesc, &textureData, &texture);
		if (texture == nullptr)
		{
			return nullptr;
		}

		const auto immediateContext = MUGraphics::GetImmediateContext();
		Diligent::StateTransitionDesc barrier(texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		immediateContext->TransitionResourceStates(1u, &barrier);
		texture->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE)->SetSampler(GetTextureSampler(samplerDesc)->Sampler);

		MUGraphics::IncreaseTransactions();
		MUGraphics::CheckIfRequireFlushContext();

		return std::make_unique<NGraphicsTexture>(GenerateTextureId(), texture, width, height, info.Alpha);
	}

	struct SamplerMode
	{
		const mu_utf8string name;
		const Diligent::FILTER_TYPE mode;
	};

	struct AddressMode
	{
		const mu_utf8string name;
		const Diligent::TEXTURE_ADDRESS_MODE mode;
	};

	const std::array<SamplerMode, 2> filters = { {
		{
			.name = "linear",
			.mode = Diligent::FILTER_TYPE_LINEAR,
		},
		{
			.name = "nearest",
			.mode = Diligent::FILTER_TYPE_POINT,
		},
	} };
	const std::array<AddressMode, 3> wraps = { {
		{
			.name = "repeat",
			.mode = Diligent::TEXTURE_ADDRESS_WRAP,
		},
		{
			.name = "clamp",
			.mode = Diligent::TEXTURE_ADDRESS_CLAMP,
		},
		{
			.name = "mirror",
			.mode = Diligent::TEXTURE_ADDRESS_MIRROR,
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
		for (mu_uint32 index = 0; index < wraps.size(); ++index)
		{
			const auto &f = wraps[index];
			if (value == f.name) return true;
		}

		return false;
	}

	const Diligent::SamplerDesc CalculateSamplerFlags(const mu_utf8string filter, const mu_utf8string wrap)
	{
		Diligent::SamplerDesc desc;

		for (mu_uint32 index = 0; index < filters.size(); ++index)
		{
			const auto &f = filters[index];
			if (filter == f.name)
			{
				desc.MinFilter = f.mode;
				desc.MagFilter = f.mode;
				desc.MipFilter = f.mode;
				break;
			}
		}

		for (mu_uint32 index = 0; index < wraps.size(); ++index)
		{
			const auto &f = wraps[index];
			if (wrap == f.name)
			{
				desc.AddressU = f.mode;
				desc.AddressV = f.mode;
				desc.AddressW = f.mode;
				break;
			}
		}

		return desc;
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