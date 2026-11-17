#include <jade.h>
#include "./object.h"
#include "./state.h"
#include "./env.h"
#include "./util.h"
#include "./vector_using.h"
#include <stdlib.h>
#include <assert.h>

JadeValue object_reference(JadeObject *o){
    JadeValue v;
    v.type = o->type;
    v.u.object = o;
    return v;
}

JadeObject *object_new(JadeState *jade, JadeValueType type){
    JadeObject *o = jmalloc(jade, sizeof(JadeObject));
    o->type = type;
    o->mark = 1;

    o->next = jade->heap;
    jade->heap = o;
    jade->heap_size++;

    vector_push(jade, jade->arena, JadeObject*, o);
    return o;
}

void object_dispose(JadeObject *o){
    if (o == NULL) return;
    switch (o->type) {
    case JADE_STRING:
        string_destroy(o);
        break;
    case JADE_ARRAY:
        array_destroy(o);
        break;
    case JADE_TABLE:
        table_destroy(o);
        break;
    case JADE_FUNCTION:
        function_destroy(o);
        break;
    default:
        assert(0);
    }
    free(o);
}

void heap_dispose(JadeState *jade){
    JadeObject *pos, *next;
    for (pos = jade->heap; pos != NULL; pos = next) {
        next = pos->next;
        object_dispose(pos);
    }
    jade->heap_size = 0;
}

void object_mark(JadeObject *o){
    if (o->mark) return;
    o->mark = 1;
    switch (o->type) {
    case JADE_STRING:
        break;
    case JADE_ARRAY:
        array_mark(o);
        break;
    case JADE_TABLE:
        table_mark(o);
        break;
    case JADE_FUNCTION:
        break;
    default:
        assert(0);
    }
}

void jade_gc(JadeState *jade){
    int i;
    JadeObject *pos, *next;
    Environment *env;

    // clear marks
    for (pos = jade->heap; pos != NULL; pos = pos->next) {
        pos->mark = 0;
    }

    // mark env
    for (env = jade->current; env != NULL; env = env->outer) {
        int i;
        for (i = 0; i < vector_len(env->bindings); i++) {
            if (JADE_OBJECT_P(vector_get(env->bindings, i).value.type)) {
                object_mark(vector_get(env->bindings, i).value.u.object);
            }
        }
    }

    // mark arena
    for (i = 0; i < vector_len(jade->arena); i++) {
        if (JADE_OBJECT_P(vector_get(jade->arena, i)->type)) object_mark(vector_get(jade->arena, i));
    }

    // sweep
    pos = jade->heap;
    jade->heap = NULL;
    jade->heap_size = 0;
    for (; pos != NULL; pos = next) {
        next = pos->next;
        if (pos->mark) {
            pos->next = jade->heap;
            jade->heap = pos;
            jade->heap_size++;
        } else {
            object_dispose(pos);
        }
    }
}

int jade_save_arena(JadeState *jade){
    return vector_len(jade->arena);
}

void jade_restore_arena(JadeState *jade, int ai){
    vector_set_len(jade->arena, ai);
}
