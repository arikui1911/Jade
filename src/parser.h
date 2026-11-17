#ifndef JADE_PRIVATE_PARSER_H
#define JADE_PRIVATE_PARSER_H
#include <jade.h>
#include <jade/parser.h>
#include <jade/ast.h>
#include <stdio.h>
#include <stddef.h>

typedef enum {
    TOKEN_INVALID,
    TOKEN_EOF,
    TOKEN_NEWLINE,

    TOKEN_LP,
    TOKEN_RP,
    TOKEN_LB,
    TOKEN_RB,
    TOKEN_LC,
    TOKEN_RC,
    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_GT,
    TOKEN_GE,
    TOKEN_LT,
    TOKEN_LE,
    TOKEN_LET,
    TOKEN_COMMA,
    TOKEN_COLON,
    TOKEN_SEMICOLON,
    TOKEN_ARROW,
    TOKEN_BANG,
    TOKEN_LAND,
    TOKEN_LOR,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_ADD_LET,
    TOKEN_SUB_LET,
    TOKEN_MUL_LET,
    TOKEN_DIV_LET,
    TOKEN_MOD_LET,

    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,

    TOKEN_IF,
    TOKEN_ELSIF,
    TOKEN_ELSE,
    TOKEN_UNLESS,
    TOKEN_WHILE,
    TOKEN_UNTIL,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_FOR,
    TOKEN_DEF,
    TOKEN_RETURN,
    TOKEN_TRY,
    TOKEN_RESCUE,
    TOKEN_ENSURE,
    TOKEN_THROW,
    TOKEN_MATCH,
    TOKEN_WHEN,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NIL,
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    JadePosition pos;
} Token;

typedef enum {
    SRC_STRING,
    SRC_FILE,
} SourceType;

typedef struct {
    const char *ptr;
    size_t pos;
    size_t len;
} StringSource;

struct JadeParser {
    JadeState *jade;
    const char *filename;
    int lineno;
    int column;
    int last_nl_col;
    int saved_ch;
    Token saved_tokens[2];
    int n_saved_tokens;
    TokenType last_token_type;
    SourceType src_type;
    union {
        StringSource string;
        FILE *file;
    } src;
    JadeNode *toplevel;
};

void set_string_source(JadeParser *, const char*, int, const char*);
void set_file_source(JadeParser *, const char*, int, FILE *);
Token next_token(JadeParser *);
void pushback(JadeParser *, Token);

#endif /* JADE_PRIVATE_PARSER_H */
