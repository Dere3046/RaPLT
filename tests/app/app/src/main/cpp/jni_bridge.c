#include <jni.h>
#include <dlfcn.h>
#include <android/log.h>
#include "raplt.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaPLT", __VA_ARGS__)

static int (*orig_get_number)(void);
static int (*orig_add)(int, int);

static int my_get_number(void)
{
    LOGI("hook: target_get_number returns 999 (was %d)", orig_get_number ? orig_get_number() : 0);
    return 999;
}

static int my_add(int a, int b)
{
    int hooked = a * b;
    LOGI("hook: target_add(%d,%d) = %d (was %d)", a, b, hooked, orig_add ? orig_add(a, b) : 0);
    return hooked;
}

JNIEXPORT jstring JNICALL
Java_com_raplt_test_MainActivity_nativeRun(JNIEnv *e, jobject o)
{
    (void)o;
    char buf[1024];
    int off = 0;

    off += snprintf(buf + off, sizeof(buf) - off, "RaPLT %s\n", raplt_version());

    void *h = dlopen("libtarget.so", RTLD_NOW);
    if(!h) {
        off += snprintf(buf + off, sizeof(buf) - off, "FAIL: dlopen target.so\n");
        return (*e)->NewStringUTF(e, buf);
    }

    /* resolve and test original functions */
    int (*get_number)(void) = dlsym(h, "target_get_number");
    int (*add)(int, int) = dlsym(h, "target_add");
    if(!get_number || !add) {
        off += snprintf(buf + off, sizeof(buf) - off, "FAIL: dlsym\n");
        dlclose(h);
        return (*e)->NewStringUTF(e, buf);
    }

    int orig_n = get_number();
    int orig_s = add(5, 3);
    off += snprintf(buf + off, sizeof(buf) - off,
                    "before hook: get_number=%d add(5,3)=%d\n", orig_n, orig_s);

    /* hook — regex matches the APK path on Android 16 */
    raplt_hook_t *hk = raplt_register(".*base\\.apk.*",
                                       "target_get_number",
                                       my_get_number,
                                       (void**)&orig_get_number,
                                       0);
    if(!hk) {
        off += snprintf(buf + off, sizeof(buf) - off, "FAIL: register get_number\n");
        dlclose(h);
        return (*e)->NewStringUTF(e, buf);
    }

    raplt_hook_t *ha = raplt_register(".*base\\.apk.*",
                                       "target_add",
                                       my_add,
                                       (void**)&orig_add,
                                       0);
    off += snprintf(buf + off, sizeof(buf) - off, "hooks: get_number=%p add=%p\n", (void*)hk, (void*)ha);

    /* call hooked versions */
    int hooked_n = get_number();
    int hooked_s = add(5, 3);
    off += snprintf(buf + off, sizeof(buf) - off,
                    "after hook: get_number=%d add(5,3)=%d\n", hooked_n, hooked_s);

    /* unregister */
    if(hk) raplt_unregister(hk);
    if(ha) raplt_unregister(ha);

    /* verify restored */
    int restored_n = get_number();
    int restored_s = add(5, 3);
    off += snprintf(buf + off, sizeof(buf) - off,
                    "restored: get_number=%d add(5,3)=%d\n", restored_n, restored_s);

    raplt_clear();
    dlclose(h);

    int pass = (hooked_n == 999 && hooked_s == 15);
    off += snprintf(buf + off, sizeof(buf) - off, "\n%s", pass ? "PASS" : "FAIL");
    return (*e)->NewStringUTF(e, buf);
}
