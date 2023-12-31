#ifndef __RES_ITEM_H__
#define __RES_ITEM_H__

#pragma once

struct NRender;

enum class EItemCategory : mu_uint16
{
	Axes = 1,
	Maces = 2,
	Staffs = 5,
	Helm = 7,
	Armor = 8,
	Pants = 9,
	Gloves = 10,
	Boots = 11,
	Wings = 12,
};

enum class EItemOption : mu_uint16
{
	ePhysicalLifeSteal = 0u,
	eMagicalLifeSteal = 1u,
	eRestoreManaPerHit = 2u,
	eIncreasePhysicalDamage = 3u,
	eIncreaseMagicalDamage = 4u,
	eIncreasePhysicalDefense = 5u,
	eIncreaseMagicalDefense = 6u,
	eIncreaseLife = 7u,
	eIncreaseMana = 8u,
	eIncreaseStamina = 9u,
	eIncreaseLifeRegen = 10u,
	eIncreaseManaRegen = 11u,
	eIncreaseBlockChance = 12u,
	eIncreaseBlockDamage = 13u,
	eIncreaseMoveSpeed = 14u,
	eIncreaseAttackSpeed = 15u,
	eIncreaseAttackRating = 16u,
	eIncreaseEvasionRating = 17u,
	eIncreaseCriticalChance = 18u,
	eIncreaseCriticalDamage = 19u,

	// Only Energy Elf, Caster increase, not Target increase.
	eIncreaseBuffDuration = 20u,
	eIncreaseHealBuff = 21u,
	eIncreaseAttackBuff = 22u,
	eIncreaseDefenseBuff = 23u,

	// Rings & Pendant only (Farming Items)
	eIncreaseItemDropRate = 24u,
	eIncreaseItemRankRate = 25u,

	eIncreaseStaminaRegen = 26u,
	eMax,
};

constexpr mu_uint32 MaxItemSpecialOptions = 8u;
constexpr mu_float MaxCriticalChance = 0.45f;

enum class EItemRank : mu_uint8
{
	eRankF,
	eRankE,
	eRankD,
	eRankC,
	eRankB,
	eRankA,
	eRankS,
	eRankSS,
	eRankSSS,
	eRankEX,
	eRankMax,
};

constexpr mu_uint32 MaxItemRank = static_cast<mu_uint32>(EItemRank::eRankMax);

struct NItem
{
	mu_uint16 Category;
	mu_uint16 Index;
	mu_utf8string Name;
	NRender *Render;
};

struct NItemOption
{
	EItemOption Type;
	mu_uint16 Rank;
};

typedef std::vector<NItemOption> NItemOptions;

NEXTMU_INLINE const mu_float ItemRankToFloat(const mu_uint16 rank)
{
	constexpr float_t cvalue = static_cast<float_t>(1.0 / static_cast<mu_double>((std::numeric_limits<mu_uint16>::max)()));
	return static_cast<mu_float>(rank) * cvalue;
}

NEXTMU_INLINE const mu_uint16 ItemRankToUInt16(const mu_float rank)
{
	constexpr float_t cvalue = static_cast<float_t>((std::numeric_limits<mu_uint16>::max)());
	return static_cast<mu_uint16>(glm::round(rank * cvalue));
}

constexpr mu_uint32 MaxItemLevel = 200u;
constexpr mu_uint32 MaxBaseItemLevel = 100u;
constexpr mu_float ItemLevelDivisor = 1.0f / 100.0f;
constexpr mu_float ItemLevelAddition = 25.0f / 100.0f;
constexpr mu_float BaseItemLevelFormula = 0.5f;

NEXTMU_INLINE const mu_float ItemLevelFormula(const mu_uint16 level)
{
	return 1.0f + static_cast<mu_float>(level) * ItemLevelDivisor * ItemLevelAddition;
}

NEXTMU_INLINE const mu_float ItemLevelSpecialFormula(const mu_uint16 level)
{
	constexpr mu_float MaxBaseItemLevelMultiplier = static_cast<mu_float>(1.0 / static_cast<mu_double>(MaxBaseItemLevel));
	constexpr mu_float BaseItemLevelSpecialFormula = 0.3f;

	const mu_float l = static_cast<mu_float>(level) * MaxBaseItemLevelMultiplier;
	const mu_float extra = glm::max(0.0f, l - 1.0f);
	return (BaseItemLevelSpecialFormula + (1.0f - BaseItemLevelSpecialFormula) * glm::min(l, 1.0f)) + extra;
}

constexpr mu_uint32 MaxGlowLevel = 15u;
constexpr mu_uint32 LevelDiv = 6u;

NEXTMU_INLINE const mu_uint32 ItemGlowToLevelFormula(const mu_uint32 level)
{
	return glm::min(level * LevelDiv, 100u);
}

NEXTMU_INLINE const mu_uint32 ItemGlowFormula(const mu_uint32 level)
{
	return glm::min(level / LevelDiv, MaxGlowLevel);
}

NEXTMU_INLINE const mu_float ItemRankToPercent(const mu_float rank)
{
	return 1.0f + ((rank - 0.5f) / 0.5f) * 0.25f;
}

NEXTMU_INLINE const mu_float ItemRankFormula(const mu_float value, const mu_float rank)
{
	return value * ItemRankToPercent(rank);
}

NEXTMU_INLINE const mu_float ItemRankFormulaU32(const mu_uint32 value, const mu_float rank)
{
	return static_cast<mu_float>(value) * ItemRankToPercent(rank);
}

NEXTMU_INLINE const mu_float ItemRankFormulaBase(const mu_float value, const mu_float base, const mu_float rank)
{
	return value * (base + (1.0f - base) * rank);
}

NEXTMU_INLINE const mu_float ItemRankFormulaBaseU32(const mu_uint32 value, const mu_float base, const mu_float rank)
{
	return static_cast<mu_float>(value) * (base + (1.0f - base) * rank);
}

NEXTMU_INLINE const EItemRank ItemRankToSimpleRank(const mu_float itemRank)
{
	if (itemRank <= 0.15f) return EItemRank::eRankF;
	if (itemRank <= 0.30f) return EItemRank::eRankE;
	if (itemRank <= 0.40f) return EItemRank::eRankD;
	if (itemRank <= 0.50f) return EItemRank::eRankC;
	if (itemRank <= 0.60f) return EItemRank::eRankB;
	if (itemRank <= 0.70f) return EItemRank::eRankA;
	if (itemRank <= 0.80f) return EItemRank::eRankS;
	if (itemRank <= 0.90f) return EItemRank::eRankSS;
	if (itemRank < 1.0f) return EItemRank::eRankSSS;
	return EItemRank::eRankEX;
}

NEXTMU_INLINE const mu_char *ItemSimpleRankToString(const EItemRank rank)
{
	switch (rank)
	{
	case EItemRank::eRankF: return "F";
	case EItemRank::eRankE: return "E";
	case EItemRank::eRankD: return "D";
	case EItemRank::eRankC: return "C";
	case EItemRank::eRankB: return "B";
	case EItemRank::eRankA: return "A";
	case EItemRank::eRankS: return "S";
	case EItemRank::eRankSS: return "SS";
	case EItemRank::eRankSSS: return "SSS";
	case EItemRank::eRankEX: return "EX";
	}
	return "F";
}

NEXTMU_INLINE const EItemRank ItemRankU16ToSimpleRank(const mu_uint16 rank)
{
	const mu_float itemRank = ItemRankToFloat(rank) * 100.0f;
	return ItemRankToSimpleRank(itemRank);
}

#endif