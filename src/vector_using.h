#ifndef VECTOR_USING_PRIVATE_H
#define VECTOR_USING_PRIVATE_H
#include "./util.h"

#ifdef vector_realloc
#undef vector_realloc
#endif
#define vector_realloc(cxt, ptr, size)  jrealloc((cxt), (ptr), (size))
#include <vector.h>

#define append_to_buffer(jade_, buf_, str_)      do {                                           \
        int i_;                                                                                 \
        JadeValue s_ = (str_);                                                                  \
        vector_reserve(jade_, buf_, char, vector_len(buf_) + STR_LEN(s_));                      \
        for (i_ = 0; i_ < STR_LEN(s_); i_++) vector_push(jade_, buf_, char, STR_PTR(s_)[i_]);   \
    } while(0)

#endif /* VECTOR_USING_PRIVATE_H */
