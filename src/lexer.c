#include <jade.h>
#include <jade/parser.h>
#include "./parser.h"
#include "./util.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

static int getch_core(JadeParser *p){
    if (p->saved_ch != EOF) {
        int c = p->saved_ch;
        p->saved_ch = EOF;
        return c;
    }
    switch (p->src_type) {
    case SRC_STRING:
        if (p->src.string.pos == p->src.string.len) return EOF;
        return p->src.string.ptr[p->src.string.pos++];
    case SRC_FILE:
        return fgetc(p->src.file);
    default:
        assert(0);
    }
}

static int getch(JadeParser *p){
    int c = getch_core(p);
    p->column++;
    if (c == '\n') {
        p->last_nl_col = p->column;
        p->column = 0;
        p->lineno++;
    }
    return c;
}

static void ungetch(JadeParser *p, int ch){
    p->saved_ch = ch;
    p->column--;
    if (p->column == 0) {
        p->column = p->last_nl_col;
        p->lineno--;
    }
}

void pushback(JadeParser *p, Token t){
    assert(p->n_saved_tokens < 2);
    p->saved_tokens[p->n_saved_tokens++] = t;
}

typedef struct {
    JadeState *jade;
    char *ptr;
    size_t len;
    size_t capa;
} Buffer;

static void init_buffer(JadeState *jade, Buffer *b){
    b->jade = jade;
    b->ptr = NULL;
    b->len = 0;
    b->capa = 0;
}

static void add_to_buffer(Buffer *b, int c){
    if (b->len == b->capa) {
        b->capa += 64;
        b->ptr = jrealloc(b->jade, b->ptr, sizeof(char) * b->capa);
    }
    b->ptr[b->len++] = c;
}

static char *finish_buffer(Buffer *b){
    char *s;
    s = jmalloc(b->jade, sizeof(char) * (b->len + 1));
    memcpy(s, b->ptr, sizeof(char) * b->len);
    s[b->len] = '\0';
    free(b->ptr);
    init_buffer(b->jade, b);
    return s;
}

typedef struct {
    const char *name;
    TokenType tt;
} Keyword;

static Keyword Keywords[] = {
    {"if", TOKEN_IF},
    {"elsif", TOKEN_ELSIF},
    {"else", TOKEN_ELSE},
    {"unless", TOKEN_UNLESS},
    {"while", TOKEN_WHILE},
    {"until", TOKEN_UNTIL},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {"for", TOKEN_FOR},
    {"def", TOKEN_DEF},
    {"return", TOKEN_RETURN},
    {"try", TOKEN_TRY},
    {"rescue", TOKEN_RESCUE},
    {"ensure", TOKEN_ENSURE},
    {"throw", TOKEN_THROW},
    {"match", TOKEN_MATCH},
    {"when", TOKEN_WHEN},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {"nil", TOKEN_NIL},
};

static TokenType lookup_keyword(const char *s){
    int i;
    for (i = 0; i < (sizeof(Keywords) / sizeof(Keyword)); i++) {
        if (strcmp(s, Keywords[i].name) == 0) return Keywords[i].tt;
    }
    return TOKEN_EOF;
}

static int newline_emit_required(JadeParser *p){
    switch (p->last_token_type) {
    case TOKEN_IDENTIFIER:
    case TOKEN_INT_LITERAL:
    case TOKEN_FLOAT_LITERAL:
    case TOKEN_STRING_LITERAL:
    case TOKEN_BREAK:
    case TOKEN_CONTINUE:
    case TOKEN_RETURN:
    case TOKEN_THROW:
    case TOKEN_TRUE:
    case TOKEN_FALSE:
    case TOKEN_NIL:
    case TOKEN_RP:
    case TOKEN_RB:
    case TOKEN_RC:
        return 1;
    default:
        break;
    }
    return 0;
}

typedef enum {
    LEX_INITIAL,
    LEX_SHARP,
    LEX_COMMENT_LINE,
    LEX_COMMENT_BLOCK,
    LEX_COMMENT_BLOCK_BAR,
    LEX_2ND_CHAR,
    LEX_STRING,
    LEX_STRING_ESC,
    LEX_ZERO,
    LEX_INT,
    LEX_FLOAT,
    LEX_IDENTIFIER,
} LexState;

static Token lex_next_token(JadeParser *p){
    Token t = {TOKEN_EOF, NULL, {1, 1, 1, 1}};
    LexState state = LEX_INITIAL;
    int c, fst;
    TokenType tt;
    Buffer buf;
    init_buffer(p->jade, &buf);

    while ((c = getch(p)) != EOF) {
        switch (state) {
        case LEX_INITIAL:
            t.pos.start_line = p->lineno;
            t.pos.start_column = p->column;
            t.pos.end_line = p->lineno;
            t.pos.end_column = p->column;
            if (c == '#') {
                state = LEX_SHARP;
            } else if (c == '\n') {
                if (newline_emit_required(p)) {
                    t.type = TOKEN_NEWLINE;
                    goto END;
                }
            } else if (isspace(c)) {
                // do nothing
            } else if (c == '}') {
                if (newline_emit_required(p)) {
                    ungetch(p, c);
                    t.type = TOKEN_NEWLINE;
                } else {
                    t.type = TOKEN_RC;
                }
                goto END;
            } else if (c == '0') {
                state = LEX_ZERO;
                add_to_buffer(&buf, c);
            } else if (isdigit(c)) {
                state = LEX_INT;
                add_to_buffer(&buf, c);
           } else if (c == '"') {
                state = LEX_STRING;
            } else if (isalpha(c)) {
                state = LEX_IDENTIFIER;
                add_to_buffer(&buf, c);
            } else {
                switch (c) {
                case '(':
                    t.type = TOKEN_LP;
                    goto END;
                case ')':
                    t.type = TOKEN_RP;
                    goto END;
                case '[':
                    t.type = TOKEN_LB;
                    goto END;
                case ']':
                    t.type = TOKEN_RB;
                    goto END;
                case '{':
                    t.type = TOKEN_LC;
                    goto END;
                case ',':
                    t.type = TOKEN_COMMA;
                    goto END;
                case ':':
                    t.type = TOKEN_COLON;
                    goto END;
                case ';':
                    t.type = TOKEN_SEMICOLON;
                    goto END;
                }

                state = LEX_2ND_CHAR;
                fst = c;
                switch (c) {
                case '=':
                    t.type = TOKEN_LET;
                    break;
                case '!':
                    t.type = TOKEN_BANG;
                    break;
                case '>':
                    t.type = TOKEN_GT;
                    break;
                case '<':
                    t.type = TOKEN_LT;
                    break;
                case '-':
                    t.type = TOKEN_SUB;
                    break;
                case '&':
                case '|':
                    break;
                case '+':
                    t.type = TOKEN_ADD;
                    break;
                case '*':
                    t.type = TOKEN_MUL;
                    break;
                case '/':
                    t.type = TOKEN_DIV;
                    break;
                case '%':
                    t.type = TOKEN_MOD;
                    break;
                default:
                    jade_raise(p->jade, "(%d:%d): invalid character - '%c'", t.pos.start_line, t.pos.start_column, c);
                }
            }
            break;
        case LEX_SHARP:
            if (c == '|') {
                state = LEX_COMMENT_BLOCK;
            } else if (c == '\n') {
                ungetch(p, c);
                state = LEX_INITIAL;
            }
            break;
        case LEX_COMMENT_LINE:
            if (c == '\n') {
                ungetch(p, c);
                state = LEX_INITIAL;
            }
            break;
        case LEX_COMMENT_BLOCK:
            if (c == '|') {
                state = LEX_COMMENT_BLOCK_BAR;
            }
            break;
        case LEX_COMMENT_BLOCK_BAR:
            if (c == '#') {
                state = LEX_INITIAL;
            } else {
                state = LEX_COMMENT_BLOCK;
            }
            break;
        case LEX_ZERO:
            if (c != '.') {
                ungetch(p, c);
                goto END;
            }
            add_to_buffer(&buf, c);
            state = LEX_FLOAT;
            break;
        case LEX_INT:
            if (c == '.') {
                add_to_buffer(&buf, c);
                state = LEX_FLOAT;
            } else if (isdigit(c)) {
                add_to_buffer(&buf, c);
            } else {
                ungetch(p, c);
                goto END;
            }
            break;
        case LEX_FLOAT:
            if (!isdigit(c)) {
                ungetch(p, c);
                goto END;
            }
            add_to_buffer(&buf, c);
            break;
        case LEX_STRING:
            if (c == '"') {
                t.type = TOKEN_STRING_LITERAL;
                t.value = finish_buffer(&buf);
                state = LEX_INITIAL;
                goto END;
            } else if (c == '\\') {
                state = LEX_STRING_ESC;
            } else {
                add_to_buffer(&buf, c);
            }
            break;
        case LEX_STRING_ESC:
            switch (c) {
            case 'n':
                add_to_buffer(&buf, '\n');
                break;
            default:
                add_to_buffer(&buf, c);
                break;
            }
            state = LEX_STRING;
            break;
        case LEX_IDENTIFIER:
            if (!isalnum(c) && c != '_') {
                ungetch(p, c);
                goto END;
            }
            add_to_buffer(&buf, c);
            break;
        case LEX_2ND_CHAR:
            state = LEX_INITIAL;
            tt = TOKEN_EOF;
            switch (fst) {
            case '=':
                if (c == '=') tt = TOKEN_EQ;
                break;
            case '!':
                if (c == '=') tt = TOKEN_NE;
                break;
            case '>':
                if (c == '=') tt = TOKEN_GE;
                break;
            case '<':
                if (c == '=') tt = TOKEN_LE;
                break;
            case '-':
                if (c == '>'){
                    tt = TOKEN_ARROW;
                } else if (c == '='){
                    tt = TOKEN_SUB_LET;
                }
                break;
            case '&':
                if (c == '&') tt = TOKEN_LAND;
                break;
            case '|':
                if (c == '|') tt = TOKEN_LOR;
                break;
            case '+':
                if (c == '=') tt = TOKEN_ADD_LET;
                break;
            case '*':
                if (c == '=') tt = TOKEN_MUL_LET;
                break;
            case '/':
                if (c == '=') tt = TOKEN_DIV_LET;
                break;
            case '%':
                if (c == '=') tt = TOKEN_MOD_LET;
                break;
            default:
                jade_raise(p->jade, "(%d:%d): invalid character - '%c'", t.pos.start_line, t.pos.start_column, fst);
            }
            if (tt == TOKEN_EOF) {
                ungetch(p, c);
            } else {
                t.type = tt;
            }
            goto END;
        default:
            assert(0);
        }
    }

END:
    t.pos.end_line = p->lineno;
    t.pos.end_column = p->column;
    if (c == EOF) t.pos.end_column--;   // EOFでトークンが終わったならungetchしてないから、その調整
    switch (state) {
    case LEX_INITIAL:
    case LEX_SHARP:
    case LEX_COMMENT_LINE:
        if (t.type == TOKEN_EOF && newline_emit_required(p)) {
            t.type = TOKEN_NEWLINE;
        }
        break;
    case LEX_COMMENT_BLOCK:
    case LEX_COMMENT_BLOCK_BAR:
        jade_raise(p->jade, "(%d:%d): unterminated block comment", t.pos.start_line, t.pos.start_column);
        break;
    case LEX_STRING:
    case LEX_STRING_ESC:
        jade_raise(p->jade, "(%d:%d): unterminated string literal", t.pos.start_line, t.pos.start_column);
        break;
    case LEX_ZERO:
    case LEX_INT:
        t.type = TOKEN_INT_LITERAL;
        t.value = finish_buffer(&buf);
        break;
    case LEX_FLOAT:
        t.type = TOKEN_FLOAT_LITERAL;
        t.value = finish_buffer(&buf);
        break;
    case LEX_IDENTIFIER:
        t.value = finish_buffer(&buf);
        t.type = lookup_keyword(t.value);
        if (t.type == TOKEN_EOF) {
            t.type = TOKEN_IDENTIFIER;
        } else {
            free(t.value);
            t.value = NULL;
        }
        break;
    case LEX_2ND_CHAR:
        if (fst == '&' || fst == '|') {
            jade_raise(p->jade, "(%d:%d): invalid character - '%c'", t.pos.start_line, t.pos.start_column, fst);
        }
        break;
    default:
        assert(0);
    }

    p->last_token_type = t.type;
    return t;
}

Token next_token(JadeParser *p) {
    if (p->n_saved_tokens > 0) {
        p->n_saved_tokens--;
        return p->saved_tokens[p->n_saved_tokens];
    }
    return lex_next_token(p);
}
