#include "shared_precompiled.h"

#if NEXTMU_CLIENT_SHARED == 1 && NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
#include <jni.h>
#include <filesystem>

namespace NXOperatingSystem
{
	void Initialize()
	{
		HasScreenKeyboardSupport = SDL_HasScreenKeyboardSupport();
	}

	const mu_boolean GetDeviceScreenSize(mu_int32 &width, mu_int32 &height)
	{
		JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
		jni_unique activity = (jobject)SDL_AndroidGetActivity();
		if (!activity)
		{
			return false;
		}

		jni_unique activity_class = env->GetObjectClass(activity.get());
		if (!activity_class)
		{
			return false;
		}

		jmethodID getWindowManager = env->GetMethodID(activity_class.get(), "getWindowManager", "()Landroid/view/WindowManager;");
		if (getWindowManager == nullptr)
		{
			return false;
		}

		jni_unique wm = env->CallObjectMethod(activity.get(), getWindowManager);
		if (!wm)
		{
			return false;
		}

		jni_unique windowManagerClass = env->FindClass("android/view/WindowManager");
		if (!windowManagerClass)
		{
			return false;
		}

		jmethodID getDefaultDisplay = env->GetMethodID(windowManagerClass.get(), "getDefaultDisplay", "()Landroid/view/Display;");
		if (getDefaultDisplay == nullptr)
		{
			return false;
		}

		jni_unique display = env->CallObjectMethod(wm.get(), getDefaultDisplay);
		if (!display)
		{
			return false;
		}

		jni_unique displayClass = env->FindClass("android/view/Display");
		if (!displayClass)
		{
			return false;
		}

		jni_unique pointClass = env->FindClass("android/graphics/Point");
		if (!pointClass)
		{
			return false;
		}

		jmethodID pointConstructor = env->GetMethodID(pointClass.get(), "<init>", "()V");
		if (pointConstructor == nullptr)
		{
			return false;
		}

		jni_unique pointDisplaySize = env->NewObject(pointClass.get(), pointConstructor);
		if (!pointDisplaySize)
		{
			return false;
		}

		jmethodID getRealSize = env->GetMethodID(displayClass.get(), "getRealSize", "(Landroid/graphics/Point;)V");
		if (getRealSize == nullptr)
		{
			return false;
		}

		env->CallVoidMethod(display.get(), getRealSize, pointDisplaySize.get());

		jfieldID width_field = env->GetFieldID(pointClass.get(), "x", "I");
		jfieldID height_field = env->GetFieldID(pointClass.get(), "y", "I");
		width = env->GetIntField(pointDisplaySize.get(), width_field);
		height = env->GetIntField(pointDisplaySize.get(), height_field);

		return true;
	}
	
	void EnumerateFilesAPK(const mu_utf8string path,
						   std::vector<mu_utf8string> &filesList)
	{
		JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
		jni_unique activity = (jobject)SDL_AndroidGetActivity();
		if (!activity)
		{
			return;
		}

		jni_unique activityClass = env->GetObjectClass(activity.get());
		if (!activityClass)
		{
			return;
		}

		jni_unique assetManagerClass = env->FindClass("android/content/res/AssetManager");
		if (!assetManagerClass)
		{
			return;
		}

		jmethodID getAssets = env->GetMethodID(activityClass.get(), "getAssets", "()Landroid/content/res/AssetManager;");
		if (getAssets == nullptr)
		{
			return;
		}

		jni_unique assetManager = env->CallObjectMethod(activity.get(), getAssets);
		if (!assetManager)
		{
			return;
		}

		jmethodID list = env->GetMethodID(assetManagerClass.get(), "list", "(Ljava/lang/String;)[Ljava/lang/String;");
		if (list == nullptr)
		{
			return;
		}

		jni_unique pathString = env->NewStringUTF(path[path.size() - 1] == '/' ? path.substr(0, path.size() - 1).c_str() : path.c_str());
		if (!pathString)
		{
			return;
		}

		jni_unique files = (jobjectArray)env->CallObjectMethod(assetManager.get(), list, pathString.get());
		if (!files)
		{
			return;
		}

		mu_uint32 filesCount = static_cast<mu_uint32>(env->GetArrayLength(files.get()));
		for (mu_uint32 n = 0; n < filesCount; ++n)
		{
			jni_unique file = (jstring)env->GetObjectArrayElement(files.get(), n);
			if (file)
			{
				const mu_char *fileName = env->GetStringUTFChars(file.get(), nullptr);
				filesList.push_back(fileName);
			}
		}
	}

	const mu_boolean EnumerateFilesFromApplication(const mu_utf8string path, std::vector<mu_utf8string> &filesList)
	{
		filesList.clear();

		EnumerateFilesAPK(path, filesList);

		return true;
	}

	const mu_boolean EnumerateFiles(const mu_utf8string path, std::vector<mu_utf8string> &filesList)
	{
		std::filesystem::path pathHandler(path);
		filesList.clear();

		if (path[0] != '/')
		{
			EnumerateFilesAPK(path, filesList);
		}

		std::error_code ec;
		if (std::filesystem::exists(pathHandler, ec) == false)
		{
			return false;
		}

		if (std::filesystem::is_directory(path, ec) == false)
		{
			return false;
		}

		const mu_boolean removeDuplicates = filesList.size() > 0;
		std::filesystem::directory_iterator endIter;
		for (std::filesystem::directory_iterator iter(path, ec);
			 iter != endIter;
			 ++iter)
		{
			if (std::filesystem::is_regular_file(iter->status()) == true)
			{
				filesList.push_back(iter->path().filename());
			}
		}

		if (removeDuplicates == true)
		{
			std::sort(filesList.begin(), filesList.end());
			filesList.erase(std::unique(filesList.begin(), filesList.end()), filesList.end());
		}

		return true;
	}

	const mu_utf8string GetStorageSupportFilesPath()
	{
		return mu_utf8string(SDL_AndroidGetExternalStoragePath()) + "/";
	}

	const mu_utf8string GetStorageCacheFilesPath()
	{
		return mu_utf8string(SDL_AndroidGetExternalStoragePath()) + "/";
	}

	const mu_utf8string GetStorageUserFilesPath()
	{
		return mu_utf8string(SDL_AndroidGetExternalStoragePath()) + "/";
	}
};
#endif