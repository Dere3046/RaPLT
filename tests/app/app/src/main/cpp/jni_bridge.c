#include <jni.h>
#include <signal.h>
#include <setjmp.h>
#include <android/log.h>
#include "raplt.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaPLT", __VA_ARGS__)

extern int calc_add(int, int);
extern int calc_sub(int, int);
extern int calc_mul(int, int);
extern int calc_div(int, int);

static int (*orig_add)(int, int);
static int (*orig_sub)(int, int);
static int (*orig_mul)(int, int);
static int (*orig_div)(int, int);
static raplt_hook_t *hk_add, *hk_sub, *hk_mul, *hk_div;
static const char *R = ".*base\\.apk.*";

/* independent SIGSEGV guard — catches crashes from raplt_register without
 * conflicting with raplt's own signal handling. */
static sigjmp_buf jg;
static struct sigaction jg_old;

static void jg_handler(int sig) {
    (void)sig;
    siglongjmp(jg, 1);
}

static int jg_enter(void) {
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = jg_handler;
    sigaction(SIGSEGV, &act, &jg_old);
    return sigsetjmp(jg, 1);
}

static void jg_exit(void) {
    sigaction(SIGSEGV, &jg_old, NULL);
}

static int safe_register(const char *sym, void *hook, void **orig, raplt_hook_t **out) {
    if(jg_enter()) { LOGI("SIGSEGV while registering %s", sym); *out = NULL; return 0; }
    *out = raplt_register(R, sym, hook, (void**)orig, 0);
    jg_exit();
    return *out ? 1 : 0;
}
static int safe_unregister(raplt_hook_t **hp) {
    if(!hp || !*hp) return 0;
    if(jg_enter()) { LOGI("SIGSEGV while unregistering"); *hp = NULL; return 0; }
    raplt_unregister(*hp);
    jg_exit();
    *hp = NULL;
    return 1;
}

/* hook proxies */
static int my_add(int a, int b) {
    LOGI("calc_add(%d,%d) → HOOKED = %d", a, b, a + b + 1000);
    return a + b + 1000;
}
static int my_sub(int a, int b) {
    LOGI("calc_sub(%d,%d) → HOOKED = %d", a, b, b - a);
    return b - a;
}
static int my_mul(int a, int b) {
    LOGI("calc_mul(%d,%d) → HOOKED = %d", a, b, a * b + a);
    return a * b + a;
}
static int my_div(int a, int b) {
    LOGI("calc_div(%d,%d) → HOOKED = %d", a, b, a * b);
    return a * b;
}

/* JNI */
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
Java_com_raplt_test_MainActivity_nativeInit(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    raplt_hook_t *dummy;
    safe_register("__raplt_init_sentinel__", my_add, NULL, &dummy);
    LOGI("init done");
    return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookAdd(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_add) safe_register("calc_add", my_add, (void**)&orig_add, &hk_add);
    return hk_add ? 1 : 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookSub(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_sub) safe_register("calc_sub", my_sub, (void**)&orig_sub, &hk_sub);
    return hk_sub ? 1 : 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookMul(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_mul) safe_register("calc_mul", my_mul, (void**)&orig_mul, &hk_mul);
    return hk_mul ? 1 : 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeHookDiv(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    if(!hk_div) safe_register("calc_div", my_div, (void**)&orig_div, &hk_div);
    return hk_div ? 1 : 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookAdd(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    safe_unregister(&hk_add); orig_add = NULL; return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookSub(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    safe_unregister(&hk_sub); orig_sub = NULL; return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookMul(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    safe_unregister(&hk_mul); orig_mul = NULL; return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookDiv(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    safe_unregister(&hk_div); orig_div = NULL; return 0;
}
JNIEXPORT jint JNICALL
Java_com_raplt_test_MainActivity_nativeUnhookAll(JNIEnv *e, jobject o) {
    (void)e; (void)o;
    safe_unregister(&hk_add); orig_add = NULL;
    safe_unregister(&hk_sub); orig_sub = NULL;
    safe_unregister(&hk_mul); orig_mul = NULL;
    safe_unregister(&hk_div); orig_div = NULL;
    return 0;
}
JNIEXPORT jstring JNICALL
Java_com_raplt_test_MainActivity_nativeVersion(JNIEnv *e, jobject o) {
    (void)o; return (*e)->NewStringUTF(e, raplt_version());
}
