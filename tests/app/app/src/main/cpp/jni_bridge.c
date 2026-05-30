#include <jni.h>
#include <android/log.h>
#include "raplt.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaPLT", __VA_ARGS__)

/* extern — these create PLT entries in libjni.so that raplt can hook */
extern int calc_add(int, int);
extern int calc_sub(int, int);
extern int calc_mul(int, int);
extern int calc_div(int, int);

/* originals saved by raplt_register */
static int (*orig_add)(int, int);
static int (*orig_sub)(int, int);
static int (*orig_mul)(int, int);
static int (*orig_div)(int, int);

/* hook registrations for unregister */
static raplt_hook_t *hk_add, *hk_sub, *hk_mul, *hk_div;

/* hook 1: add → +1000 */
static int my_add(int a, int b) {
    int r = orig_add ? orig_add(a, b) : a + b;
    LOGI("calc_add(%d,%d) = %d → HOOKED = %d", a, b, r, r + 1000);
    return r + 1000;
}

/* hook 2: sub → swap operands */
static int my_sub(int a, int b) {
    int r = orig_sub ? orig_sub(a, b) : a - b;
    LOGI("calc_sub(%d,%d) = %d → HOOKED = %d", a, b, r, b - a);
    return b - a;
}

/* hook 3: mul → a*b + a */
static int my_mul(int a, int b) {
    int r = orig_mul ? orig_mul(a, b) : a * b;
    LOGI("calc_mul(%d,%d) = %d → HOOKED = %d", a, b, r, r + a);
    return r + a;
}

/* hook 4: div → a * b */
static int my_div(int a, int b) {
    int r = orig_div ? orig_div(a, b) : (b ? a / b : 0);
    LOGI("calc_div(%d,%d) = %d → HOOKED = %d", a, b, r, a * b);
    return a * b;
}

/* regex that matches any library inside the APK */
static const char *R = ".*base\\.apk.*";

JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeAdd(JNIEnv *e, jobject o, jint a, jint b) {
    (void)e; (void)o; return calc_add(a, b);
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeSub(JNIEnv *e, jobject o, jint a, jint b) {
    (void)e; (void)o; return calc_sub(a, b);
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeMul(JNIEnv *e, jobject o, jint a, jint b) {
    (void)e; (void)o; return calc_mul(a, b);
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeDiv(JNIEnv *e, jobject o, jint a, jint b) {
    (void)e; (void)o; return calc_div(a, b);
}

JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookAdd(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_add) hk_add = raplt_register(R, "calc_add", my_add, (void**)&orig_add, 0);
    LOGI("hook_add = %p (orig=%p)", (void*)hk_add, (void*)orig_add);
    return hk_add ? 1 : 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookSub(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_sub) hk_sub = raplt_register(R, "calc_sub", my_sub, (void**)&orig_sub, 0);
    return hk_sub ? 1 : 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookMul(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_mul) hk_mul = raplt_register(R, "calc_mul", my_mul, (void**)&orig_mul, 0);
    return hk_mul ? 1 : 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookDiv(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_div) hk_div = raplt_register(R, "calc_div", my_div, (void**)&orig_div, 0);
    return hk_div ? 1 : 0;
}

JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookAdd(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(hk_add) { raplt_unregister(hk_add); hk_add = NULL; orig_add = NULL; }
    return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookSub(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(hk_sub) { raplt_unregister(hk_sub); hk_sub = NULL; orig_sub = NULL; }
    return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookMul(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(hk_mul) { raplt_unregister(hk_mul); hk_mul = NULL; orig_mul = NULL; }
    return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookDiv(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(hk_div) { raplt_unregister(hk_div); hk_div = NULL; orig_div = NULL; }
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookAll(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(hk_add) { raplt_unregister(hk_add); hk_add = NULL; orig_add = NULL; }
    if(hk_sub) { raplt_unregister(hk_sub); hk_sub = NULL; orig_sub = NULL; }
    if(hk_mul) { raplt_unregister(hk_mul); hk_mul = NULL; orig_mul = NULL; }
    if(hk_div) { raplt_unregister(hk_div); hk_div = NULL; orig_div = NULL; }
    return 0;
}

JNIEXPORT jstring JNICALL
Java_com_raplt_test_MainActivity_nativeVersion(JNIEnv *e, jobject o) {
    (void)o; return (*e)->NewStringUTF(e, raplt_version());
}

JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeInit(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    /* use non-existent symbol to trigger init without modifying any GOT */
    raplt_register(R, "__raplt_init_sentinel__", my_add, NULL, 0);
    LOGI("init done");
    return 0;
}
