#ifndef __MU_ENVIRONMENT_CHARACTER_H__
#define __MU_ENVIRONMENT_CHARACTER_H__

#pragma once

enum class CharacterType
{
	Character,
	Monster,
};

namespace MUCharacter
{
	struct Settings
	{
		mu_key Key;
		CharacterType Type;
		mu_uint16 X, Y;
		mu_float Rotation;
	};
}

#endif