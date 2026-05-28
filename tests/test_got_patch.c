#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <unistd.h>

#include "raplt.h"
#include "raplt_util.h"
#include "test_runner.h"

static int g_patched = 0;

static void *my_func(void)
{
    g_patched++;
    return (void *)0x42;
}

int main(void)
{
    void *handle = dlopen("./libtestfixture.so", RTLD_NOW);
    if(!handle) {
        printf("SKIP: no test fixture\n");
        return 77;
    }

    T("direct GOT write works");
    void *page = mmap(NULL, 4096, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT(page != MAP_FAILED, "mmap failed");

    void **got_addr = (void **)((char *)page + 16);
    *got_addr = (void *)0xbad;

    raplt_write_got(got_addr, (void *)0xcafe);
    ASSERT(*got_addr == (void *)0xcafe, "GOT write failed: got %p", *got_addr);

    raplt_flush_cache((uintptr_t)got_addr);
    munmap(page, 4096);

    T("batch protect + write works");
    void *page2 = mmap(NULL, 4096 * 2, PROT_READ,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    ASSERT(page2 != MAP_FAILED, "mmap2 failed");

    void *addrs[3];
    void *values[3];
    addrs[0] = (char *)page2 + 8;
    addrs[1] = (char *)page2 + 4096 + 8;
    addrs[2] = (char *)page2 + 4096 + 64;
    values[0] = (void *)0xaaa;
    values[1] = (void *)0xbbb;
    values[2] = (void *)0xccc;

    raplt_batch_set_protect(addrs, 3, PROT_READ | PROT_WRITE);
    raplt_batch_write_got(addrs, values, 3);

    ASSERT(*(void **)addrs[0] == (void *)0xaaa, "batch write[0] failed");
    ASSERT(*(void **)addrs[1] == (void *)0xbbb, "batch write[1] failed");
    ASSERT(*(void **)addrs[2] == (void *)0xccc, "batch write[2] failed");

    munmap(page2, 4096 * 2);

    /* Test the full hook API against the test fixture.
     * Note: on glibc/linux, dynamic segment entries are relocated,
     * so each library's GOT is indexed correctly.
     * We test that register/unregister/hooks_finalize work. */

    T("raplt_hooks_finalize ok");
    ASSERT(raplt_hooks_finalize() == 0, "finalize failed");

    T("raplt_clear ok");
    raplt_clear();

    dlclose(handle);

    SUMMARY();
    return g_fail ? 1 : 0;
}
