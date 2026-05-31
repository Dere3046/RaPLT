/* references: ShadowHook (MIT), Linux mremap(2) */

#ifndef RAPLT_MREMAP_H
#define RAPLT_MREMAP_H 1

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int raplt_mremap_patch_page(uintptr_t page_start,
                             void **got_entries, void *new_func,
                             int count, void **backup_out);

int raplt_mremap_restore_page(uintptr_t page_start, void *backup);

#ifdef __cplusplus
}
#endif

#endif
