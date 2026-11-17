#ifndef VECTOR_H
#define VECTOR_H
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifndef vector_realloc
#define vector_realloc(cxt, ptr, size)  realloc((ptr), (size))
#endif
#ifndef vector_free
#define vector_free(cxt, ptr)           free(ptr)
#endif

#define vector_t(type)  struct {    \
    type *buf;                      \
    size_t len;                     \
    size_t capa;                    \
}

#define vector_init(v)              ((v).buf = NULL, (v).len = (v).capa = 0)
#define vector_destroy(cxt, v)      (vector_free(cxt, (v).buf))
#define vector_ptr(v)               ((v).buf)
#define vector_len(v)               ((v).len)
#define vector_set_len(v, n)        ((v).len = (n))
#define vector_get(v, i)            ((v).buf[i])
#define vector_set(v, i, x)         ((v).buf[i] = (x))

#define vector_reserve(cxt, v, type, n)     do {                        \
    if ((v).capa < (n)) {                                               \
        void *r;                                                        \
        (v).capa = (n);                                                 \
        r = vector_realloc(cxt, (v).buf, (sizeof(type) * (v).capa));    \
        if (r != NULL) (v).buf = r;                                     \
    }                                                                   \
} while(0)

#define vector_push(cxt, v, type, x)    do {                                \
    if ((v).len == (v).capa) {                                              \
        vector_reserve(cxt, v, type, ((v).capa == 0 ? 2 : (v).capa * 2));   \
    }                                                                       \
    (v).buf[(v).len++] = (x);                                               \
} while (0)

#define vector_copy(cxt, dest, src, type)   do {                \
    (dest).len = (src).len;                                     \
    vector_reserve(cxt, dest, type, (src).capa);                \
    memcpy((dest).buf, (src).buf, (sizeof(type) * (src).len));  \
} while(0)

#define vector_concat(cxt, v1, v2, type)    do {                        \
    vector_reserve(cxt, v1, type, (v1).len + (v2).len);                 \
    memcpy((v1).buf + (v1).len, (v2).buf, (sizeof(type) * (v2).len));   \
    (v1).len += (v2).len;                                               \
} while(0)

#endif /* VECTOR_H */
