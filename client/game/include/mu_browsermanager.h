#ifndef __MU_BROWSERMANAGER_H__
#define __MU_BROWSERMANAGER_H__

#pragma once

#pragma pack(4)
struct NBrowserVertex
{
	glm::vec2 Position;
};
#pragma pack()

namespace MUBrowserManager
{
    const mu_int32 TryInitializeAsChildren(mu_int32 argc, mu_char **argv, void *instance);
    const mu_boolean Initialize(mu_int32 argc, mu_char **argv, void *instance);
	const mu_boolean InitializeBrowser(const mu_utf8string url);
	const mu_boolean ReloadShaders();
    void Destroy();
	void DestroyBrowser();

	void Update();
	void Render();
    const mu_boolean ProcessEvent(const SDL_Event *event);
};

#endif