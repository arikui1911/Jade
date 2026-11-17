#ifndef JADE_PRIVATE_STATE_H
#define JADE_PRIVATE_STATE_H
#include <jade.h>
#include <jade/ast.h>
#include <setjmp.h>
#include <stddef.h>
#include <khash.h>
#include "./vector_using.h"
#include "./object.h"
#include "./env.h"

KHASH_MAP_INIT_STR(symbols, JadeID);

typedef enum {
    JUMP_INVALID,
    JUMP_BREAK,
    JUMP_CONTINUE,
    JUMP_RETURN,
    JUMP_THROW,
} JumpType;

typedef struct {
    jmp_buf buf;
    JumpType type;
    JadeValue value;
} Jump;

struct JadeState {
    Jump current_jump;
    int jump_ready;
    const char *source_filename;
    JadeNode *toplevel;
    khash_t(symbols) *symbol_table;
    vector_t(char*) symbol_names;
    JadeObject *heap;
    size_t heap_size;
    vector_t(JadeObject*) arena;
    Environment *builtin;
    Environment *global;
    Environment *current;
};

void heap_dispose(JadeState *);

JadeValue eval(JadeState *, JadeNode *, Environment *);

#endif /* JADE_PRIVATE_STATE_H */
