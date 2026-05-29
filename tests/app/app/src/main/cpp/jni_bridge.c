#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include "raplt.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaPLT", __VA_ARGS__)

static int g_mh = 0;
static int g_sh = 0;
static void *(*om)(size_t);
static size_t (*os)(const char *);

static void *my_malloc(size_t s) { g_mh++; return om ? om(s) : NULL; }
static size_t my_strlen(const char *s) { g_sh++; return os ? os(s) : 0; }

static void test(void)
{
    LOGI("raplt %s", raplt_version());

    void *h = dlopen("libtarget.so", RTLD_NOW);
    if(!h) { LOGI("FAIL dlopen target.so"); return; }

    raplt_hook_t *a = raplt_register(".*libtarget\\.so$", "malloc", my_malloc, (void**)&om, 0);
    raplt_hook_t *b = raplt_register(".*libtarget\\.so$", "strlen", my_strlen, (void**)&os, 0);
    LOGI("hook malloc=%p strlen=%p", (void*)a, (void*)b);

    void (*t)(void) = dlsym(h, "trigger");
    if(t) t();
    LOGI("count malloc=%d strlen=%d", g_mh, g_sh);

    if(a) { raplt_unregister(a); a = NULL; LOGI("unhook malloc ok"); }
    if(b) { raplt_unregister(b); b = NULL; LOGI("unhook strlen ok"); }

    raplt_clear();
    dlclose(h);
}

JNIEXPORT jstring JNICALL
Java_com_raplt_test_MainActivity_nativeRun(JNIEnv *e, jobject o)
{
    (void)o;
    test();
    return (*e)->NewStringUTF(e, g_mh > 0 && g_sh > 0 ? "PASS" : "FAIL");
}
