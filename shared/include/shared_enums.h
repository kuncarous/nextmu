#ifndef __SHARED_ENUMS_H__
#define __SHARED_ENUMS_H__

#pragma once

namespace NCharacterSex
{
	typedef mu_uint8 Type;
	enum : Type
	{
		Male,
		Female,
	};
};

namespace NCharacterWeaponType
{
	typedef mu_uint8 Type;
	enum : Type
	{
		LeftWeapon,
		RightWeapon,
		DualWeapon,
		TwoHandedWeapon,
	};
};

#endif