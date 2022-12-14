#include "stdafx.h"
#include "shared_operatingsystem_io.h"

mu_utf8string SupportPathUTF8 = "./";
mu_unicodestring SupportPathUnicode = ConvertToUnicodeString(SupportPathUTF8);
mu_utf8string CachePathUTF8 = "./";
mu_unicodestring CachePathUnicode = ConvertToUnicodeString(CachePathUTF8);
mu_utf8string UserPathUTF8 = "./";
mu_unicodestring UserPathUnicode = ConvertToUnicodeString(UserPathUTF8);

mu_boolean ReadFromSupport = false;

void SetReadFromSupport(const mu_boolean enable)
{
	ReadFromSupport = enable;
}

const mu_boolean IsReadFromSupportAvailable()
{
	return ReadFromSupport;
}