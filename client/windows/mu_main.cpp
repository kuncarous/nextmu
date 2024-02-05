// mu_main.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "mu_root.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	if (MURoot::Initialize() == true)
	{
		MURoot::Run();
	}

	MURoot::Destroy();

	return 0;
}