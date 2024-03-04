#include "mu_precompiled.h"
#include "mu_scenemanager.h"
#include "scn_intro.h"

namespace MUSceneManager
{
    NSceneBasePtr CurrentScene;
    NSceneBasePtr QueueScene;

    const mu_boolean Initialize()
    {
        CurrentScene.reset(new_nothrow NIntroScene());
        CurrentScene->Load();

        return true;
    }

    void Destroy()
    {
        if (CurrentScene != nullptr)
        {
            CurrentScene->Unload();
            CurrentScene.reset();
        }
    }

    void Update()
    {
        if (QueueScene != nullptr)
        {
            SetScene(std::move(QueueScene));
        }
    }

    void SetQueueScene(NSceneBasePtr scene)
    {
        QueueScene = std::move(scene);
    }

    mu_boolean SetScene(NSceneBasePtr scene)
    {
        if (CurrentScene != nullptr)
        {
            CurrentScene->Unload();
        }

        if (scene->Load() == false)
        {
            MUResourcesManager::SetResourcesManager(nullptr);
            return false;
        }

		CurrentScene = std::move(scene);
		return true;
    }
    
    NSceneBase *GetScene()
    {
        return CurrentScene.get();
    }
};