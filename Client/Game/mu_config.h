#ifndef __MU_CONFIG_H__
#define __MU_CONFIG_H__

#pragma once

namespace MUConfig
{
	const mu_boolean Initialize();
	void Destroy();

	void Save();

	const mu_boolean GetWindowMode();
	const mu_uint32 GetWindowWidth();
	const mu_uint32 GetWindowHeight();

	const mu_boolean GetEnableShadows();

	const mu_boolean GetAntialiasing();
	const mu_boolean GetVerticalSync();
};

#endif