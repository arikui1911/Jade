#ifndef JADE_PRIVATE_ENV_H
#define JADE_PRIVATE_ENV_H
#include <jade.h>
#include "./vector_using.h"

typedef struct Environment Environment;

typedef struct {
    JadeID id;
    JadeValue value;
} Binding;

struct Environment {
    JadeState *jade;
    Environment *outer;
    vector_t(Binding) bindings;
};

Environment *env_new(JadeState *, Environment *);
void env_dispose(Environment *);

Environment *env_find(Environment *, JadeID);

JadeValue env_fetch(Environment *, JadeID);
void env_store(Environment *, JadeID, JadeValue);

#endif /* JADE_PRIVATE_ENV_H */
