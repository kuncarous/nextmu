// mu_main.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "mu_root.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	mu_int32 result = 0;
	if (MURoot::Initialize(__argc, __argv, hInstance, result) == true)
	{
		MURoot::Run();
	}

	MURoot::Destroy();

	return result;
}