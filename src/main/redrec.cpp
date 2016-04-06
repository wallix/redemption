#include <iostream>
#include <cstring>

#include <dlfcn.h>

inline void usage(char const * prog) {
  std::cerr << prog << ": Usage library.so [hex-hmac_key hex-key] -- lib-args\n";
}

inline bool is_sep(char const * s) {
  return s[0] == '-' && s[1] == '-' && !s[2];
}

char const * g_hmac_key_str = nullptr;
char const * g_key_str = nullptr;

inline int get_hmac_key_prototype_fn(char * buffer) {
  memcpy(buffer, g_hmac_key_str, 32);
  return 0;
}

inline int get_trace_key_prototype_fn(char *, int, char * buffer) {
  memcpy(buffer, g_key_str, 32);
  return 0;
}

int main(int argc, char** argv) {
  if (argc < 3 || ! (is_sep(argv[2]) || (!is_sep(argv[2]) && argc > 4 && is_sep(argv[4])))) {
    usage(argv[0]);
    return -1;
  }
  
  unsigned arg_consumed = 2;
  char hmac_key_str[] =
    "\xe3\x8d\xa1\x5e\x50\x1e\x4f\x6a\x01\xef\xde\x6c\xd9\xb3\x3a\x3f"
    "\x2b\x41\x72\x13\x1e\x97\x5b\x4c\x39\x54\x23\x14\x43\xae\x22\xae";
  char key_str[] = 
    "\x56\x3e\xb6\xe8\x15\x8f\x0e\xed\x2e\x5f\xb6\xbc\x28\x93\xbc\x15"
    "\x27\x0d\x7e\x78\x15\xfa\x80\x4a\x72\x3e\xf4\xfb\x31\x5f\xf4\xb2";
  g_hmac_key_str = hmac_key_str;
  g_key_str = key_str;

  if (!is_sep(argv[2])) {
    if (strlen(argv[2]) != 64 || strlen(argv[3]) != 64) {
      std::cerr << "hmac_key or key len is not 64\n";
    }
    auto ka = argv + 2;
    for (auto a : {hmac_key_str, key_str}) {
      auto k = *ka;
      for (unsigned i = 0; i < 32; ++i) {
        auto char_to_hex = [](char c){
          auto in = [&c](char left, char right) { return left <= c && c <= right; };
          return 
              in('0', '9') ? c-'0'
            : in('a', 'f') ? c-'a'
            : in('A', 'F') ? c-'A'
            : -1;
        };
        unsigned const c1 = char_to_hex(k[i*2]);
        unsigned const c2 = char_to_hex(k[i*2+1]);
        a[i] = static_cast<char>(c1 << 4 | c2);
      }
      ++ka;
    }
    arg_consumed += 2;
  }

  void * handle = dlopen(argv[1], RTLD_LAZY);
  if (!handle) {
    std::cerr << dlerror() << '\n';
    return -2;
  }

  dlerror();    /* Clear any existing error */

  using do_main_t = int(
    int argc, char** argv, 
    decltype(get_hmac_key_prototype_fn) * hmac_fn, 
    decltype(get_trace_key_prototype_fn) * trace_fn
  );
  do_main_t * do_main;
  *reinterpret_cast<void**>(&do_main) = dlsym(handle, "do_main");

  auto const len = strlen(argv[0]);
  memcpy(argv[arg_consumed] + 2 - len, argv[0], len);
  argv[arg_consumed] = argv[arg_consumed] - len;
  return (*do_main)(
    argc - arg_consumed,
    argv + arg_consumed,
    get_hmac_key_prototype_fn, 
    get_trace_key_prototype_fn
  );
}
