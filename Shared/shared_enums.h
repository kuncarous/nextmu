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

namespace NCharacterClass
{
	typedef mu_uint8 Type;
	enum : Type
	{
		DarkWizard,
		DarkKnight,
		Elf,
		MagicGladiator,
		DarkLord,
		Summoner,

		MaxClasses,

		// 1st SubClass
		BeginSubclass1st = MaxClasses,
		SoulMaster = BeginSubclass1st,
		BladeKnight,
		MuseElf,
		BloodySummoner,

		// 2nd SubClass
		BeginSubclass2nd,
		GrandMaster = BeginSubclass2nd,
		BladeMaster,
		HighElf,
		DuelMaster,
		LordEmperor,
		DimensionMaster,
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