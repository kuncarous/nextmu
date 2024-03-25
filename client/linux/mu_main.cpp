// mu_main.cpp : Defines the entry point for the application.
//

#include "mu_precompiled.h"
#include "mu_root.h"

int main(int argc, char **argv)
{
	mu_int32 result = 0;
	if (MURoot::Initialize(argc, argv, nullptr, result) == true)
	{
		MURoot::Run();
	}

	MURoot::Destroy();

	return result;
}