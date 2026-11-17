#include "./fnv.h"
#include <khash.h>

#define FNV_PRIME   (0x01000193U)

khint_t fnv1a_32(khint_t acc, const void *p, size_t n) {
    const unsigned char *bytes = (const unsigned char *)p;  // as byte array
    khint_t h = acc;
    size_t i;

    for (i = 0; i < n; i++) {
        h ^= bytes[i];
        h *= FNV_PRIME;
    }

    return h;
}
