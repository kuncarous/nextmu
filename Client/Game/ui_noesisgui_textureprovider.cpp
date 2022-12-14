#include "stdafx.h"
#include "ui_noesisgui_textureprovider.h"
#include "ui_noesisgui_stream.h"
#include "ui_noesisgui_consts.h"
#include "mu_textures.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	/// Returns metadata for the texture at the given URI or empty rectangle if texture is not found
	Noesis::TextureInfo TextureProvider::GetTextureInfo(const Noesis::Uri &uri)
	{
		Noesis::TextureInfo info;
		const mu_utf8string filename = GetResourcesPath() + uri.Str();
		Noesis::Ptr<Noesis::Stream> stream = Stream::Load(filename);
		if (!stream)
		{
			return info;
		}

		const uint32_t fileSize = stream->GetLength();
		std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileSize]);
		if (!buffer)
		{
			return info;
		}

		const uint32_t readedSize = stream->Read(buffer.get(), fileSize);
		if (readedSize < fileSize)
		{
			return info;
		}

		FIMEMORY *memory = FreeImage_OpenMemory(buffer.get(), fileSize);
		if (!memory)
		{
			return info;
		}

		FIBITMAP *bitmap = FreeImage_LoadFromMemory(FreeImage_GetFIFFromFilename(filename.c_str()), memory);
		FreeImage_CloseMemory(memory);

		if (!bitmap)
		{
			return info;
		}

		info.x = 0;
		info.y = 0;
		info.width = FreeImage_GetWidth(bitmap);
		info.height = FreeImage_GetHeight(bitmap);

		FreeImage_Unload(bitmap);

		return info;
	}

	/// Returns a texture compatible with the given device or null if texture is not found
	Noesis::Ptr<Noesis::Texture> TextureProvider::LoadTexture(const Noesis::Uri &uri, Noesis::RenderDevice *device)
	{
		const mu_utf8string filename = GetResourcesPath() + uri.Str();
		Noesis::Ptr<Noesis::Stream> stream = Stream::Load(filename);
		if (!stream)
		{
			return nullptr;
		}

		const uint32_t fileSize = stream->GetLength();
		std::unique_ptr<mu_uint8[]> buffer(new_nothrow mu_uint8[fileSize]);
		if (!buffer)
		{
			return nullptr;
		}

		const uint32_t readedSize = stream->Read(buffer.get(), fileSize);
		stream->Close();

		if (readedSize < fileSize)
		{
			return nullptr;
		}

		FIMEMORY *memory = FreeImage_OpenMemory(buffer.get(), fileSize);
		if (!memory)
		{
			return nullptr;
		}

		FIBITMAP *bitmap = FreeImage_LoadFromMemory(FreeImage_GetFIFFromFilename(filename.c_str()), memory);
		FreeImage_CloseMemory(memory);

		if (!bitmap)
		{
			return nullptr;
		}

		const auto width = FreeImage_GetWidth(bitmap);
		const auto height = FreeImage_GetHeight(bitmap);
		const auto bpp = FreeImage_GetBPP(bitmap);
		const auto components = MUTextures::CalculateComponentsCount(bitmap);
		const auto pixels = width * height;

		//     N=#comp     components
		//       1           grey
		//       2           grey, alpha
		//       3           red, green, blue
		//       4           red, green, blue, alpha
		const mu_boolean hasAlpha = components == 2 || components == 4;

		// Premultiply alpha
		if (hasAlpha)
		{
			if (!FreeImage_PreMultiplyWithAlpha(bitmap))
			{
				return nullptr;
			}

			if (device->GetCaps().linearRendering)
			{
				// TODO : check if FreeImage requires conversion to sRGB of the textures.
			}
		}

		// TODO : Generate Mipmaps
		uint32_t numLevels = static_cast<uint32_t>(glm::max(glm::log2(static_cast<mu_float>(width)), glm::log2(static_cast<mu_float>(height)))) + 1u;
		std::vector<FIBITMAP *> bitmaps;
		std::vector<void *> data;

		bitmaps.reserve(numLevels);
		bitmaps.push_back(bitmap);
		data.reserve(numLevels);
		data.push_back(FreeImage_GetBits(bitmap));

		for (uint32_t i = 1; i < numLevels; i++)
		{
			uint32_t outW = glm::max(width >> i, 1u);
			uint32_t outH = glm::max(height >> i, 1u);

			FIBITMAP *source = bitmaps[i - 1];
			FIBITMAP *clone = FreeImage_Rescale(source, outW, outH, FILTER_BICUBIC);
			if (!clone)
			{
				for (auto &bitmap : bitmaps)
				{
					FreeImage_Unload(bitmap);
				}
				return nullptr;
			}

			bitmaps.push_back(clone);
			data.push_back(FreeImage_GetBits(clone));
		}

		mu_int32 index = Noesis::StrFindLast(uri.Str(), "/");
		const mu_char *label = index == -1 ? uri.Str() : uri.Str() + index + 1;

		// Not using compressed textures is far from ideal but it would complicate the pipeline as
		// having to support and detect availability for all formats in all supported GPUs is not a
		// simple task. Here we are just following the easy path, the texture is created uncompressed
		// using the RenderDevice. This is fine for the App framework and for our examples.
		// For production, we do *not* recommend doing this. Just preprocess your textures, compressing
		// them offline and wrap them inside a Texture object, similar to D3D11Fatory::WrapTexture

		Noesis::TextureFormat::Enum format = hasAlpha ? Noesis::TextureFormat::RGBA8 : Noesis::TextureFormat::RGBX8;
		Noesis::Ptr<Noesis::Texture> texture = device->CreateTexture(label, width, height, numLevels, format, (const void**)data.data());

		for (auto &bitmap : bitmaps)
		{
			FreeImage_Unload(bitmap);
		}

		return texture;
	}
};
#endif