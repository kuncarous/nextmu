#include "mu_precompiled.h"
#include "mu_root.h"

void Main(int argc, char **argv)
{
	mu_int32 result = 0;
	if (MURoot::Initialize(argc, argv, nullptr, result) == true)
	{
		MURoot::Run();
	}
	MURoot::Destroy();

	return result;
}