#include "jni/include/jni.h"


JNIEXPORT void JNICALL Java_getVersion(JNIEnv *env, jobject obj)
{
	jint version = env->GetVersion();
}





