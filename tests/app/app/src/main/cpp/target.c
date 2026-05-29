#include <stdlib.h>
#include <string.h>
#include <android/log.h>

static int g_malloc_calls = 0;
static int g_strlen_calls = 0;

__attribute__((visibility("default")))
void trigger(void)
{
    void *p = malloc(64);
    free(p);
    size_t n = strlen("raplt");
    (void)n;
}
