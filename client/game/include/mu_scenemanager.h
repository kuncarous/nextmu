#ifndef __MU_SCENEMANAGER_H__
#define __MU_SCENEMANAGER_H__

#pragma once

#include "scn_base.h"

namespace MUSceneManager
{
    const mu_boolean Initialize();
    void Destroy();

	void Update();
	void SetQueueScene(NSceneBasePtr scene);
    
    mu_boolean SetScene(NSceneBasePtr scene);
    NSceneBase *GetScene();
};

#endif