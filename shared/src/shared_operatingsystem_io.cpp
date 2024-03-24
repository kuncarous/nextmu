#include "shared_precompiled.h"
#include "shared_operatingsystem_io.h"

mu_utf8string ExecutablePath = "./NextMUBrowser.exe";
mu_utf8string GamePath = "./";
mu_utf8string SupportPath = "./";
mu_utf8string CachePath = "./";
mu_utf8string UserPath = "./";
mu_utf8string GameDataPath = "data/";

mu_boolean ReadFromSupport = false;

void SetReadFromSupport(const mu_boolean enable)
{
	ReadFromSupport = enable;
}

const mu_boolean IsReadFromSupportAvailable()
{
	return ReadFromSupport;
}