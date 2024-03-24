#ifndef __MU_ROOT_H__
#define __MU_ROOT_H__

#pragma once

namespace MURoot
{
	const mu_boolean Initialize(mu_int32 argc, mu_char **argv, void *instance, mu_int32 &exitResult);
	void Destroy();

	void Run();
};

#endif