#ifndef JADE_PRIVATE_FNV_H
#define JADE_PRIVATE_FNV_H
#include <khash.h>

#define FNV_BASIS   (0x811c9dc5U)

khint_t fnv1a_32(khint_t, const void *, size_t);

#endif /* JADE_PRIVATE_FNV_H */
