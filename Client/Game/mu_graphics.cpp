#include "stdafx.h"
#include "mu_graphics.h"
#include "mu_window.h"
#include "mu_config.h"

#include <SDL_syswm.h>

#include <NativeWindow.h>
#if D3D11_SUPPORTED
#include <EngineFactoryD3D11.h>
#endif
#if D3D12_SUPPORTED
#include <EngineFactoryD3D12.h>
#endif
#if GL_SUPPORTED || GLES_SUPPORTED
#include <EngineFactoryOpenGL.h>
#endif
#if VULKAN_SUPPORTED
#include <EngineFactoryVk.h>
#endif
#if METAL_SUPPORTED
#include <EngineFactoryMtl.h>
#endif

namespace MUGraphics
{
    Diligent::Int32 ValidationLevel = -1;
    Diligent::Uint32 AdapterID = Diligent::DEFAULT_ADAPTER_ID;
    Diligent::ADAPTER_TYPE AdapterType = Diligent::ADAPTER_TYPE_UNKNOWN;
    Diligent::RENDER_DEVICE_TYPE DeviceType = Diligent::RENDER_DEVICE_TYPE_UNDEFINED;
    Diligent::Uint32 NumImmediateContexts = 0;

    Diligent::SwapChainDesc SwapChainInitDesc;
    Diligent::GraphicsAdapterInfo AdapterAttribs;
    std::vector<Diligent::DisplayModeAttribs> DisplayModes;

    Diligent::RefCntAutoPtr<Diligent::IEngineFactory> EngineFactory;
    Diligent::RefCntAutoPtr<Diligent::IRenderDevice> Device;
    Diligent::RefCntAutoPtr<Diligent::ISwapChain> SwapChain;
    std::vector<Diligent::RefCntAutoPtr<Diligent::IDeviceContext>> DeviceContexts;
    mu_uint32 CurrentDeviceContext = 0;

    std::unique_ptr<NRenderManager> RenderManager;

    const Diligent::TEXTURE_FORMAT DesiredColorFormat = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
    constexpr mu_boolean ForceNonSeprblProgs = false;

    const mu_boolean InitializeEngine(const Diligent::NativeWindow *window)
    {
#if D3D11_SUPPORTED || D3D12_SUPPORTED || VULKAN_SUPPORTED
        auto findAdapter = [](auto *pFactory, Diligent::Version GraphicsAPIVersion, Diligent::GraphicsAdapterInfo &AdapterAttribs) {
            Diligent::Uint32 numAdapters = 0;
            pFactory->EnumerateAdapters(GraphicsAPIVersion, numAdapters, nullptr);
            std::vector<Diligent::GraphicsAdapterInfo> adapters(numAdapters);
            if (numAdapters > 0)
                pFactory->EnumerateAdapters(GraphicsAPIVersion, numAdapters, adapters.data());
            else
                LOG_ERROR_AND_THROW("Failed to find compatible hardware adapters");

            auto adapterId = AdapterID;
            if (adapterId != Diligent::DEFAULT_ADAPTER_ID)
            {
                if (adapterId < adapters.size())
                {
                    AdapterType = adapters[adapterId].Type;
                }
                else
                {
                    LOG_ERROR_MESSAGE("Adapter ID (", adapterId, ") is invalid. Only ", adapters.size(), " compatible ", (adapters.size() == 1 ? "adapter" : "adapters"), " present in the system");
                    adapterId = Diligent::DEFAULT_ADAPTER_ID;
                }
            }

            if (adapterId == Diligent::DEFAULT_ADAPTER_ID && AdapterType != Diligent::ADAPTER_TYPE_UNKNOWN)
            {
                for (Diligent::Uint32 i = 0; i < adapters.size(); ++i)
                {
                    if (adapters[i].Type == AdapterType)
                    {
                        adapterId = i;
                        break;
                    }
                }
                if (adapterId == Diligent::DEFAULT_ADAPTER_ID)
                    LOG_WARNING_MESSAGE("Unable to find the requested adapter type. Using default adapter.");
            }

            if (adapterId == Diligent::DEFAULT_ADAPTER_ID)
            {
                AdapterType = Diligent::ADAPTER_TYPE_UNKNOWN;
                for (Uint32 i = 0; i < adapters.size(); ++i)
                {
                    const auto &adapterInfo = adapters[i];
                    const auto  adapterType = adapterInfo.Type;
                    static_assert((Diligent::ADAPTER_TYPE_DISCRETE > Diligent::ADAPTER_TYPE_INTEGRATED &&
                                   Diligent::ADAPTER_TYPE_INTEGRATED > Diligent::ADAPTER_TYPE_SOFTWARE &&
                                   Diligent::ADAPTER_TYPE_SOFTWARE > Diligent::ADAPTER_TYPE_UNKNOWN),
                                  "Unexpected ADAPTER_TYPE enum ordering");
                    if (adapterType > AdapterType)
                    {
                        // Prefer Discrete over Integrated over Software
                        AdapterType = adapterType;
                        adapterId = i;
                    }
                    else if (adapterType == AdapterType)
                    {
                        // Select adapter with more memory
                        const auto &newAdapterMem = adapterInfo.Memory;
                        const auto  newTotalMemory = newAdapterMem.LocalMemory + newAdapterMem.HostVisibleMemory + newAdapterMem.UnifiedMemory;
                        const auto &currAdapterMem = adapters[adapterId].Memory;
                        const auto  currTotalMemory = currAdapterMem.LocalMemory + currAdapterMem.HostVisibleMemory + currAdapterMem.UnifiedMemory;
                        if (newTotalMemory > currTotalMemory)
                        {
                            adapterId = i;
                        }
                    }
                }
            }

            if (adapterId != Diligent::DEFAULT_ADAPTER_ID)
            {
                AdapterAttribs = adapters[adapterId];
                LOG_INFO_MESSAGE("Using adapter ", adapterId, ": '", AdapterAttribs.Description, "'");
            }

            return adapterId;
        };
#endif

        try
        {
            Diligent::Uint32 numImmediateContexts = 0;
            std::vector<Diligent::IDeviceContext *> ppContexts;
            switch (DeviceType)
            {
#if D3D11_SUPPORTED
            case Diligent::RENDER_DEVICE_TYPE_D3D11:
                {
#if ENGINE_DLL
                    // Load the dll and import GetEngineFactoryD3D11() function
                    auto GetEngineFactoryD3D11 = LoadGraphicsEngineD3D11();
#endif
                    auto *pFactoryD3D11 = Diligent::GetEngineFactoryD3D11();
                    EngineFactory = pFactoryD3D11;

                    Diligent::EngineD3D11CreateInfo EngineCI;
                    EngineCI.GraphicsAPIVersion = { 11, 0 };

#ifdef DILIGENT_DEBUG
                    EngineCI.SetValidationLevel(VALIDATION_LEVEL_2);
#endif
                    if (ValidationLevel >= 0)
                        EngineCI.SetValidationLevel(static_cast<Diligent::VALIDATION_LEVEL>(ValidationLevel));

                    EngineCI.AdapterId = findAdapter(pFactoryD3D11, EngineCI.GraphicsAPIVersion, AdapterAttribs);

                    if (AdapterType != Diligent::ADAPTER_TYPE_SOFTWARE && EngineCI.AdapterId != Diligent::DEFAULT_ADAPTER_ID)
                    {
                        // Display mode enumeration fails with error for software adapter
                        Diligent::Uint32 NumDisplayModes = 0;
                        pFactoryD3D11->EnumerateDisplayModes(EngineCI.GraphicsAPIVersion, EngineCI.AdapterId, 0, DesiredColorFormat, NumDisplayModes, nullptr);
                        DisplayModes.resize(NumDisplayModes);
                        pFactoryD3D11->EnumerateDisplayModes(EngineCI.GraphicsAPIVersion, EngineCI.AdapterId, 0, DesiredColorFormat, NumDisplayModes, DisplayModes.data());
                    }

                    numImmediateContexts = std::max(1u, EngineCI.NumImmediateContexts);
                    ppContexts.resize(size_t{ numImmediateContexts } + size_t{ EngineCI.NumDeferredContexts });
                    pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &Device, ppContexts.data());
                    if (!Device)
                    {
                        LOG_ERROR_AND_THROW("Unable to initialize Diligent Engine in Direct3D11 mode. The API may not be available, "
                                            "or required features may not be supported by this GPU/driver/OS version.");
                    }
                    if (window != nullptr)
                        pFactoryD3D11->CreateSwapChainD3D11(Device, ppContexts[0], SwapChainInitDesc, Diligent::FullScreenModeDesc{}, *window, &SwapChain);
                }
                break;
#endif

#if D3D12_SUPPORTED
            case Diligent::RENDER_DEVICE_TYPE_D3D12:
                {
#if ENGINE_DLL
                    // Load the dll and import GetEngineFactoryD3D12() function
                    auto GetEngineFactoryD3D12 = LoadGraphicsEngineD3D12();
#endif
                    auto *pFactoryD3D12 = Diligent::GetEngineFactoryD3D12();
                    if (!pFactoryD3D12->LoadD3D12())
                    {
                        LOG_ERROR_AND_THROW("Failed to load Direct3D12");
                    }
                    EngineFactory = pFactoryD3D12;

                    Diligent::EngineD3D12CreateInfo EngineCI;
                    EngineCI.GraphicsAPIVersion = { 11, 0 };
                    if (ValidationLevel >= 0)
                        EngineCI.SetValidationLevel(static_cast<Diligent::VALIDATION_LEVEL>(ValidationLevel));

                    try
                    {
                        EngineCI.AdapterId = findAdapter(pFactoryD3D12, EngineCI.GraphicsAPIVersion, AdapterAttribs);
                    }
                    catch (...)
                    {
                        return false;
                    }

                    if (AdapterType != Diligent::ADAPTER_TYPE_SOFTWARE && EngineCI.AdapterId != Diligent::DEFAULT_ADAPTER_ID)
                    {
                        // Display mode enumeration fails with error for software adapter
                        Uint32 NumDisplayModes = 0;
                        pFactoryD3D12->EnumerateDisplayModes(EngineCI.GraphicsAPIVersion, EngineCI.AdapterId, 0, DesiredColorFormat, NumDisplayModes, nullptr);
                        DisplayModes.resize(NumDisplayModes);
                        pFactoryD3D12->EnumerateDisplayModes(EngineCI.GraphicsAPIVersion, EngineCI.AdapterId, 0, DesiredColorFormat, NumDisplayModes, DisplayModes.data());
                    }

                    numImmediateContexts = std::max(1u, EngineCI.NumImmediateContexts);
                    ppContexts.resize(numImmediateContexts + EngineCI.NumDeferredContexts);
                    pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &Device, ppContexts.data());
                    if (!Device)
                    {
                        LOG_ERROR_AND_THROW("Unable to initialize Diligent Engine in Direct3D12 mode. The API may not be available, "
                                            "or required features may not be supported by this GPU/driver/OS version.");
                    }

                    if (!SwapChain && window != nullptr)
                        pFactoryD3D12->CreateSwapChainD3D12(Device, ppContexts[0], SwapChainInitDesc, Diligent::FullScreenModeDesc{}, *window, &SwapChain);
                }
                break;
#endif

#if GL_SUPPORTED || GLES_SUPPORTED
            case Diligent::RENDER_DEVICE_TYPE_GL:
            case Diligent::RENDER_DEVICE_TYPE_GLES:
                {
#if NEXTMU_OPERATING_SYSTEM != NEXTMU_OS_MACOS
                    VERIFY_EXPR(pWindow != nullptr);
#endif
#if EXPLICITLY_LOAD_ENGINE_GL_DLL
                    // Load the dll and import GetEngineFactoryOpenGL() function
                    auto GetEngineFactoryOpenGL = LoadGraphicsEngineOpenGL();
#endif
                    auto *pFactoryOpenGL = Diligent::GetEngineFactoryOpenGL();
                    EngineFactory = pFactoryOpenGL;

                    Diligent::EngineGLCreateInfo EngineCI;
                    EngineCI.Window = *window;

                    if (ValidationLevel >= 0)
                        EngineCI.SetValidationLevel(static_cast<Diligent::VALIDATION_LEVEL>(ValidationLevel));

                    if (ForceNonSeprblProgs)
                        EngineCI.Features.SeparablePrograms = Diligent::DEVICE_FEATURE_STATE_DISABLED;
                    if (EngineCI.NumDeferredContexts != 0)
                    {
                        LOG_WARNING_MESSAGE("Deferred contexts are not supported in OpenGL mode");
                        EngineCI.NumDeferredContexts = 0;
                    }

                    numImmediateContexts = 1;
                    ppContexts.resize(numImmediateContexts + EngineCI.NumDeferredContexts);
                    pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &Device, ppContexts.data(), SwapChainInitDesc, &SwapChain);
                    if (!Device)
                    {
                        LOG_ERROR_AND_THROW("Unable to initialize Diligent Engine in OpenGL mode. The API may not be available, "
                                            "or required features may not be supported by this GPU/driver/OS version.");
                    }
                }
                break;
#endif

#if VULKAN_SUPPORTED
            case Diligent::RENDER_DEVICE_TYPE_VULKAN:
                {
#if EXPLICITLY_LOAD_ENGINE_VK_DLL
                    // Load the dll and import GetEngineFactoryVk() function
                    auto GetEngineFactoryVk = LoadGraphicsEngineVk();
#endif
                    Diligent::EngineVkCreateInfo EngineCI;
                    if (ValidationLevel >= 0)
                        EngineCI.SetValidationLevel(static_cast<Diligent::VALIDATION_LEVEL>(ValidationLevel));

                    const char *const ppIgnoreDebugMessages[] = //
                    {
                        // Validation Performance Warning: [ UNASSIGNED-CoreValidation-Shader-OutputNotConsumed ]
                        // vertex shader writes to output location 1.0 which is not consumed by fragment shader
                        "UNASSIGNED-CoreValidation-Shader-OutputNotConsumed" //
                    };
                    EngineCI.ppIgnoreDebugMessageNames = ppIgnoreDebugMessages;
                    EngineCI.IgnoreDebugMessageCount = _countof(ppIgnoreDebugMessages);

                    auto *pFactoryVk = Diligent::GetEngineFactoryVk();
                    EngineFactory = pFactoryVk;

                    EngineCI.AdapterId = findAdapter(pFactoryVk, EngineCI.GraphicsAPIVersion, AdapterAttribs);

                    numImmediateContexts = std::max(1u, EngineCI.NumImmediateContexts);
                    ppContexts.resize(numImmediateContexts + EngineCI.NumDeferredContexts);
                    pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &Device, ppContexts.data());
                    if (!Device)
                    {
                        LOG_ERROR_AND_THROW("Unable to initialize Diligent Engine in Vulkan mode. The API may not be available, "
                                            "or required features may not be supported by this GPU/driver/OS version.");
                    }

                    if (!SwapChain && window != nullptr)
                        pFactoryVk->CreateSwapChainVk(Device, ppContexts[0], SwapChainInitDesc, *window, &SwapChain);
                }
                break;
#endif


#if METAL_SUPPORTED
            case Diligent::RENDER_DEVICE_TYPE_METAL:
                {
                    Diligent::EngineMtlCreateInfo EngineCI;
                    if (ValidationLevel >= 0)
                        EngineCI.SetValidationLevel(static_cast<Diligent::VALIDATION_LEVEL>(ValidationLevel));

                    auto *pFactoryMtl = Diligent::GetEngineFactoryMtl();
                    EngineFactory = pFactoryMtl;

                    numImmediateContexts = std::max(1u, EngineCI.NumImmediateContexts);
                    ppContexts.resize(numImmediateContexts + EngineCI.NumDeferredContexts);
                    pFactoryMtl->CreateDeviceAndContextsMtl(EngineCI, &Device, ppContexts.data());
                    if (!Device)
                    {
                        LOG_ERROR_AND_THROW("Unable to initialize Diligent Engine in Metal mode. The API may not be available, "
                                            "or required features may not be supported by this GPU/driver/OS version.");
                    }

                    if (!SwapChain && window != nullptr)
                        pFactoryMtl->CreateSwapChainMtl(Device, ppContexts[0], SwapChainInitDesc, *window, &SwapChain);
                }
                break;
#endif

            default:
                LOG_ERROR_AND_THROW("Unknown device type");
                break;
            }

            NumImmediateContexts = numImmediateContexts;
            DeviceContexts.resize(ppContexts.size());
            for (size_t i = 0; i < ppContexts.size(); ++i)
                DeviceContexts[i].Attach(ppContexts[i]);

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    const mu_boolean Initialize()
    {
        auto *sdlWindow = MUWindow::GetWindow();

        SDL_SysWMinfo wmi;
        SDL_VERSION(&wmi.version);
        if (SDL_GetWindowWMInfo(sdlWindow, &wmi) == SDL_FALSE)
        {
            return false;
        }

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
        Diligent::NativeWindow window(wmi.info.win.window);
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
        init.platformData.ndt = wmi.info.x11.display;
        init.platformData.nwh = (void *)(uintptr_t)wmi.info.x11.window;
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
        init.platformData.nwh = wmi.info.cocoa.window;
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
        init.platformData.nwh = wmi.info.android.window;
#endif

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
        // We need at least 3 buffers in Metal to avoid massive
        // performance degradation in full screen mode.
        // https://github.com/KhronosGroup/MoltenVK/issues/808
        SwapChainInitDesc.BufferCount = 3;
#endif
        SwapChainInitDesc.ColorBufferFormat = DesiredColorFormat;
        SwapChainInitDesc.DepthBufferFormat = Diligent::TEX_FORMAT_D32_FLOAT_S8X24_UINT;

        const Diligent::RENDER_DEVICE_TYPE deviceTypes[] = {
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
            Diligent::RENDER_DEVICE_TYPE_VULKAN,
#endif
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
            Diligent::RENDER_DEVICE_TYPE_D3D12,
#endif
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
            Diligent::RENDER_DEVICE_TYPE_D3D11,
#endif
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
            Diligent::RENDER_DEVICE_TYPE_METAL,
#endif
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
            Diligent::RENDER_DEVICE_TYPE_GL,
#endif
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID || NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
            Diligent::RENDER_DEVICE_TYPE_GLES,
#endif
        };
        mu_boolean initialized = false;
        for (mu_uint32 n = 0; n < mu_countof(deviceTypes); ++n)
        {
            DeviceType = deviceTypes[n];
            if (InitializeEngine(&window) == true)
            {
                initialized = true;
                break;
            }
        }
        if (initialized == false)
        {
            return false;
        }

        RenderManager.reset(new (std::nothrow) NRenderManager());
        CreateInputLayouts();
        CreatePipelineResources();
        FreeImage_Initialise(true);

        return true;
    }

    void Destroy()
    {
        RenderManager.reset();
        DestroyShaderBindings();
        DestroyPipelines();
        DeviceContexts.clear();
        SwapChain.Release();
        Device.Release();
        EngineFactory.Release();
        FreeImage_DeInitialise();
    }

    Diligent::IRenderDevice *GetDevice()
    {
        return Device.RawPtr();
    }

    Diligent::ISwapChain *GetSwapChain()
    {
        return SwapChain.RawPtr();
    }

    Diligent::IDeviceContext *GetImmediateContext()
    {
        return DeviceContexts[CurrentDeviceContext].RawPtr();
    }

    NRenderManager *GetRenderManager()
    {
        return RenderManager.get();
    }

	const char *GetShaderFolder()
	{
        return "/";
#if 0
#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_WINDOWS
		return "/windows/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_LINUX
		return "/linux/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_MACOS
		return "/macos/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
		return "/android/";
#elif NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_IOS
		return "/ios/";
#endif
#endif
	}

	const char *GetShaderExtension()
	{
        return "";
#if 0
		switch (DeviceType)
		{
		default: mu_error("Not supported renderer");
		case Diligent::RENDER_DEVICE_TYPE_GLES: return ".gles";
		case Diligent::RENDER_DEVICE_TYPE_GL: return ".gl";
		case Diligent::RENDER_DEVICE_TYPE_D3D11: return ".d3d";
		case Diligent::RENDER_DEVICE_TYPE_D3D12: return ".d3d";
		case Diligent::RENDER_DEVICE_TYPE_VULKAN: return ".vk";
        case Diligent::RENDER_DEVICE_TYPE_METAL: return ".mtl";
		}
#endif
	}
};