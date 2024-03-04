#include "shared_precompiled.h"
#include "shared_operatingsystem_io.h"

mu_utf8string SupportPathUTF8 = "./";
mu_utf8string CachePathUTF8 = "./";
mu_utf8string UserPathUTF8 = "./";
mu_utf8string GameDataPathUTF8 = "data/";

mu_boolean ReadFromSupport = false;

void SetReadFromSupport(const mu_boolean enable)
{
	ReadFromSupport = enable;
}

const mu_boolean IsReadFromSupportAvailable()
{
	return ReadFromSupport;
}