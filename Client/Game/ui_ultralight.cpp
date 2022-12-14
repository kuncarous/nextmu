#include "stdafx.h"
#include "ui_ultralight.h"
#include "ui_ultralight_sdl.h"
#include "ui_ultralight_filesystem.h"
#include "ui_ultralight_logger.h"
#include "mu_config.h"
#include "mu_resourcesmanager.h"

#if NEXTMU_UI_LIBRARY == NEXTMU_UI_ULTRALIGHT
#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>

#pragma comment(lib, "AppCore.lib")
#pragma comment(lib, "UltralightCore.lib")
#pragma comment(lib, "WebCore.lib")
#pragma comment(lib, "Ultralight.lib")

namespace UIUltralight
{
	RWFileSystem FileSystem;
	ConsoleLogger Logger;

	ultralight::RefPtr<ultralight::Renderer> Renderer;
	ultralight::RefPtr<ultralight::View> View;
	bgfx::TextureHandle Texture = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle Sampler = BGFX_INVALID_HANDLE;
	bgfx::UniformHandle ScreenUniform = BGFX_INVALID_HANDLE;
	bgfx::VertexBufferHandle VertexBuffer = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle IndexBuffer = BGFX_INVALID_HANDLE;
	bgfx::VertexLayout VertexLayout;

	// DON'T DESTROY THIS, it is being managed by the resources manager.
	bgfx::ProgramHandle Program = BGFX_INVALID_HANDLE;

	void Configure();
	void InitPlatform();
	const mu_boolean CreateRenderer();
	const mu_boolean CreateView();
	const mu_boolean CreateTexture();
	const mu_boolean GenerateBuffers();

	const mu_boolean Initialize()
	{
		Configure();
		InitPlatform();

		if (CreateRenderer() == false)
		{
			mu_error("failed to create ultraligh renderer");
			return false;
		}

		if (CreateView() == false)
		{
			mu_error("failed to create ultraligh view");
			return false;
		}

		if (CreateTexture() == false)
		{
			mu_error("failed to create ultraligh texture");
			return false;
		}

		if (GenerateBuffers() == false)
		{
			mu_error("failed to generate ultraligh buffers");
			return false;
		}

		return true;
	}

	void Destroy()
	{
		if (bgfx::isValid(Texture))
		{
			bgfx::destroy(Texture);
			Texture = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(Sampler))
		{
			bgfx::destroy(Sampler);
			Sampler = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(ScreenUniform))
		{
			bgfx::destroy(ScreenUniform);
			ScreenUniform = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(VertexBuffer))
		{
			bgfx::destroy(VertexBuffer);
			VertexBuffer = BGFX_INVALID_HANDLE;
		}

		if (bgfx::isValid(IndexBuffer))
		{
			bgfx::destroy(IndexBuffer);
			IndexBuffer = BGFX_INVALID_HANDLE;
		}

		View = nullptr;
		Renderer = nullptr;
	}

	void Configure()
	{
		ultralight::Config config;

		///
		/// We need to tell config where our resources are so it can 
		/// load our bundled SSL certificates to make HTTPS requests.
		///
		config.resource_path = "./resources/";

		///
		/// The GPU renderer should be disabled to render Views to a 
		/// pixel-buffer (Surface).
		///
		config.use_gpu_renderer = false;

		///
		/// You can set a custom DPI scale here. Default is 1.0 (100%)
		///
		config.device_scale = 1.0;

		///
		/// Pass our configuration to the Platform singleton so that
		/// the library can use it.
		///
		ultralight::Platform::instance().set_config(config);
	}

	void InitPlatform()
	{
		///
		/// Use the OS's native font loader
		///
		ultralight::Platform::instance().set_font_loader(ultralight::GetPlatformFontLoader());

		///
		/// Use the OS's native file loader, with a base directory of "."
		/// All file:/// URLs will load relative to this base directory.
		///
		ultralight::Platform::instance().set_file_system(&FileSystem);

		///
		/// Use the default logger (writes to a log file)
		///
		ultralight::Platform::instance().set_logger(&Logger);
	}

	const mu_boolean CreateRenderer()
	{
		///
		/// Create our Renderer (call this only once per application).
		/// 
		/// The Renderer singleton maintains the lifetime of the library
		/// and is required before creating any Views.
		///
		/// You should set up the Platform handlers before this.
		///
		Renderer = ultralight::Renderer::Create();

		return true;
	}

	const mu_boolean CreateView()
	{
		///
		/// Create an HTML view.
		///
		View = Renderer->CreateView(MUConfig::GetWindowWidth(), MUConfig::GetWindowHeight(), true, nullptr);

		///
		/// Load HTML from URL.
		///
		View->LoadURL("file:///ui/index.html");

		///
		/// Notify the View it has input focus (updates appearance).
		///
		View->Focus();
		
		return true;
	}

	const mu_boolean CreateTexture()
	{
		const mu_uint16 width = static_cast<mu_uint16>(MUConfig::GetWindowWidth());
		const mu_uint16 height = static_cast<mu_uint16>(MUConfig::GetWindowHeight());
		Texture = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_NONE | BGFX_SAMPLER_POINT);
		if (bgfx::isValid(Texture) == false)
		{
			return false;
		}

		Sampler = bgfx::createUniform("s_view", bgfx::UniformType::Sampler);
		if (bgfx::isValid(Sampler) == false)
		{
			return false;
		}

		ScreenUniform = bgfx::createUniform("u_screen", bgfx::UniformType::Vec4);
		if (bgfx::isValid(ScreenUniform) == false)
		{
			return false;
		}

		Program = MUResourcesManager::GetProgram("ultralight_view");

		return true;
	}

	void InitializeVertexLayout()
	{
		static mu_boolean initialized = false;
		if (initialized) return;
		initialized = true;
		VertexLayout
			.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Uint8)
			.end();
	}

	struct ViewVertex
	{
		mu_uint8 x, y;
	};
	ViewVertex Vertices[4] = {
		{
			.x = 0,
			.y = 0,
		},
		{
			.x = 1,
			.y = 0,
		},
		{
			.x = 1,
			.y = 1,
		},
		{
			.x = 0,
			.y = 1,
		}
	};
	mu_uint16 Indexes[6] = { 0, 1, 2, 0, 2, 3 };

	const mu_boolean GenerateBuffers()
	{
		InitializeVertexLayout();

		const bgfx::Memory *mem = bgfx::makeRef(Vertices, sizeof(Vertices));
		VertexBuffer = bgfx::createVertexBuffer(mem, VertexLayout);
		if (bgfx::isValid(VertexBuffer) == false)
		{
			return false;
		}

		mem = bgfx::makeRef(Indexes, sizeof(Indexes));
		IndexBuffer = bgfx::createIndexBuffer(mem);
		if (bgfx::isValid(VertexBuffer) == false)
		{
			return false;
		}

		return true;
	}

	void UpdateLogic()
	{
		///
		/// Give the library a chance to handle any pending tasks and timers.
		///
		///
		Renderer->Update();
	}

	void RenderOneFrame()
	{
		///
		/// Render all active Views (this updates the Surface for each View).
		///
		Renderer->Render();
	}

	void Upload()
	{
		///
		/// Get the pixel-buffer Surface for a View.
		///
		ultralight::Surface *surface = View->surface();

		///
		/// Cast it to a BitmapSurface.
		///
		ultralight::BitmapSurface *bitmap_surface = (ultralight::BitmapSurface *)surface;
		if (surface->dirty_bounds().IsEmpty())
		{
			return;
		}

		///
		/// Get the underlying bitmap.
		///
		ultralight::RefPtr<ultralight::Bitmap> bitmap = bitmap_surface->bitmap();

		///
		/// Lock the Bitmap to retrieve the raw pixels.
		/// The format is BGRA, 8-bpp, premultiplied alpha.
		///
		void *pixels = bitmap->LockPixels();

		///
		/// Get the bitmap dimensions.
		///
		uint32_t width = bitmap->width();
		uint32_t height = bitmap->height();
		uint32_t stride = bitmap->row_bytes();

		///
		/// Psuedo-code to upload our pixels to a GPU texture.
		///
		const bgfx::Memory *memory = bgfx::copy(pixels, height * stride);
		bgfx::updateTexture2D(Texture, 0, 0, 0, 0, width, height, memory);

		///
		/// Unlock the Bitmap when we are done.
		///
		bitmap->UnlockPixels();

		///
		/// Clear the dirty bounds.
		///
		surface->ClearDirtyBounds();
	}

	void Present()
	{
		Upload();

		const mu_uint16 width = MUConfig::GetWindowWidth();
		const mu_uint16 height = MUConfig::GetWindowHeight();
		const bgfx::Caps *caps = bgfx::getCaps();

		mu_float ortho[16];
		bx::mtxOrtho(
			ortho,
			0.0f, static_cast<mu_float>(width),
			static_cast<mu_float>(height), 0.0f,
			0.0f, 1000.0f,
			0.0f,
			caps->homogeneousDepth
		);

		constexpr bgfx::ViewId viewId = 255;
		bgfx::setViewTransform(viewId, nullptr, ortho);
		bgfx::setViewRect(viewId, 0, 0, width, height);

		bgfx::setState(
			BGFX_STATE_WRITE_RGB |
			BGFX_STATE_WRITE_A |
			BGFX_STATE_MSAA |
			BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
		);

		glm::vec4 screen(
			static_cast<mu_float>(MUConfig::GetWindowWidth()),
			static_cast<mu_float>(MUConfig::GetWindowHeight()),
			0.0f, 0.0f
		);
		bgfx::setUniform(ScreenUniform, glm::value_ptr(screen));
		bgfx::setTexture(0, Sampler, Texture);

		bgfx::setVertexBuffer(0, VertexBuffer);
		bgfx::setIndexBuffer(IndexBuffer);
		bgfx::submit(viewId, Program);
	}

	const mu_boolean OnEvent(const SDL_Event *event)
	{
		switch (event->type)
		{
		case SDL_MOUSEMOTION:
			{
				ultralight::MouseEvent uevent;
				uevent.type = ultralight::MouseEvent::kType_MouseMoved;
				uevent.x = event->motion.x;
				uevent.y = event->motion.y;

				View->FireMouseEvent(uevent);
			}
			return true;

		case SDL_MOUSEWHEEL:
			{
				ultralight::ScrollEvent uevent;
				uevent.type = ultralight::ScrollEvent::kType_ScrollByPixel;
				uevent.delta_x = event->wheel.x;
				uevent.delta_y = event->wheel.y;

				View->FireScrollEvent(uevent);
			}
			return true;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			{
				ultralight::MouseEvent::Button button = ultralight::MouseEvent::kButton_None;
				if (event->button.button == SDL_BUTTON_LEFT) { button = ultralight::MouseEvent::kButton_Left; }
				if (event->button.button == SDL_BUTTON_RIGHT) { button = ultralight::MouseEvent::kButton_Middle; }
				if (event->button.button == SDL_BUTTON_MIDDLE) { button = ultralight::MouseEvent::kButton_Right; }
				if (button == ultralight::MouseEvent::kButton_None)
					break;

				ultralight::MouseEvent uevent;
				uevent.type = event->type == SDL_MOUSEBUTTONDOWN ? ultralight::MouseEvent::kType_MouseDown : ultralight::MouseEvent::kType_MouseUp;
				uevent.x = event->button.x;
				uevent.y = event->button.y;
				uevent.button = button;

				View->FireMouseEvent(uevent);
			}
			return true;

		case SDL_TEXTINPUT:
			{
				ultralight::KeyEvent uevent;
				uevent.type = ultralight::KeyEvent::kType_Char;
				uevent.text = event->text.text;
				uevent.unmodified_text = event->text.text;

				View->FireKeyEvent(uevent);
			}
			return true;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
			{
				ultralight::KeyEvent uevent;
				uevent.type = event->type == SDL_KEYDOWN ? ultralight::KeyEvent::kType_RawKeyDown : ultralight::KeyEvent::kType_KeyUp;
				uevent.virtual_key_code = SDLKeyCodeToUltralightKeyCode(event->key.keysym.sym);
				uevent.native_key_code = event->key.keysym.scancode;
				ultralight::GetKeyIdentifierFromVirtualKeyCode(uevent.virtual_key_code, uevent.key_identifier);
				uevent.modifiers = SDLModsToUltralightMods(event->key.keysym.mod);
				uevent.is_auto_repeat = event->key.repeat > 0;
				uevent.is_keypad = SDLIsKeypad(event);

				View->FireKeyEvent(uevent);
			}
			return View->HasInputFocus();
		}

		return false;
	}
};
#endif