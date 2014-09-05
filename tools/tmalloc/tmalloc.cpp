#include <dlfcn.h>
#include <stdio.h>

#include <iostream>
#include <atomic>

namespace {
    std::atomic<std::size_t> n_alloc{};
    std::atomic<std::size_t> total_alloc{};
}

void* malloc(size_t sz)
{
    static auto libc_malloc = reinterpret_cast<void*(*)(size_t)>(dlsym(RTLD_NEXT, "malloc"));

    if (sz > -1u/4) {
        abort();
    }
    void * ptr = libc_malloc(sz);
    std::cout 
      << "malloc: " << ptr << "   " << sz << "    ("
      << ++n_alloc << ") (total: "
      << ++total_alloc << ")"
    << std::endl;
    return ptr;
}

void free(void *p)
{
    static auto libc_free = reinterpret_cast<void(*)(void*)>(dlsym(RTLD_NEXT, "free"));

    if (!p) {
        return ;
    }
    if (!n_alloc) {
        abort();
    }
    libc_free(p);
    std::cout << "free: " << p << "    (" << --n_alloc << ")" << std::endl;
}
