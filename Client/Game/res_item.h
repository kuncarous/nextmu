#ifndef __RES_ITEM_H__
#define __RES_ITEM_H__

#pragma once

struct NRender;

enum class NItemCategory : mu_uint16
{
	Axes = 1,
	Helm = 7,
	Armor = 8,
	Pants = 9,
	Gloves = 10,
	Boots = 11,
};

struct NItem
{
	mu_uint16 Category;
	mu_uint16 Index;
	mu_utf8string Name;
	const NRender *Render;
};

#endif