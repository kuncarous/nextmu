#ifndef __MU_ANGELSCRIPT_H__
#define __MU_ANGELSCRIPT_H__

#pragma once

namespace MUAngelScript
{
	const mu_boolean Initialize();
	void Destroy();

	AngelScript::asIScriptContext *GetAvailableContext();
	void ReleaseContext(AngelScript::asIScriptContext *context);

	ASModuleScript CompileScript(mu_utf8string filename);
}

#endif