#include "stdafx.h"
#include "ui_noesisgui_textureprovider.h"
#include "ui_noesisgui_stream.h"
#include "ui_noesisgui_consts.h"
#include "mu_textures.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_NOESISGUI
namespace UINoesis
{
	template <class T> void SwapValue(T &a, T &b) {
		T tmp = a;
		a = b;
		b = tmp;
	}

	// Thanks to STB library for this, since FreeImage doesn't have this functionality I decided to implement it using this from STB
	static mu_float srgb_uchar_to_linear_float[256] = {
		0.000000f, 0.000304f, 0.000607f, 0.000911f, 0.001214f, 0.001518f, 0.001821f, 0.002125f, 0.002428f, 0.002732f, 0.003035f,
		0.003347f, 0.003677f, 0.004025f, 0.004391f, 0.004777f, 0.005182f, 0.005605f, 0.006049f, 0.006512f, 0.006995f, 0.007499f,
		0.008023f, 0.008568f, 0.009134f, 0.009721f, 0.010330f, 0.010960f, 0.011612f, 0.012286f, 0.012983f, 0.013702f, 0.014444f,
		0.015209f, 0.015996f, 0.016807f, 0.017642f, 0.018500f, 0.019382f, 0.020289f, 0.021219f, 0.022174f, 0.023153f, 0.024158f,
		0.025187f, 0.026241f, 0.027321f, 0.028426f, 0.029557f, 0.030713f, 0.031896f, 0.033105f, 0.034340f, 0.035601f, 0.036889f,
		0.038204f, 0.039546f, 0.040915f, 0.042311f, 0.043735f, 0.045186f, 0.046665f, 0.048172f, 0.049707f, 0.051269f, 0.052861f,
		0.054480f, 0.056128f, 0.057805f, 0.059511f, 0.061246f, 0.063010f, 0.064803f, 0.066626f, 0.068478f, 0.070360f, 0.072272f,
		0.074214f, 0.076185f, 0.078187f, 0.080220f, 0.082283f, 0.084376f, 0.086500f, 0.088656f, 0.090842f, 0.093059f, 0.095307f,
		0.097587f, 0.099899f, 0.102242f, 0.104616f, 0.107023f, 0.109462f, 0.111932f, 0.114435f, 0.116971f, 0.119538f, 0.122139f,
		0.124772f, 0.127438f, 0.130136f, 0.132868f, 0.135633f, 0.138432f, 0.141263f, 0.144128f, 0.147027f, 0.149960f, 0.152926f,
		0.155926f, 0.158961f, 0.162029f, 0.165132f, 0.168269f, 0.171441f, 0.174647f, 0.177888f, 0.181164f, 0.184475f, 0.187821f,
		0.191202f, 0.194618f, 0.198069f, 0.201556f, 0.205079f, 0.208637f, 0.212231f, 0.215861f, 0.219526f, 0.223228f, 0.226966f,
		0.230740f, 0.234551f, 0.238398f, 0.242281f, 0.246201f, 0.250158f, 0.254152f, 0.258183f, 0.262251f, 0.266356f, 0.270498f,
		0.274677f, 0.278894f, 0.283149f, 0.287441f, 0.291771f, 0.296138f, 0.300544f, 0.304987f, 0.309469f, 0.313989f, 0.318547f,
		0.323143f, 0.327778f, 0.332452f, 0.337164f, 0.341914f, 0.346704f, 0.351533f, 0.356400f, 0.361307f, 0.366253f, 0.371238f,
		0.376262f, 0.381326f, 0.386430f, 0.391573f, 0.396755f, 0.401978f, 0.407240f, 0.412543f, 0.417885f, 0.423268f, 0.428691f,
		0.434154f, 0.439657f, 0.445201f, 0.450786f, 0.456411f, 0.462077f, 0.467784f, 0.473532f, 0.479320f, 0.485150f, 0.491021f,
		0.496933f, 0.502887f, 0.508881f, 0.514918f, 0.520996f, 0.527115f, 0.533276f, 0.539480f, 0.545725f, 0.552011f, 0.558340f,
		0.564712f, 0.571125f, 0.577581f, 0.584078f, 0.590619f, 0.597202f, 0.603827f, 0.610496f, 0.617207f, 0.623960f, 0.630757f,
		0.637597f, 0.644480f, 0.651406f, 0.658375f, 0.665387f, 0.672443f, 0.679543f, 0.686685f, 0.693872f, 0.701102f, 0.708376f,
		0.715694f, 0.723055f, 0.730461f, 0.737911f, 0.745404f, 0.752942f, 0.760525f, 0.768151f, 0.775822f, 0.783538f, 0.791298f,
		0.799103f, 0.806952f, 0.814847f, 0.822786f, 0.830770f, 0.838799f, 0.846873f, 0.854993f, 0.863157f, 0.871367f, 0.879622f,
		0.887923f, 0.896269f, 0.904661f, 0.913099f, 0.921582f, 0.930111f, 0.938686f, 0.947307f, 0.955974f, 0.964686f, 0.973445f,
		0.982251f, 0.991102f, 1.0f
	};

	typedef union
	{
		mu_uint32 u;
		mu_float f;
	} FP32;

	const mu_uint32 fp32_to_srgb8_tab4[104] = {
		0x0073000d, 0x007a000d, 0x0080000d, 0x0087000d, 0x008d000d, 0x0094000d, 0x009a000d, 0x00a1000d,
		0x00a7001a, 0x00b4001a, 0x00c1001a, 0x00ce001a, 0x00da001a, 0x00e7001a, 0x00f4001a, 0x0101001a,
		0x010e0033, 0x01280033, 0x01410033, 0x015b0033, 0x01750033, 0x018f0033, 0x01a80033, 0x01c20033,
		0x01dc0067, 0x020f0067, 0x02430067, 0x02760067, 0x02aa0067, 0x02dd0067, 0x03110067, 0x03440067,
		0x037800ce, 0x03df00ce, 0x044600ce, 0x04ad00ce, 0x051400ce, 0x057b00c5, 0x05dd00bc, 0x063b00b5,
		0x06970158, 0x07420142, 0x07e30130, 0x087b0120, 0x090b0112, 0x09940106, 0x0a1700fc, 0x0a9500f2,
		0x0b0f01cb, 0x0bf401ae, 0x0ccb0195, 0x0d950180, 0x0e56016e, 0x0f0d015e, 0x0fbc0150, 0x10630143,
		0x11070264, 0x1238023e, 0x1357021d, 0x14660201, 0x156601e9, 0x165a01d3, 0x174401c0, 0x182401af,
		0x18fe0331, 0x1a9602fe, 0x1c1502d2, 0x1d7e02ad, 0x1ed4028d, 0x201a0270, 0x21520256, 0x227d0240,
		0x239f0443, 0x25c003fe, 0x27bf03c4, 0x29a10392, 0x2b6a0367, 0x2d1d0341, 0x2ebe031f, 0x304d0300,
		0x31d105b0, 0x34a80555, 0x37520507, 0x39d504c5, 0x3c37048b, 0x3e7c0458, 0x40a8042a, 0x42bd0401,
		0x44c20798, 0x488e071e, 0x4c1c06b6, 0x4f76065d, 0x52a50610, 0x55ac05cc, 0x5892058f, 0x5b590559,
		0x5e0c0a23, 0x631c0980, 0x67db08f6, 0x6c55087f, 0x70940818, 0x74a007bd, 0x787d076c, 0x7c330723,
	};

	mu_uint8 linear_to_srgb_uchar(mu_float in)
	{
		static const FP32 almostone = { 0x3f7fffff }; // 1-eps
		static const FP32 minval = { (127 - 13) << 23 };
		mu_uint32 tab, bias, scale, t;
		FP32 f;

		// Clamp to [2^(-13), 1-eps]; these two values map to 0 and 1, respectively.
		// The tests are carefully written so that NaNs map to 0, same as in the reference
		// implementation.
		if (!(in > minval.f)) // written this way to catch NaNs
			in = minval.f;
		if (in > almostone.f)
			in = almostone.f;

		// Do the table lookup and unpack bias, scale
		f.f = in;
		tab = fp32_to_srgb8_tab4[(f.u - minval.u) >> 20];
		bias = (tab >> 16) << 9;
		scale = tab & 0xffff;

		// Grab next-highest mantissa bits and perform linear interpolation
		t = (f.u >> 12) & 0xff;
		return (mu_uint8)((bias + scale * t) >> 16);
	}

	/// Returns metadata for the texture at the given URI or empty rectangle if texture is not found
	Noesis::TextureInfo TextureProvider::GetTextureInfo(const Noesis::Uri &uri)
	{
		Noesis::TextureInfo info;
		const mu_utf8string filename = SupportPathUTF8 + GetResourcesPath() + uri.Str();
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
		const mu_utf8string filename = SupportPathUTF8 + GetResourcesPath() + uri.Str();
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
		const auto components = MUTextures::CalculateComponentsCount(bitmap);
		const auto pixels = width * height;

		if (components != 4)
		{
			FIBITMAP *newBitmap = FreeImage_ConvertTo32Bits(bitmap);
			FreeImage_Unload(bitmap);
			if (newBitmap == nullptr)
			{
				return nullptr;
			}
			bitmap = newBitmap;
		}

		const auto bpp = FreeImage_GetBPP(bitmap);
		const auto bytesPerPixel = bpp >> 3;

#if FREEIMAGE_COLORORDER == FREEIMAGE_COLORORDER_BGR
		if (bpp == 24 || bpp == 32) {
			for (mu_uint32 y = 0; y < height; y++) {
				mu_uint8 *pixels = FreeImage_GetScanLine(bitmap, y);
				for (mu_uint32 x = 0; x < width; x++) {
					SwapValue(pixels[0], pixels[2]);
					pixels += bytesPerPixel;
				}
			}
		}
#endif

		//     N=#comp     components
		//       1           grey
		//       2           grey, alpha
		//       3           red, green, blue
		//       4           red, green, blue, alpha
		const mu_boolean hasAlpha = components == 2 || components == 4;

		// Premultiply alpha
		if (hasAlpha)
		{
			constexpr mu_uint32 Red = 0;
			constexpr mu_uint32 Green = 1;
			constexpr mu_uint32 Blue = 2;
			constexpr mu_uint32 Alpha = 3;

			if (device->GetCaps().linearRendering)
			{
				for (mu_uint32 y = 0; y < height; y++)
				{
					mu_uint8 *pixels = FreeImage_GetScanLine(bitmap, y);
					for (mu_uint32 x = 0; x < width; x++)
					{
						const mu_float a = static_cast<mu_float>(pixels[Alpha]) / 255.0f;
						pixels[Red] = linear_to_srgb_uchar(srgb_uchar_to_linear_float[pixels[Red]] * a);
						pixels[Green] = linear_to_srgb_uchar(srgb_uchar_to_linear_float[pixels[Green]] * a);
						pixels[Blue] = linear_to_srgb_uchar(srgb_uchar_to_linear_float[pixels[Blue]] * a);

						pixels += bytesPerPixel;
					}
				}
			}
			else
			{
				if (!FreeImage_PreMultiplyWithAlpha(bitmap))
				{
					return nullptr;
				}
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