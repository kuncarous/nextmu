#ifndef __T_CHARACTER_STRUCTS_H__
#define __T_CHARACTER_STRUCTS_H__

#pragma once

enum class CharacterType
{
	Character,
	Monster,
};

namespace TCharacter
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