#ifndef JADE_H
#define JADE_H
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

typedef struct JadeState JadeState;

JadeState *jade_new(void);
void jade_dispose(JadeState *);

void jade_raise(JadeState *, const char *, ...);
void jade_raise_v(JadeState *, const char *, va_list);
void jade_run(JadeState *);

void jade_dump_ast(JadeState *, FILE *);

void jade_gc(JadeState *);
int jade_save_arena(JadeState *);
void jade_restore_arena(JadeState *, int);

typedef unsigned int JadeID;

JadeID jade_intern(JadeState *, const char *);
const char *jade_id_name(JadeState *, JadeID);

typedef enum {
    JADE_UNDEFINED,
    JADE_NIL,
    JADE_BOOL,
    JADE_INT,
    JADE_FLOAT,
    JADE_STRING,
    JADE_ARRAY,
    JADE_TABLE,
    JADE_FUNCTION,
} JadeValueType;

const char *jade_value_type_name(JadeValueType);

#define JADE_IMMEDIATE_P(t)     ((t) >= JADE_NIL && (t) <= JADE_FLOAT)
#define JADE_OBJECT_P(t)        ((t) >= JADE_STRING && (t) <= JADE_FUNCTION)

typedef struct JadeObject JadeObject;

typedef struct {
    JadeValueType type;
    union {
        int int_value;
        double float_value;
        JadeObject *object;
    } u;
} JadeValue;

JadeValue jade_undefined_value(void);
JadeValue jade_nil_value(void);
JadeValue jade_bool_value(int);
JadeValue jade_int_value(int);
JadeValue jade_float_value(double);


JadeValue jade_string_new(JadeState *, const char *);
JadeValue jade_string_new_len(JadeState *, const char *, size_t);
JadeValue jade_string_dump(JadeState *, JadeValue);
JadeValue jade_string_concat(JadeState *, JadeValue, JadeValue);

JadeValue jade_array_new(JadeState *);
JadeValue jade_array_push(JadeState *, JadeValue, JadeValue);
JadeValue jade_array_concat(JadeState *, JadeValue, JadeValue);
JadeValue jade_array_fetch(JadeState *, JadeValue, JadeValue);
JadeValue jade_array_store(JadeState *, JadeValue, JadeValue, JadeValue);

JadeValue jade_table_new(JadeState *);
JadeValue jade_table_fetch(JadeState *, JadeValue, JadeValue);
JadeValue jade_table_store(JadeState *, JadeValue, JadeValue, JadeValue);

void jade_define_function(JadeState *, const char *, JadeValue (*)(JadeState *, JadeValue *, int));

#define jade_test(v)    ((v).type == JADE_NIL || ((v).type == JADE_BOOL && (v).u.int_value))

int jade_eq(JadeValue, JadeValue);
int jade_equal(JadeValue, JadeValue);
// ( < | = | > ) -- ( -1 | 0 | 1 ): else -> error
int jade_compare(JadeValue, JadeValue);

JadeValue jade_tostring(JadeState *, JadeValue);
JadeValue jade_inspection(JadeState *, JadeValue);

#endif /* JADE_H */
