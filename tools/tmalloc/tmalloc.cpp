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
    if (ptr) {
      std::cout
      << "malloc: " << ptr << "   " << sz << "    ("
      << ++n_alloc << ") (total: "
      << ++total_alloc << ")"
      << std::endl;
    }
    return ptr;
}

void *realloc(void *ptr, size_t sz)
{
    static auto libc_realloc = reinterpret_cast<void*(*)(void*,size_t)>(dlsym(RTLD_NEXT, "realloc"));

    if (sz > -1u/4) {
        abort();
    }
    void * newptr = libc_realloc(ptr, sz);
    if (newptr != ptr) {
        std::cout
        << "realloc: " << newptr << "   " << sz << "    ("
        << ++n_alloc << ") (total: "
        << ++total_alloc << ")"
        << std::endl;
    }
    return newptr;
}

// void *calloc(size_t nmemb, size_t sz)
// {
//     static auto libc_calloc = reinterpret_cast<void*(*)(size_t,size_t)>(dlsym(RTLD_NEXT, "calloc"));
//
//     if (sz > -1u/4) {
//         abort();
//     }
//     void * ptr = libc_calloc(nmemb, sz);
//     if (ptr) {
//         std::cout
//         << "calloc: " << ptr << "   " << sz << "    ("
//         << ++n_alloc << ") (total: "
//         << ++total_alloc << ")"
//         << std::endl;
//     }
//     return ptr;
// }

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
