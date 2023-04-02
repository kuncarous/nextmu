#include "stdafx.h"
#include "mu_angelscript.h"
#include "scriptstdstring.h"
#include "scriptarray.h"
#include "scriptmath.h"

namespace MUAngelScript
{
	AngelScript::asIScriptEngine *Engine = nullptr;
	std::vector<AngelScript::asIScriptContext *> Contexts;

	void MessageCallback(const AngelScript::asSMessageInfo &msg);

	const mu_boolean Initialize()
	{
		Engine = AngelScript::asCreateScriptEngine();
		if (!Engine)
		{
			mu_error("failed to create angelscript engine");
			return false;
		}

		auto result = Engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, AngelScript::asCALL_CDECL);
		if (result < 0)
		{
			mu_error("failed to set angelscript message callback");
			return false;
		}

		AngelScript::RegisterStdString(Engine);
		AngelScript::RegisterStdStringUtils(Engine);
		AngelScript::RegisterScriptMath(Engine);

		return true;
	}

	void Destroy()
	{
		for (mu_uint32 n = 0; n < Contexts.size(); ++n)
		{
			Contexts[n]->Release();
		}
		Contexts.clear();

		if (Engine)
		{
			Engine->Release();
			Engine = nullptr;
		}
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