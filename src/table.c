#include <jade.h>
#include "./state.h"
#include "./value.h"
#include "./object.h"
#include "./fnv.h"
#include <khash.h>

JadeValue jade_table_new(JadeState *jade){
    JadeObject *o = object_new(jade, JADE_TABLE);
    o->u.table.ht = kh_init(table);
    if (o->u.table.ht == NULL) {
        jade_gc(jade);
        o->u.table.ht = kh_init(table);
    }
    if (o->u.table.ht == NULL) jade_raise(jade, "bad alloc");
    return object_reference(o);
}

void table_destroy(JadeObject *o){
    kh_destroy(table, o->u.table.ht);
}

void table_mark(JadeObject *o){
    khiter_t k;
    for (k = kh_begin(o->u.table.ht); k != kh_end(o->u.table.ht); ++k) {
        if (!kh_exist(o->u.table.ht, k)) continue;
        VALUE_MARK(kh_key(o->u.table.ht, k));
        VALUE_MARK(kh_value(o->u.table.ht, k));
    }
}

#define TBL_KH(v)  ((v).u.object->u.table.ht)

khint_t table_hash_value(JadeValue t){
    khint_t h;
    khiter_t k;

    h = fnv1a_32(FNV_BASIS, &TBL_LEN(t), sizeof(khint_t));

    for (k = kh_begin(TBL_KH(t)); k != kh_end(TBL_KH(t)); ++k) {
        khint_t e;
        if (!kh_exist(TBL_KH(t), k)) continue;

        e = hash_value(kh_key(TBL_KH(t), k));
        h = fnv1a_32(h, &e, sizeof(khint_t));
        e = hash_value(kh_value(TBL_KH(t), k));
        h = fnv1a_32(h, &e, sizeof(khint_t));
    }

    return h;
}

int table_equal(JadeValue x, JadeValue y){
    khiter_t k;

    if (y.type != JADE_TABLE) return 0;
    if (TBL_LEN(x) != TBL_LEN(y)) return 0;

    for (k = kh_begin(TBL_KH(x)); k != kh_end(TBL_KH(x)); ++k) {
        khiter_t j;
        if (!kh_exist(TBL_KH(x), k)) continue;

        j = kh_get(table, TBL_KH(y), kh_key(TBL_KH(x), k));
        if (!jade_equal(kh_value(TBL_KH(x), k), kh_value(TBL_KH(y), j))) return 0;
    }

    return 1;
}

JadeValue jade_table_fetch(JadeState *jade, JadeValue t, JadeValue key){
    khiter_t k;

    if (t.type != JADE_TABLE) {
        jade_raise(jade, "not a table - %s", jade_value_type_name(t.type));
    }

    k = kh_get(table, TBL_KH(t), key);
    if (k == kh_end(TBL_KH(t))) return jade_nil_value();
    return kh_value(TBL_KH(t), k);
}

JadeValue jade_table_store(JadeState *jade, JadeValue t, JadeValue key, JadeValue val){
    khiter_t k;
    int r;

    if (t.type != JADE_TABLE) {
        jade_raise(jade, "not a table - %s", jade_value_type_name(t.type));
    }

    k = kh_put(table, TBL_KH(t), key, &r);
    if (r < 0) {
        jade_gc(jade);
        k = kh_put(table, TBL_KH(t), key, &r);
    }
    if (r < 0) jade_raise(jade, "hash table might failed to memory allocation.");

    kh_value(TBL_KH(t), k) = val;
    return val;
}

JadeValue table_tostring(JadeState *jade, JadeValue t, int inspection){
    JadeValue r;
    khiter_t k;
    int at_first, ai;
    vector_t(char) buf;
    if (t.type != JADE_TABLE) jade_raise(jade, "not a table - %s", jade_value_type_name(t.type));
    if (TBL_LEN(t) == 0) return jade_string_new(jade, "{}");

    vector_init(buf);
    vector_push(jade, buf, char, '{');

    ai = jade_save_arena(jade);
    at_first = 1;
    for (k = kh_begin(TBL_KH(t)); k != kh_end(TBL_KH(t)); ++k) {
        if (!kh_exist(TBL_KH(t), k)) continue;

        if (at_first) {
            at_first = 0;
        } else {
            vector_push(jade, buf, char, ',');
            vector_push(jade, buf, char, ' ');
        }
        append_to_buffer(jade, buf, tostring(jade, kh_key(TBL_KH(t), k), inspection));
        vector_push(jade, buf, char, ':');
        vector_push(jade, buf, char, ' ');
        append_to_buffer(jade, buf, tostring(jade, kh_value(TBL_KH(t), k), inspection));
    }
    jade_restore_arena(jade, ai);

    vector_push(jade, buf, char, '}');
    r = jade_string_new_len(jade, vector_ptr(buf), vector_len(buf));
    vector_destroy(jade, buf);
    return r;
}
