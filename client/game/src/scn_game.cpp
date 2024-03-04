#include "mu_precompiled.h"
#include "scn_game.h"
#include "mu_skeletonmanager.h"

mu_boolean NGameScene::Load()
{
    return true;
}

void NGameScene::Unload()
{
    
}
    
void NGameScene::Run()
{
	MUSkeletonManager::Update();
}