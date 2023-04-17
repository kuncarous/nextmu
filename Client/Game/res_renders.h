#ifndef __RES_RENDERS_H__
#define __RES_RENDERS_H__

#pragma once

#include "res_render.h"

namespace MURendersManager
{
	const mu_boolean Initialize();
	void Destroy();

	const NRender *GetRender(const mu_utf8string id);
}

#endif