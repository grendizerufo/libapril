/// @file
/// @version 3.5
/// 
/// @section LICENSE
/// 
/// This program is free software; you can redistribute it and/or modify it under
/// the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause

#ifdef _ANDROID
#include <jni.h>

#include <gtypes/Vector2.h>
#include <hltypes/hmap.h>
#include <hltypes/hstring.h>

#define __NATIVE_INTERFACE_CLASS "com/april/NativeInterface"
#include "androidUtilJNI.h"
#include "Platform.h"
#include "RenderSystem.h"

namespace april
{
	void* javaVM = NULL;
	void (*dialogCallback)(MessageBoxButton) = NULL; // defined here to avoid making a bunch of _OPENKODE #ifdefs in Android_Platform.cpp
	jobject classLoader = NULL;

	hstr _jstringToHstr(JNIEnv* env, jstring string)
	{
		const char* chars = env->GetStringUTFChars(string, NULL);
		hstr result(chars);
		env->ReleaseStringUTFChars(string, chars);
		return result;
	}
	
	JNIEnv* getJNIEnv()
	{
		JNIEnv* env = NULL;
		return (((JavaVM*)april::javaVM)->AttachCurrentThread(&env, NULL) == JNI_OK ? env : NULL);
	}
	
	jobject getActivity()
	{
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jfieldID fieldActivity = env->GetStaticFieldID(classNativeInterface, "Activity", _JCLASS("android/app/Activity"));
		return env->GetStaticObjectField(classNativeInterface, fieldActivity);
	}
	
	jobject getAprilActivity()
	{
		APRIL_GET_NATIVE_INTERFACE_CLASS(classNativeInterface);
		jfieldID fieldAprilActivity = env->GetStaticFieldID(classNativeInterface, "AprilActivity", _JCLASS("com/april/Activity"));
		return env->GetStaticObjectField(classNativeInterface, fieldAprilActivity);
	}

	jclass findJNIClass(JNIEnv* env, chstr classPath)
	{
		if (april::classLoader == NULL)
		{
			return env->FindClass(classPath.cStr());
		}
		jclass classClassLoader = env->GetObjectClass(april::classLoader);
		jmethodID methodLoadClass = env->GetMethodID(classClassLoader, "loadClass", _JARGS(_JCLASS("java/lang/Class"), _JSTR _JBOOL));
		jstring jClassPath = env->NewStringUTF(classPath.cStr());
		jboolean jInitialize = JNI_TRUE;
		return (jclass)env->CallObjectMethod(april::classLoader, methodLoadClass, jClassPath, jInitialize);
	}

}
#endif
