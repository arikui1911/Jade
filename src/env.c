#include <jade.h>
#include "./env.h"
#include "./util.h"
#include "./vector_using.h"

Environment *env_new(JadeState *jade, Environment *outer){
    Environment *env = jmalloc(jade, sizeof(Environment));
    env->jade = jade;
    env->outer = outer;
    vector_init(env->bindings);
    return env;
}

void env_dispose(Environment *env){
    if (env == NULL) return;
    vector_destroy(env->jade, env->bindings);
    free(env);
}

Binding *search(Environment *env, JadeID k) {
    int i;
    for (i = 0; i < vector_len(env->bindings); i++) {
        if (vector_get(env->bindings, i).id == k) return &(vector_get(env->bindings, i));
    }
    return NULL;
}

Environment *env_find(Environment *env, JadeID k){
    Binding *b = search(env, k);
    if (b != NULL) return env;
    if (env->outer != NULL) return env_find(env->outer, k);
    return NULL;
}

JadeValue env_fetch(Environment *env, JadeID k){
    Binding *b = search(env, k);
    if (b == NULL) return jade_nil_value();
    return b->value;
}

void env_store(Environment *env, JadeID k, JadeValue v){
    Binding *b = search(env, k);
    if (b == NULL) {
        Binding x = {k, v};
        vector_push(env->jade, env->bindings, Binding, x);
    } else {
        b->value = v;
    }
}
