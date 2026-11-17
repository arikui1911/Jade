#include <jade.h>
#include "./util.h"
#include "./value.h"
#include "./object.h"
#include <khash.h>
#include "./vector_using.h"
#include <string.h>
#include <assert.h>

JadeValue jade_string_new(JadeState *jade, const char *s){
    return jade_string_new_len(jade, s, strlen(s));
}

JadeValue jade_string_new_len(JadeState *jade, const char *ptr, size_t len){
    JadeObject *o = object_new(jade, JADE_STRING);
    o->u.string.len = len;
    o->u.string.ptr = jmalloc(jade, sizeof(char) * (len + 1));
    memcpy(o->u.string.ptr, ptr, sizeof(char) * len);
    o->u.string.ptr[len] = '\0';
    return object_reference(o);
}

void string_destroy(JadeObject *o){
    free(o->u.string.ptr);
}

JadeValue jade_string_concat(JadeState *jade, JadeValue a, JadeValue b){
    JadeObject *o;
    size_t n;

    if (a.type != JADE_STRING || b.type != JADE_STRING) {
        jade_raise(jade,
            "invalid concatenation - %s and %s",
            jade_value_type_name(a.type), jade_value_type_name(b.type)
        );
    }

    o = object_new(jade, JADE_STRING);
    o->u.string.len = STR_LEN(a) + STR_LEN(b);
    o->u.string.ptr = jmalloc(jade, sizeof(char) * (o->u.string.len + 1));
    
    n = sizeof(char) * STR_LEN(a);
    memcpy(o->u.string.ptr, STR_PTR(a), n);
    memcpy(o->u.string.ptr + n, STR_PTR(b), sizeof(char) * STR_LEN(b));
    o->u.string.ptr[o->u.string.len] = '\0';
    
    return object_reference(o);

}

khint_t string_hash_value(JadeValue v){
    return kh_str_hash_func(STR_PTR(v));
}

int string_equal(JadeValue x, JadeValue y){
    assert(x.type == JADE_STRING);
    if (y.type != JADE_STRING) return 0;
    if (STR_LEN(x) != STR_LEN(y)) return 0;
    return memcmp(STR_PTR(x), STR_PTR(y), sizeof(char) * STR_LEN(x)) == 0;
}

int string_compare(JadeValue x, JadeValue y){
    int r;

    assert(x.type == JADE_STRING);
    assert(y.type == JADE_STRING);

    r = strcmp(STR_PTR(x), STR_PTR(y));
    if (r < 0) return -1;
    if (r > 0) return 1;
    return 0;
}

JadeValue string_tostring(JadeState *jade, JadeValue v, int inspection){
    assert(v.type == JADE_STRING);
    if (!inspection) return v;
    return jade_string_dump(jade, v);
}

JadeValue jade_string_dump(JadeState *jade, JadeValue s){
    JadeValue r;
    int i;
    vector_t(char) buf;
    assert(s.type == JADE_STRING);

    vector_init(buf);
    vector_push(jade, buf, char, '"');
    
    for (i = 0; i < STR_LEN(s); i++) {
        char c = STR_PTR(s)[i];

        switch (c) {
        case '"':
            vector_push(jade, buf, char, '\\');
            vector_push(jade, buf, char, '"');
            break;
        case '\\':
            vector_push(jade, buf, char, '\\');
            vector_push(jade, buf, char, '\\');
            break;
        case '\n':
            vector_push(jade, buf, char, '\\');
            vector_push(jade, buf, char, 'n');
            break;
        default:
            vector_push(jade, buf, char, c);
        }
    }
    
    vector_push(jade, buf, char, '"');
    r = jade_string_new_len(jade, vector_ptr(buf), vector_len(buf));
    vector_destroy(jade, buf);
    return r;
}

