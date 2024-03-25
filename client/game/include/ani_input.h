#ifndef __ANI_INPUT_H__
#define __ANI_INPUT_H__

#pragma once

struct NAnimationInput
{
	mu_utf8string Action;
	mu_boolean SafeZone = false;
	mu_boolean Swimming = false;
	mu_boolean HasWings = false;
	NCharacterSex::Type Sex = NCharacterSex::Male;
	NCharacterTypeInfo CharacterType;
	mu_uint16 Mount = NInvalidUInt16;
	mu_uint16 Pet = NInvalidUInt16;
	mu_uint16 Wings = NInvalidUInt16;
	mu_uint16 WeaponLeft = NInvalidUInt16;
	mu_uint16 WeaponRight = NInvalidUInt16;
	NCharacterWeaponType::Type Weapons = NCharacterWeaponType::LeftWeapon;
};

#endif