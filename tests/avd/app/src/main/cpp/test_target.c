#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

static int g_malloc_hook_called = 0;
static int g_strlen_hook_called = 0;

static void *(*orig_malloc)(size_t);
static size_t (*orig_strlen)(const char *);

static void *my_malloc(size_t sz)
{
    g_malloc_hook_called++;
    if(orig_malloc) return orig_malloc(sz);
    return NULL;
}

static size_t my_strlen(const char *s)
{
    g_strlen_hook_called++;
    if(orig_strlen) return orig_strlen(s);
    return 0;
}

void test_target_trigger_hooks(void)
{
    void *p = malloc(128);
    __android_log_print(ANDROID_LOG_INFO, "RaPLT-target", "malloc called, hook_called=%d", g_malloc_hook_called);

    size_t len = strlen("RaPLT hook test");
    __android_log_print(ANDROID_LOG_INFO, "RaPLT-target", "strlen=%zu hook_called=%d", len, g_strlen_hook_called);

    free(p);
}

__attribute__((visibility("default")))
void test_target_do_nothing(void)
{
    /* noop, just an exported function */
}
