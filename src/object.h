#ifndef JADE_PRIVATE_OBJECT_H
#define JADE_PRIVATE_OBJECT_H
#include <jade.h>
#include <jade/ast.h>
#include "./value.h"
#include <stddef.h>
#include "./vector_using.h"
#include <khash.h>

typedef struct {
    char *ptr;
    size_t len;
} JadeString;

typedef struct {
    vector_t(JadeValue) vec;
} JadeArray;

KHASH_INIT(table, JadeValue, JadeValue, 1, hash_value, jade_equal);

typedef struct {
    khash_t(table) *ht;
} JadeTable;

typedef enum {
    JADE_USER_FUNCTION,
    JADE_NATIVE_FUNCTION,
} JadeFunctionType;

typedef struct {
    vector_t(JadeID) parameters;
    JadeNode *body;
} JadeUserFunction;

typedef struct {
    JadeValue (*body)(JadeState *, JadeValue *, int);
} JadeNativeFunction;

typedef struct {
    JadeFunctionType type;
    union {
        JadeUserFunction user;
        JadeNativeFunction native;
    } u;
} JadeFunction;

struct JadeObject {
    JadeValueType type;
    int mark;
    JadeObject *next;
    union {
        JadeString string;
        JadeArray array;
        JadeTable table;
        JadeFunction function;
    } u;
};

#define STR_PTR(v)  ((v).u.object->u.string.ptr)
#define STR_LEN(v)  ((v).u.object->u.string.len)

#define ARY_VEC(v)      ((v).u.object->u.array.vec)
#define ARY_LEN(v)      (vector_len((v).u.object->u.array.vec))
#define ARY_REF(v, i)   (vector_get((v).u.object->u.array.vec, (i)))
#define ARY_SET(v, i, x)    (vector_set((v).u.object->u.array.vec, (i), (x)))

#define TBL_LEN(v)  (kh_size((v).u.object->u.table.ht))

JadeValue object_reference(JadeObject *);
JadeObject *object_new(JadeState *, JadeValueType);
void object_dispose(JadeObject *);
void object_mark(JadeObject *);

JadeValue function_new(JadeState *, JadeNode *, JadeNode *);

void string_destroy(JadeObject *);
void array_destroy(JadeObject *);
void table_destroy(JadeObject *);
void function_destroy(JadeObject *);

void array_mark(JadeObject *);
void table_mark(JadeObject *);
void function_mark(JadeObject *);

#define VALUE_MARK(v)   do {                                \
    if (JADE_OBJECT_P((v).type)) object_mark((v).u.object); \
} while(0)

#endif /* JADE_PRIVATE_OBJECT_H */
