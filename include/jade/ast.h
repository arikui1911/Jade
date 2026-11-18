#ifndef JADE_AST_H
#define JADE_AST_H
#include <stddef.h>

typedef struct {
    int start_line;
    int start_column;
    int end_line;
    int end_column;
} JadePosition;

typedef enum {
    JADE_LIST_NODE,
    JADE_BLOCK_NODE,
    JADE_EXPRESSION_STATEMENT_NODE,
    JADE_DEF_NODE,
    JADE_IF_NODE,
    JADE_WHILE_NODE,
    JADE_UNTIL_NODE,
    JADE_FOR_NODE,
    JADE_BREAK_NODE,
    JADE_CONTINUE_NODE,
    JADE_RETURN_NODE,
    JADE_THROW_NODE,
    JADE_TRY_NODE,
    JADE_IDENTIFIER_NODE,
    JADE_INT_LITERAL_NODE,
    JADE_FLOAT_LITERAL_NODE,
    JADE_STRING_LITERAL_NODE,
    JADE_BOOLEAN_LITERAL_NODE,
    JADE_NIL_LITERAL_NODE,
    JADE_ARRAY_LITERAL_NODE,
    JADE_TABLE_LITERAL_NODE,
    JADE_TABLE_ENTRY_NODE,
    JADE_FUNCTION_LITERAL_NODE,
    JADE_MINUS_NODE,
    JADE_NOT_NODE,
    JADE_LOGICAL_OR_NODE,
    JADE_LOGICAL_AND_NODE,
    JADE_EQUAL_NODE,
    JADE_NOT_EQUAL_NODE,
    JADE_LESS_THAN_NODE,
    JADE_GREATER_THAN_NODE,
    JADE_LESS_THAN_EQUAL_NODE,
    JADE_GREATER_THAN_EQUAL_NODE,
    JADE_ADDITION_NODE,
    JADE_SUBTRACTION_NODE,
    JADE_MULTIPLICATION_NODE,
    JADE_DIVISION_NODE,
    JADE_MODULO_NODE,
    JADE_LET_NODE,
    JADE_CALL_NODE,
    JADE_FETCH_NODE,
    JADE_STORE_NODE,
    JADE_MATCH_NODE,
    JADE_MATCH_CLAUSE_NODE,
} JadeNodeType;

typedef struct JadeNode JadeNode;

typedef struct {
    JadeNode *next;
    JadeNode *node;
} JadeListNodeAttributes;

typedef struct {
    JadeNode *statements;
} JadeBlockNodeAttributes;

typedef struct {
    JadeNode *expression;
} JadeExpressionStatementNodeAttributes;

typedef struct {
    JadeNode *name;
    JadeNode *init;
} JadeDefNodeAttributes;

typedef struct {
    JadeNode *test;
    JadeNode *body;
    JadeNode *alt;
} JadeIfNodeAttributes;

typedef struct {
    JadeNode *cond;
    JadeNode *body;
} JadeLoopNodeAttributes;

typedef struct {
    JadeNode *variables;
    JadeNode *range;
    JadeNode *body;
} JadeForNodeAttributes;

typedef struct {
    JadeNode *result;
} JadeControlTransferNodeAttributes;

typedef struct {
    JadeNode *body;
    JadeNode *variable;
    JadeNode *rescue;
    JadeNode *ensure;
} JadeTryNodeAttributes;

typedef struct {
    char *name;
} JadeIdentifierNodeAttributes;

typedef struct {
    int value;
} JadeIntLiteralNodeAttributes;

typedef struct {
    double value;
} JadeFloatLiteralNodeAttributes;

typedef struct {
    char *value;
} JadeStringLiteralNodeAttributes;

typedef struct {
    int value;
} JadeBooleanLiteralNodeAttributes;

typedef struct {
    JadeNode *elements;
} JadeArrayLiteralNodeAttributes;

typedef struct {
    JadeNode *entries;
} JadeTableLiteralNodeAttributes;

typedef struct {
    JadeNode *key;
    JadeNode *value;
} JadeTableEntryNodeAttributes;

typedef struct {
    JadeNode *parameters;
    JadeNode *body;
} JadeFunctionLiteralNodeAttributes;

typedef struct {
    JadeNode *operand;
} JadeUnaryExpressionNodeAttributes;

typedef struct {
    JadeNode *left;
    JadeNode *right;
} JadeBinaryExpressionNodeAttributes;

typedef struct {
    JadeNode *name;
    JadeNode *operand;
} JadeLetNodeAttributes;

typedef struct {
    JadeNode *function;
    JadeNode *arguments;
} JadeCallNodeAttributes;

typedef struct {
    JadeNode *container;
    JadeNode *key;
} JadeFetchNodeAttributes;

typedef struct {
    JadeNode *fetch;
    JadeNode *value;
} JadeStoreNodeAttributes;

typedef struct {
    JadeNode *operand;
    JadeNode *variable;
    JadeNode *clauses;
    JadeNode *otherwise;
} JadeMatchNodeAttributes;

typedef struct {
    JadeNode *pattern;
    JadeNode *body;
} JadeMatchClauseNodeAttributes;

struct JadeNode {
    JadePosition pos;
    JadeNodeType type;
    union {
        JadeListNodeAttributes list;
        JadeBlockNodeAttributes block;
        JadeExpressionStatementNodeAttributes expression_statement;
        JadeDefNodeAttributes def;
        JadeIfNodeAttributes if_;
        JadeLoopNodeAttributes loop;
        JadeForNodeAttributes for_;
        JadeControlTransferNodeAttributes control_transfer;
        JadeTryNodeAttributes try;
        JadeIdentifierNodeAttributes identifier;
        JadeIntLiteralNodeAttributes int_literal;
        JadeFloatLiteralNodeAttributes float_literal;
        JadeStringLiteralNodeAttributes string_literal;
        JadeBooleanLiteralNodeAttributes boolean_literal;
        JadeArrayLiteralNodeAttributes array_literal;
        JadeTableLiteralNodeAttributes table_literal;
        JadeTableEntryNodeAttributes table_entry;
        JadeFunctionLiteralNodeAttributes function_literal;
        JadeUnaryExpressionNodeAttributes unary_expression;
        JadeBinaryExpressionNodeAttributes binary_expression;
        JadeLetNodeAttributes let;
        JadeCallNodeAttributes call;
        JadeFetchNodeAttributes fetch;
        JadeStoreNodeAttributes store;
        JadeMatchNodeAttributes match;
        JadeMatchClauseNodeAttributes match_clause;
    } u;
};

#endif /* JADE_AST_H */
