#include <jade.h>
#include "./value.h"
#include "./util.h"
#include "./fnv.h"
#include <khash.h>
#include <assert.h>
#include <stdint.h>

static const char *type_names[] = {
    "Undefined",
    "Nil",
    "Bool",
    "Int",
    "Float",
    "String",
    "Array",
    "Table",
    "Function",
};

const char *jade_value_type_name(JadeValueType t){
    if (t < 0 || t >= (sizeof(type_names) / sizeof(const char *))) return "???";
    return type_names[t];
}

JadeValue jade_undefined_value(void){
    JadeValue v = {JADE_UNDEFINED};
    v.u.int_value = 0;
    return v;
}

JadeValue jade_nil_value(void){
    JadeValue v = {JADE_NIL};
    v.u.int_value = 0;
    return v;
}

JadeValue jade_bool_value(int b){
    JadeValue v = {JADE_BOOL};
    v.u.int_value = b;
    return v;
}

JadeValue jade_int_value(int n){
    JadeValue v = {JADE_INT};
    v.u.int_value = n;
    return v;
}

JadeValue jade_float_value(double x){
    JadeValue v = {JADE_FLOAT};
    v.u.float_value = x;
    return v;
}

int jade_eq(JadeValue x, JadeValue y){
    if (x.type != y.type) return 0;
    switch (x.type) {
    case JADE_NIL:
        return 1;
    case JADE_BOOL:
    case JADE_INT:
        return x.u.int_value == y.u.int_value;
    case JADE_FLOAT:
        return x.u.float_value == y.u.float_value;
    case JADE_STRING:
    case JADE_ARRAY:
    case JADE_TABLE:
    case JADE_FUNCTION:
        return x.u.object == y.u.object;
    default:
        assert(0);
    }
}

int jade_equal(JadeValue x, JadeValue y){
    switch (x.type) {
    case JADE_INT:
        switch (y.type) {
        case JADE_INT:
            return x.u.int_value == y.u.int_value;
        case JADE_FLOAT:
            return x.u.int_value == y.u.float_value;
        default:
            return 0;
        }
    case JADE_FLOAT:
        switch (y.type) {
        case JADE_INT:
            return x.u.float_value == y.u.int_value;
        case JADE_FLOAT:
            return x.u.float_value == y.u.float_value;
        default:
            return 0;
        }
    case JADE_STRING:
        return string_equal(x, y);
    case JADE_ARRAY:
        return array_equal(x, y);
    case JADE_TABLE:
        return table_equal(x, y);
    default:
        return jade_eq(x, y);
    }
}

#define COMPARE(a, b)   ((a) == (b) ? 0 : ((a) < (b) ? -1 : 1))

int jade_compare(JadeValue x, JadeValue y){
    switch (x.type) {
    case JADE_INT:
        switch (y.type) {
        case JADE_INT:
            return COMPARE(x.u.int_value, y.u.int_value);
        case JADE_FLOAT:
            return COMPARE(x.u.int_value, y.u.float_value);
        default:
            break;
        }
    case JADE_FLOAT:
        switch (y.type) {
        case JADE_INT:
            return COMPARE(x.u.float_value, y.u.int_value);
        case JADE_FLOAT:
            return COMPARE(x.u.float_value, y.u.float_value);
        default:
            break;
        }
    case JADE_STRING:
        if (y.type == JADE_STRING) return string_compare(x, y);
    default:
        break;
    }
    return 2;
}

khint_t hash_value(JadeValue v){
    switch (v.type) {
    case JADE_NIL:
    case JADE_BOOL:
        return kh_int_hash_func(v.u.int_value + v.type);
    case JADE_INT:
        return kh_int_hash_func(v.u.int_value);
    case JADE_FLOAT:
        return fnv1a_32(FNV_BASIS, &v.u.float_value, sizeof(double));
    case JADE_STRING:
        return string_hash_value(v);
    case JADE_ARRAY:
        return array_hash_value(v);
    case JADE_TABLE:
        return table_hash_value(v);
    case JADE_FUNCTION:
        return (khint_t)((khint64_t)v.u.object % UINT32_MAX);
    default:
        assert(0);
    }
}

JadeValue tostring(JadeState *jade, JadeValue v, int inspection){
    JadeValue r;
    char *s;

    switch (v.type) {
    case JADE_NIL:
        return jade_string_new(jade, inspection ? "nil" : "");
    case JADE_BOOL:
        return jade_string_new(jade, (v.u.int_value ? "true" : "false"));
    case JADE_INT:
        s = jsprintf(jade, "%d", v.u.int_value);
        r = jade_string_new(jade, s);
        free(s);
        return r;
    case JADE_FLOAT:
        s = jsprintf(jade, "%lg", v.u.float_value);
        r = jade_string_new(jade, s);
        free(s);
        return r;
    case JADE_STRING:
        return string_tostring(jade, v, inspection);
    case JADE_ARRAY:
        return array_tostring(jade, v, inspection);
    case JADE_TABLE:
        return table_tostring(jade, v, inspection);
    case JADE_FUNCTION:
        return function_tostring(jade, v);
    default:
        assert(0);
    }
    return jade_undefined_value();
}

JadeValue jade_tostring(JadeState *jade, JadeValue v){
    return tostring(jade, v, 0);
}

JadeValue jade_inspection(JadeState *jade, JadeValue v){
    return tostring(jade, v, 1);
}
