#if defined(_MSC_VER)
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

// 64-bit
uint64_t MurmurHash64A(const void* key, int len, unsigned int seed);
// 32-bit
uint64_t MurmurHash64B(const void* key, int len, unsigned int seed);

#if defined(_LP64) || defined(__x86_64__) || defined(ia64) || defined(__ia64__) ||                 \
    defined(__sparc64__) || defined(_LP64) || defined(amd64) || defined(__amd64__) ||              \
    defined(__powerpc64__)
#define MURMUR64_64
#define MurmurHash64 MurmurHash64A
#else
#define MURMUR64_32
#define MurmurHash64 MurmurHash64B
#endif
