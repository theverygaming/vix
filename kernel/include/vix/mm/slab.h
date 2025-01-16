#pragma once
#include <vix/status.h>
#include <vix/types.h>

namespace mm::slab {
    struct cache {
        const char *obj_name;
        size_t obj_size; // alignment = obj_size, obj_size MUST BE BELOW BLOCK/PAGE size!
        //void (*constructor)(void *obj);
        //void (*destructor)(void *obj);
        struct slab *slabs;
    };
    struct slab {
        size_t n_blocks;
        struct slab *next;
        struct slab_free *freelist;
    };
    struct slab_free {
        struct slab_free *prev;
        struct slab_free *next;
        size_t size;
    };

    status::StatusOr<struct cache *> create_cache(const char *obj_name, size_t obj_size);
}
