#include "stdafx.h"
#include "mu_root.h"

#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <android/log.h>
#include <stdio.h>
#include <iostream>

JNIEXPORT int main(int argc, char **argv)
{
	mu_int32 result = 0;
	if (MURoot::Initialize(argc, argv, nullptr, result) == true)
	{
		MURoot::Run();
	}
	MURoot::Destroy();

	return result;
}