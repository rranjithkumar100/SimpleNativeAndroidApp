#include <jni.h>
#include <string.h>

JNIEXPORT jstring JNICALL
Java_com_twt_simplenativeandroidapp_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    char *hello = "Hello from JNI";
    return (*env)->NewStringUTF(env, hello);
}
