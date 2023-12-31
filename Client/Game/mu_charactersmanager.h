#ifndef __MU_CHARACTERSMANAGER_H__
#define __MU_CHARACTERSMANAGER_H__

#pragma once

#include "t_charactersmanager_structs.h"
#include "t_character_structs.h"

namespace MUCharactersManager
{
	const mu_boolean Load();
	void Destroy();

	const NCharacterType GetTypeFromString(const mu_utf8string id);
	const NCharacterConfiguration *GetConfiguration(const mu_uint32 classId, const mu_uint32 subClassId);
}

#endif