// mu_main.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <include/cef_app.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

    CefMainArgs args(hInstance);
    int result = CefExecuteProcess(args, nullptr, nullptr);
    return result;
}