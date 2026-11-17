#include <jade.h>
#include "./value.h"
#include "./object.h"
#include "./fnv.h"
#include "./vector_using.h"
#include <khash.h>
#include <stdio.h>

JadeValue jade_array_new(JadeState *jade){
    JadeObject *o = object_new(jade, JADE_ARRAY);
    vector_init(o->u.array.vec);
    return object_reference(o);
}

void array_destroy(JadeObject *o){
    vector_destroy(NULL, o->u.array.vec);
}

JadeValue jade_array_concat(JadeState *jade, JadeValue a, JadeValue b){
    JadeObject *o;

    if (a.type != JADE_ARRAY || b.type != JADE_ARRAY) {
        jade_raise(jade,
            "invalid concatenation - %s and %s",
            jade_value_type_name(a.type), jade_value_type_name(b.type)
        );
    }

    o = object_new(jade, JADE_ARRAY);
    vector_init(o->u.array.vec);
    vector_reserve(jade, o->u.array.vec, JadeValue, ARY_LEN(a) + ARY_LEN(b));
    vector_copy(jade, o->u.array.vec, ARY_VEC(a), JadeValue);
    vector_concat(jade, o->u.array.vec, ARY_VEC(b), JadeValue);
    return object_reference(o);
}

void array_mark(JadeObject *o){
    int i;
    for (i = 0; vector_len(o->u.array.vec); i++) {
        VALUE_MARK(vector_get(o->u.array.vec, i));
    }
}

khint_t array_hash_value(JadeValue a){
    khint_t h;
    int i;
    h = fnv1a_32(FNV_BASIS, &ARY_LEN(a), sizeof(size_t));

    for (i = 0; i < ARY_LEN(a); i++) {
        khint_t e = hash_value(ARY_REF(a, i));
        h = fnv1a_32(h, &e, sizeof(khint_t));
    }

    return h;
}

int array_equal(JadeValue x, JadeValue y){
    int i;

    if (y.type != JADE_ARRAY) return 0;
    if (ARY_LEN(x) != ARY_LEN(y)) return 0;

    for (i = 0; i < ARY_LEN(x); i++) {
        if (!jade_equal(ARY_REF(x, i), ARY_REF(y, i))) return 0;
    }

    return 1;
}

JadeValue jade_array_push(JadeState *jade, JadeValue a, JadeValue x){
    if (a.type != JADE_ARRAY) jade_raise(jade, "not an array - %s", jade_value_type_name(a.type));
    vector_push(jade, ARY_VEC(a), JadeValue, x);
    return a;
}

JadeValue jade_array_fetch(JadeState *jade, JadeValue a, JadeValue k){
    if (a.type != JADE_ARRAY) jade_raise(jade, "not an array - %s", jade_value_type_name(a.type));
    if (k.type != JADE_INT) jade_raise(jade, "not an int - %s", jade_value_type_name(k.type));

    if (k.u.int_value < 0 || k.u.int_value >= ARY_LEN(k)) {
        jade_raise(jade, "array index out of bounds - %d", k.u.int_value);
    }
    
    return ARY_REF(a, k.u.int_value);
}

JadeValue jade_array_store(JadeState *jade, JadeValue a, JadeValue k, JadeValue v){
    if (a.type != JADE_ARRAY) jade_raise(jade, "not an array - %s", jade_value_type_name(a.type));
    if (k.type != JADE_INT) jade_raise(jade, "not an int - %s", jade_value_type_name(k.type));

    if (k.u.int_value < 0 || k.u.int_value >= ARY_LEN(k)) {
        jade_raise(jade, "array index out of bounds - %d", k.u.int_value);
    }

    ARY_SET(a, k.u.int_value, v);
    return v;
}

JadeValue array_tostring(JadeState *jade, JadeValue a, int inspection){
    JadeValue r;
    int i, ai;
    vector_t(char) buf;
    if (a.type != JADE_ARRAY) jade_raise(jade, "not an array - %s", jade_value_type_name(a.type));
    if (ARY_LEN(a) == 0) return jade_string_new(jade, "[]");

    vector_init(buf);
    vector_push(jade, buf, char, '[');

    ai = jade_save_arena(jade);
    for (i = 0; i < ARY_LEN(a); i++) {
        if (i != 0) {
            vector_push(jade, buf, char, ',');
            vector_push(jade, buf, char, ' ');
        }
        append_to_buffer(jade, buf, tostring(jade, ARY_REF(a, i), inspection));
    }
    jade_restore_arena(jade, ai);

    vector_push(jade, buf, char, ']');
    r = jade_string_new_len(jade, vector_ptr(buf), vector_len(buf));
    vector_destroy(jade, buf);
    return r;
}
