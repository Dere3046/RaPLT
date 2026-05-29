#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>

#include "raplt.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaPLT-jni", __VA_ARGS__)

static void *g_orig_malloc = NULL;
static void *g_orig_strlen = NULL;
static int   g_malloc_cnt = 0;
static int   g_strlen_cnt = 0;

static void *my_malloc(size_t sz)
{
    g_malloc_cnt++;
    return ((void *(*)(size_t))g_orig_malloc)(sz);
}

static size_t my_strlen(const char *s)
{
    g_strlen_cnt++;
    return ((size_t (*)(const char *))g_orig_strlen)(s);
}

JNIEXPORT jstring JNICALL
Java_com_raplt_test_MainActivity_nativeTest(JNIEnv *env, jobject thiz)
{
    (void)thiz;
    char buf[1024];
    int off = 0;

    off += snprintf(buf + off, sizeof(buf) - off, "RaPLT %s\n", raplt_version());

    /* hook malloc in the test target lib */
    void *handle = dlopen("libtest_target.so", RTLD_NOW);
    if(!handle) {
        off += snprintf(buf + off, sizeof(buf) - off, "FAIL: dlopen test_target\n");
        return (*env)->NewStringUTF(env, buf);
    }

    raplt_hook_t *h1 = raplt_register(".*libtest_target\\.so$",
                                       "malloc",
                                       my_malloc,
                                       &g_orig_malloc,
                                       0);
    raplt_hook_t *h2 = raplt_register(".*libtest_target\\.so$",
                                       "strlen",
                                       my_strlen,
                                       &g_orig_strlen,
                                       RAPLT_FLAG_BATCH);

    off += snprintf(buf + off, sizeof(buf) - off,
                    "h1=%p h2=%p orig_malloc=%p orig_strlen=%p\n",
                    (void*)h1, (void*)h2, g_orig_malloc, g_orig_strlen);

    raplt_commit();

    /* trigger hooks by calling functions in the target lib */
    void (*trigger)(void) = dlsym(handle, "test_target_trigger_hooks");
    if(trigger) trigger();

    off += snprintf(buf + off, sizeof(buf) - off,
                    "malloc_hook_cnt=%d strlen_hook_cnt=%d\n",
                    g_malloc_cnt, g_strlen_cnt);

    /* unregister and verify recovery */
    if(h1) {
        raplt_unregister(h1);
        g_malloc_cnt = 0;
        if(trigger) trigger();
        off += snprintf(buf + off, sizeof(buf) - off,
                        "after unregister: malloc_cnt=%d\n", g_malloc_cnt);
    }
    if(h2) {
        raplt_unregister(h2);
        g_strlen_cnt = 0;
        if(trigger) trigger();
        off += snprintf(buf + off, sizeof(buf) - off,
                        "after unregister: strlen_cnt=%d\n", g_strlen_cnt);
    }

    raplt_hooks_finalize();

    dlclose(handle);
    return (*env)->NewStringUTF(env, buf);
}
