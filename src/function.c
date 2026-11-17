#include <jade.h>
#include "./value.h"
#include "./object.h"
#include "./node.h"
#include "./env.h"
#include "./state.h"
#include <assert.h>
#include "./vector_using.h"

JadeValue function_new(JadeState *jade, JadeNodeList *params, JadeNode *body){
    JadeNodeList *pos;
    JadeObject *o = object_new(jade, JADE_FUNCTION);

    o->u.function.type = JADE_USER_FUNCTION;

    vector_init(o->u.function.u.user.parameters);
    for (pos = params; pos != NULL; pos = pos->next) {
        vector_push(jade, o->u.function.u.user.parameters, JadeNode*, jade_intern(jade, pos->node->u.identifier.name));
    }

    o->u.function.u.user.body = copy_node(jade, body);

    return object_reference(o);
}

void jade_define_function(JadeState *jade, const char *name, JadeValue (*body)(JadeState *, JadeValue *, int)){
    JadeObject *o = object_new(jade, JADE_FUNCTION);

    o->u.function.type = JADE_NATIVE_FUNCTION;
    o->u.function.u.native.body = body;

    env_store(jade->builtin, jade_intern(jade, name), object_reference(o));
}

void function_destroy(JadeObject *o){
    switch (o->u.function.type) {
    case JADE_USER_FUNCTION:
        vector_destroy(jade, o->u.function.u.user.parameters);
        dispose_node(o->u.function.u.user.body);
        break;
    case JADE_NATIVE_FUNCTION:
        break;
    default:
        assert(0);
    }
}

JadeValue function_tostring(JadeState *jade, JadeValue v){
    JadeValue r;
    char *s;
    assert(v.type != JADE_FUNCTION);
    s = jsprintf(jade, "<#Function:%p>", v.u.object);
    r = jade_string_new(jade, s);
    free(s);
    return r;
}
