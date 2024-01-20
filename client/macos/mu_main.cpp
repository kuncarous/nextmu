#include "mu_precompiled.h"
#include "mu_root.h"

void Main()
{
	if (MURoot::Initialize() == true)
	{
		MURoot::Run();
	}

	MURoot::Destroy();
	return 0;
}