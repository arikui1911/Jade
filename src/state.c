#include <jade.h>
#include "./state.h"
#include "./util.h"
#include "./node.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <khash.h>
#include "./vector_using.h"

static JadeValue b_print(JadeState *jade, JadeValue *args, int n_args){
    int i;
    for (i = 0; i < n_args; i++) {
        JadeValue s = jade_inspection(jade, args[i]);
        fprintf(stdout, "%s\n", STR_PTR(s));
    }
    return jade_nil_value();
}

static void prelude(JadeState *jade){
    jade_define_function(jade, "print", b_print);
}

JadeState *jade_new(void){
    JadeState *jade = NULL;
    khash_t(symbols) *syms = NULL;

    syms = kh_init(symbols);
    if (syms == NULL) return NULL;

    jade = malloc(sizeof(JadeState));
    if (jade == NULL) {
        kh_destroy(symbols, syms);
        return NULL;
    }

    jade->jump_ready = 0;
    jade->source_filename = NULL;
    jade->toplevel = NULL;
    jade->symbol_table = syms;
    jade->builtin = NULL;
    jade->global = NULL;
    jade->current = NULL;

    jade->heap = NULL;
    jade->heap_size = 0;
    vector_init(jade->nodes);
    vector_init(jade->symbol_names);
    vector_init(jade->arena);
    jade->builtin = env_new(jade, NULL);
    jade->global = env_new(jade, jade->builtin);
    jade->current = jade->global;

    prelude(jade);

    return jade;
}

void jade_dispose(JadeState *jade){
    int i;
    if (jade == NULL) return;

    for (i = 0; i < vector_len(jade->nodes); i++) dispose_node(vector_get(jade->nodes, i));
    vector_destroy(jade, jade->nodes);

    kh_destroy(symbols, jade->symbol_table);
    for (i = 0; i < vector_len(jade->symbol_names); i++) free(vector_get(jade->symbol_names, i));
    vector_destroy(jade, jade->symbol_names);

    heap_dispose(jade);
    vector_destroy(jade, jade->arena);

    env_dispose(jade->builtin);
    env_dispose(jade->global);

    free(jade);
}

void jade_run(JadeState *jade){
    eval(jade, jade->toplevel, jade->global);
}

void jade_dump_ast(JadeState *jade, FILE *fp){
    dump_node(jade->toplevel, fp, 0);
}

void jade_raise(JadeState *jade, const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    jade_raise_v(jade, fmt, args);
    va_end(args);
}

void jade_raise_v(JadeState *jade, const char *fmt, va_list args){
    if (!jade->jump_ready) {
        vfprintf(stderr, fmt, args);
        exit(1);
    }
    longjmp(jade->current_jump.buf, 1);
}

void *jrealloc(JadeState *jade, void *ptr, size_t n){
    void *r = realloc(ptr, n);
    if (r == NULL) {
        jade_gc(jade);
        r = realloc(ptr, n);
    }
    if (r == NULL) jade_raise(jade, "bad alloc");
    return r;
}

char *jstrdup(JadeState *jade, const char *s){
    size_t size = sizeof(char) * (strlen(s) + 1);
    char *d = jmalloc(jade, size);
    memcpy(d, s, size);
    return d;
}

char *jsprintf(JadeState *jade, const char *fmt, ...){
    int n;
    char *s;
    va_list args, copied;

    va_start(args, fmt);
    va_copy(copied, args);

    n = vsnprintf(NULL, 0, fmt, args);
    s = jmalloc(jade, sizeof(char) * (n + 1));
    vsnprintf(s, n + 1, fmt, copied);

    va_end(args);
    va_end(copied);
    return s;
}

JadeID jade_intern(JadeState *jade, const char *name){
    khiter_t k;
    int r;

    k = kh_put(symbols, jade->symbol_table, name, &r);
    if (r < 0) {
        jade_gc(jade);
        k = kh_put(symbols, jade->symbol_table, name, &r);
    }
    if (r < 0) jade_raise(jade, "hash table might failed to memory allocation.");

    if (r > 0) {
        kh_value(jade->symbol_table, k) = vector_len(jade->symbol_names);
        vector_push(jade, jade->symbol_names, char*, jstrdup(jade, name));
    }
    return kh_value(jade->symbol_table, k);
}

const char *jade_id_name(JadeState *jade, JadeID id){
    if (id < 0 || id >= vector_len(jade->symbol_names)) return NULL;
    return vector_get(jade->symbol_names, id);
}
