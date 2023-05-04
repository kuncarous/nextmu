#ifndef __MU_ANIMATIONSMANAGER_H__
#define __MU_ANIMATIONSMANAGER_H__

#pragma once

#include "ani_node.h"
#include "ani_input.h"

namespace MUAnimationsManager
{
	const mu_boolean Load();
	void Destroy();

	const NAnimationsRoot *GetAnimationsRoot(const mu_utf8string id);
	const mu_utf8string GetAnimation(const NAnimationsRoot *root, const NAnimationInput &input);
};

#endif