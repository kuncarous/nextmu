#ifndef __RES_ITEMS_H__
#define __RES_ITEMS_H__

#pragma once

#include "res_item.h"

namespace MUItemsManager
{
	const mu_boolean Initialize();
	void Destroy();

	NItem *GetItem(const mu_uint16 category, const mu_uint16 index);
}

#endif