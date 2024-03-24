#include "mu_precompiled.h"
#include "mu_angelscript.h"
#include "scriptstdstring.h"
#include "scriptarray.h"
#include "scriptmath.h"
#include "mu_threadsmanager.h"
#include "as_scriptbuilder.h"
#include "mu_modelrenderer.h"
#include "mu_state.h"
#include <queue>

namespace MUAngelScript
{
	AngelScript::asIScriptEngine *Engine = nullptr;
	std::mutex ContextMutex;
	std::vector<AngelScript::asIScriptContext *> Contexts;
	std::queue<AngelScript::asIScriptContext *> AvailableContexts;
	std::map<mu_utf8string, ASModuleScript> Modules;

	void MessageCallback(const AngelScript::asSMessageInfo &msg);

	const mu_boolean Initialize()
	{
		Engine = AngelScript::asCreateScriptEngine();
		if (!Engine)
		{
			mu_error("failed to create angelscript engine");
			return false;
		}

		auto r = Engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, AngelScript::asCALL_CDECL);
		if (r < 0)
		{
			mu_error("failed to set angelscript message callback");
			return false;
		}

		const auto threadsCount = MUThreadsManager::GetThreadsCount();
		Contexts.resize(threadsCount);
		for (auto &context : Contexts)
		{
			context = Engine->CreateContext();
			if (context == nullptr)
			{
				mu_error("failed to create angelscript context");
				return false;
			}

			AvailableContexts.push(context);
		}

		AngelScript::RegisterStdString(Engine);
		AngelScript::RegisterScriptArray(Engine, false);
		AngelScript::RegisterStdStringUtils(Engine);
		AngelScript::RegisterScriptMath(Engine);
		
		// GLM
		{
			// glm::vec2
			r = Engine->RegisterObjectType("glm_vec2", sizeof(glm::vec2), AngelScript::asOBJ_REF | AngelScript::asOBJ_NOCOUNT);
			mu_assert(r >= 0 || !"failed to register object type");
			r = Engine->RegisterObjectProperty("glm_vec2", "float x", offsetof(glm::vec2, x));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("glm_vec2", "float y", offsetof(glm::vec2, y));
			mu_assert(r >= 0 || !"failed to register object property");

			// glm::vec3
			r = Engine->RegisterObjectType("glm_vec3", sizeof(glm::vec3), AngelScript::asOBJ_REF | AngelScript::asOBJ_NOCOUNT);
			mu_assert(r >= 0 || !"failed to register object type");
			r = Engine->RegisterObjectProperty("glm_vec3", "float x", offsetof(glm::vec3, x));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("glm_vec3", "float y", offsetof(glm::vec3, y));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("glm_vec3", "float z", offsetof(glm::vec3, z));
			mu_assert(r >= 0 || !"failed to register object property");

			// glm::vec4
			r = Engine->RegisterObjectType("glm_vec4", sizeof(glm::vec4), AngelScript::asOBJ_REF | AngelScript::asOBJ_NOCOUNT);
			mu_assert(r >= 0 || !"failed to register object type");
			r = Engine->RegisterObjectProperty("glm_vec4", "float x", offsetof(glm::vec4, x));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("glm_vec4", "float y", offsetof(glm::vec4, y));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("glm_vec4", "float z", offsetof(glm::vec4, z));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("glm_vec4", "float w", offsetof(glm::vec4, w));
			mu_assert(r >= 0 || !"failed to register object property");
		}
		
		// Global
		{
			r = Engine->RegisterGlobalFunction("float GetWorldTime()", AngelScript::asFUNCTIONPR(MUState::GetWorldTime, (), const mu_float), AngelScript::asCALL_CDECL);
			mu_assert(r >= 0 || !"failed to register global function");
		}

		// Models
		{
			// NRenderConfig
			r = Engine->RegisterObjectType("NRenderConfig", sizeof(NRenderConfig), AngelScript::asOBJ_REF | AngelScript::asOBJ_NOCOUNT);
			mu_assert(r >= 0 || !"failed to register object type");
			r = Engine->RegisterObjectProperty("NRenderConfig", "uint32 BoneOffset", offsetof(NRenderConfig, BoneOffset));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NRenderConfig", "glm_vec3 BodyOrigin", offsetof(NRenderConfig, BodyOrigin));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NRenderConfig", "float BodyScale", offsetof(NRenderConfig, BodyScale));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NRenderConfig", "bool EnableLight", offsetof(NRenderConfig, EnableLight));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NRenderConfig", "glm_vec4 BodyLight", offsetof(NRenderConfig, BodyLight));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NRenderConfig", "float BlendMeshLight", offsetof(NRenderConfig, BlendMeshLight));
			mu_assert(r >= 0 || !"failed to register object property");

			// NModelSettings
			r = Engine->RegisterObjectType("NModelSettings", sizeof(NModelSettings), AngelScript::asOBJ_REF | AngelScript::asOBJ_NOCOUNT);
			mu_assert(r >= 0 || !"failed to register object type");
			r = Engine->RegisterObjectProperty("NModelSettings", "glm_vec4 LightPosition", offsetof(NModelSettings, LightPosition));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "glm_vec4 BodyLight", offsetof(NModelSettings, BodyLight));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "glm_vec4 BodyOrigin", offsetof(NModelSettings, BodyOrigin));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float BoneOffset", offsetof(NModelSettings, BoneOffset));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float NormalScale", offsetof(NModelSettings, NormalScale));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float EnableLight", offsetof(NModelSettings, EnableLight));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float AlphaTest", offsetof(NModelSettings, AlphaTest));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float PremultiplyAlpha", offsetof(NModelSettings, PremultiplyAlpha));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float WorldTime", offsetof(NModelSettings, WorldTime));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float ZTestRef", offsetof(NModelSettings, ZTestRef));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float BlendMeshLight", offsetof(NModelSettings, BlendMeshLight));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "glm_vec2 BlendTexCoord", offsetof(NModelSettings, BlendTexCoord));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float Dummy2", offsetof(NModelSettings, Dummy2));
			mu_assert(r >= 0 || !"failed to register object property");
			r = Engine->RegisterObjectProperty("NModelSettings", "float Dummy3", offsetof(NModelSettings, Dummy3));
			mu_assert(r >= 0 || !"failed to register object property");
		}

		return true;
	}

	void Destroy()
	{
		for (auto *context : Contexts)
		{
			if (context == nullptr) continue;
			context->Release();
		}
		Contexts.clear();
		Modules.clear();

		if (Engine)
		{
			Engine->Release();
			Engine = nullptr;
		}
	}

	AngelScript::asIScriptContext *GetAvailableContext()
	{
		std::lock_guard lock(ContextMutex);
		if (AvailableContexts.empty() == false)
		{
			auto *context = AvailableContexts.front();
			AvailableContexts.pop();
			return context;
		}

		auto *context = Engine->CreateContext();
		Contexts.push_back(context);

		return context;
	}

	void ReleaseContext(AngelScript::asIScriptContext *context)
	{
		std::lock_guard lock(ContextMutex);
		AvailableContexts.push(context);
	}

	void ModuleDeleter(AngelScript::asIScriptModule *module)
	{
		module->Discard();
	}

	ASModuleScript CompileScript(mu_utf8string filename)
	{
		filename = ResolveToRelativePath(filename);

		auto iter = Modules.find(filename);
		if (iter != Modules.end())
		{
			return iter->second;
		}

		AngelScript::CScriptBuilder builder;

		mu_int32 r = builder.StartNewModule(Engine, filename.c_str());
		if (r < 0)
		{
			return nullptr;
		}

		if (mu_rwexists<EGameDirectoryType::eSupport>(filename) == false)
		{
			return nullptr;
		}

		r = builder.AddSectionFromFile(filename.c_str());
		if (r < 0)
		{
			return nullptr;
		}

		r = builder.BuildModule();
		if (r < 0)
		{
			return nullptr;
		}

		auto module = ASModuleScript(builder.GetModule(), ModuleDeleter);
		Modules.insert(std::make_pair(filename, module));

		return module;
	}

	void MessageCallback(const AngelScript::asSMessageInfo &msg)
	{
		switch (msg.type)
		{
		case AngelScript::asMSGTYPE_INFORMATION:
		case AngelScript::asMSGTYPE_WARNING:
			{
				mu_info("{}({}, {}) : {}", msg.section, msg.row, msg.col, msg.message);
			}
			break;

		case AngelScript::asMSGTYPE_ERROR:
			{
				mu_error("{}({}, {}) : {}", msg.section, msg.row, msg.col, msg.message);
			}
			break;
		}
	}
}