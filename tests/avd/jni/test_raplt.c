#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <android/log.h>

#include "raplt.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "RaPLT", __VA_ARGS__)

static int g_malloc_hooks = 0;
static int g_strlen_hooks = 0;
static void *(*orig_malloc)(size_t);
static size_t (*orig_strlen)(const char *);

static void *my_malloc(size_t sz)
{
    g_malloc_hooks++;
    return orig_malloc ? orig_malloc(sz) : NULL;
}

static size_t my_strlen(const char *s)
{
    g_strlen_hooks++;
    return orig_strlen ? orig_strlen(s) : 0;
}

int main(void)
{
    int fail = 0;

    LOGI("RaPLT %s starting", raplt_version());

    raplt_hook_t *h1 = raplt_register(".*libc\\.so$", "malloc",
                                        my_malloc, (void **)&orig_malloc, 0);
    raplt_hook_t *h2 = raplt_register(".*libc\\.so$", "strlen",
                                        my_strlen, (void **)&orig_strlen, 0);

    if(!h1) { LOGI("FAIL: hook malloc"); fail++; }
    if(!h2) { LOGI("FAIL: hook strlen"); fail++; }

    char *p = (char *)malloc(64);
    size_t len = strlen("test");
    LOGI("malloc_called=%d strlen_called=%d p=%p len=%zu",
         g_malloc_hooks, g_strlen_hooks, p, len);

    if(g_malloc_hooks == 0) { LOGI("FAIL: malloc not hooked"); fail++; }
    if(g_strlen_hooks == 0) { LOGI("FAIL: strlen not hooked"); fail++; }

    if(h1) raplt_unregister(h1);
    if(h2) raplt_unregister(h2);

    raplt_clear();

    if(fail) { LOGI("FAILED: %d failures", fail); return 1; }
    LOGI("PASS");
    return 0;
}
