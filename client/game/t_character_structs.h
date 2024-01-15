#ifndef __T_CHARACTER_STRUCTS_H__
#define __T_CHARACTER_STRUCTS_H__

#pragma once

enum class CharacterType
{
	Character,
	Monster,
};

struct NCharacterType
{
	mu_uint16 Class = NInvalidUInt16;
	mu_uint16 SubClass = NInvalidUInt16;
};


namespace TCharacter
{
	struct Settings
	{
		mu_key Key;
		CharacterType Type;
		union
		{
			mu_uint32 MonsterType;
			NCharacterType CharacterType;
		};
		mu_utf8string AnimationsId;
		mu_uint16 X, Y;
		mu_float Rotation;
	};
}

#endif