#ifndef __SHARED_OPERATINGSYSTEM_ANDROID_H__
#define __SHARED_OPERATINGSYSTEM_ANDROID_H__

#pragma once

//__ANDROID_API__=$(AndroidAPILevelNumber)

#if NEXTMU_OPERATING_SYSTEM == NEXTMU_OS_ANDROID
#include <jni.h>

template<typename T>
class jni_unique
{
public:
	jni_unique(T object) : _object(object)
	{
	}

	~jni_unique()
	{
		if (_object != nullptr)
		{
			JNIEnv *env = (JNIEnv *)SDL_AndroidGetJNIEnv();
			env->DeleteLocalRef(_object);
			_object = nullptr;
		}
	}

	T get() const
	{
		return _object;
	}

	T operator*()
	{
		return get();
	}

	explicit operator bool() const noexcept
	{
		return (get() != nullptr);
	}

private:
	T _object;
};
#endif

#endif