#include <jade.h>
#include <jade/parser.h>
#include "./parser.h"
#include "./state.h"
#include "./node.h"
#include "./util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

JadeParser *jade_parser_new(JadeState *jade){
    JadeParser *p;
    p = jmalloc(jade, sizeof(JadeParser));
    p->jade = jade;
    return p;
}

void jade_parser_dispose(JadeParser *p){
    if (p == NULL) return;
    free(p);
}

static void do_parse(JadeParser *);

static void reset(JadeParser *p, const char* filename, int lineno){
    p->filename = filename;
    p->lineno = lineno;
    p->column = 0;
    p->last_nl_col = 0;
    p->saved_ch = EOF;
    p->n_saved_tokens = 0;
    p->last_token_type = TOKEN_INVALID;
    p->toplevel = NULL;
}

void set_string_source(JadeParser *p, const char* filename, int lineno, const char* src){
    reset(p, filename, lineno);
    p->src_type = SRC_STRING;
    p->src.string.ptr = src;
    p->src.string.pos = 0;
    p->src.string.len = strlen(src);
}

void jade_parser_parse_string(JadeParser *p, const char* filename, int lineno, const char* src){
    set_string_source(p, filename, lineno, src);
    do_parse(p);

    // move AST
    p->jade->source_filename = p->filename;
    p->jade->toplevel = p->toplevel;
    p->toplevel = NULL;
}

void set_file_source(JadeParser *p, const char* filename, int lineno, FILE *fp){
    reset(p, filename, lineno);
    p->src_type = SRC_FILE;
    p->src.file = fp;
}

void jade_parser_parse_file(JadeParser *p, const char* filename, int lineno, FILE *fp){
    set_file_source(p, filename, lineno, fp);
    do_parse(p);

    // move AST
    p->jade->source_filename = p->filename;
    p->jade->toplevel = p->toplevel;
    p->toplevel = NULL;
}

static const char *token_type_names[] = {
    "INVALID",
    "EOF",
    "NEWLINE",
    "LP",
    "RP",
    "LB",
    "RB",
    "LC",
    "RC",
    "EQ",
    "NE",
    "GT",
    "GE",
    "LT",
    "LE",
    "LET",
    "COMMA",
    "COLON",
    "SEMICOLON",
    "ARROW",
    "BANG",
    "LAND",
    "LOR",
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MOD",
    "ADD_LET",
    "SUB_LET",
    "MUL_LET",
    "DIV_LET",
    "MOD_LET",
    "IDENTIFIER",
    "INT_LITERAL",
    "FLOAT_LITERAL",
    "STRING_LITERAL",
    "IF",
    "ELSIF",
    "ELSE",
    "UNLESS",
    "WHILE",
    "UNTIL",
    "BREAK",
    "CONTINUE",
    "FOR",
    "DEF",
    "RETURN",
    "TRY",
    "RESCUE",
    "ENSURE",
    "THROW",
    "MATCH",
    "WHEN",
    "TRUE",
    "FALSE",
    "NIL",
};

static const char *token_type_name(TokenType tt){
    if (tt < 0 || tt >= TOKEN_NIL) return "???";
    return token_type_names[tt];
}

static JadePosition between(JadePosition a, JadePosition b){
    a.end_line = b.end_line;
    a.end_column = b.end_column;
    return a;
}

static void parse_error(JadeParser *p, Token t, const char *msg) {
    jade_raise(
        p->jade, "(%d:%d):(%d:%d): %s %s",
        t.pos.start_line, t.pos.start_column, t.pos.end_line, t.pos.end_column, msg,
        token_type_name(t.type)
    );
}

static void unexpected(JadeParser *p, Token t){
    jade_raise(
        p->jade, "(%d:%d):(%d:%d): unexpected %s",
        t.pos.start_line, t.pos.start_column, t.pos.end_line, t.pos.end_column,
        token_type_name(t.type)
    );
}

static int expect(JadeParser *p, TokenType type, Token *dest){
    Token t = next_token(p);
    if (t.type != type) {
        pushback(p, t);
        return 0;
    }
    if (dest != NULL) *dest = t;
    return 1;
}

static int expect_eos(JadeParser *p, Token *dest){
    Token t = next_token(p);
    if (t.type != TOKEN_NEWLINE && t.type != TOKEN_SEMICOLON) {
        pushback(p, t);
        return 0;
    }
    if (dest != NULL) *dest = t;
    return 1;
}

static Token must_be(JadeParser *p, TokenType want){
    Token t = next_token(p);
    if (t.type != want) {
        jade_raise(
            p->jade, "(%d:%d):(%d:%d): expect %s but %s",
            t.pos.start_line, t.pos.start_column, t.pos.end_line, t.pos.end_column,
            token_type_name(want), token_type_name(t.type)
        );
    }
    return t;
}

static Token must_be_eos(JadeParser *p){
    Token t = next_token(p);
    if (t.type != TOKEN_NEWLINE && t.type != TOKEN_SEMICOLON) {
        parse_error(p, t, "expect NEWLINE | SEMICOLON but");
    }
    return t;
}

static JadeNode *parse_statement(JadeParser *);

static JadeNode *parse_statements(JadeParser *p, TokenType term, Token *dest){
    JadeNode *s, *list = NULL;
    
    while (!expect(p, term, dest)) {
        s = parse_statement(p);
        if (s == NULL) continue;
        list = create_list_entry(p->jade, list, s);
    }

    return list;
}

static JadeNode *parse_block(JadeParser *p){
    JadeNode *ss;
    Token rc;
    Token lc = must_be(p, TOKEN_LC);

    ss = parse_statements(p, TOKEN_RC, &rc);
    return create_block(p->jade, between(lc.pos, rc.pos), ss);
}

static JadeNode *parse_comma_list(JadeParser *p, TokenType term, Token *dest, JadeNode *(*f)(JadeParser *)){
    JadeNode *list = NULL;
    Token t;

    if (expect(p, term, dest)) return NULL;

    for (;;) {
        list = create_list_entry(p->jade, list, f(p));

        t = next_token(p);
        if (t.type == term) {
            break;
        } else if (t.type == TOKEN_COMMA) {
            if (expect(p, term, &t)) break;
        } else if (t.type == TOKEN_NEWLINE) {
            t = must_be(p, term);
            break;
        } else {
            unexpected(p, t);
        }
    }

    if (dest != NULL) *dest = t;
    return list;
}

static void do_parse(JadeParser *p){
    Token eof;
    JadeNode *ss;
    JadePosition pos = {1, 1, 1, 1};

    ss = parse_statements(p, TOKEN_EOF, &eof);
    p->toplevel = create_block(p->jade, between(pos, eof.pos), ss);
}

typedef enum {
    PREC_INVALID,
    PREC_LOWEST,
    PREC_LOR,
    PREC_LAND,
    PREC_EQUALITY,
    PREC_COMPARE,
    PREC_ADDITIVE,
    PREC_MULTIVE,
    PREC_PREFIX,
    PREC_CALL,
    PREC_HIGHEST,
} Precedence;

static JadeNode *parse_expression(JadeParser *, Precedence);

static JadeNode *parse_expression_statement(JadeParser *p){
    JadeNode *x;
    Token t;

    x = parse_expression(p, PREC_LOWEST);
    t = must_be_eos(p);
    return create_expression_statement(p->jade, between(x->pos, t.pos), x);
}

static JadeNode *parse_function_literal(JadeParser *, Token);

static JadeNode *parse_def(JadeParser *p, Token kw){
    Token name, t;
    JadeNode *x;

    name = must_be(p, TOKEN_IDENTIFIER);
    t = next_token(p);

    switch (t.type) {
    case TOKEN_NEWLINE:
    case TOKEN_SEMICOLON:
        pushback(p, t);
        x = NULL;
        break;
    case TOKEN_LET:
        x = parse_expression(p, PREC_LOWEST);
        break;
    case TOKEN_LP:
    case TOKEN_LC:
        pushback(p, t);
        x = parse_function_literal(p, t);
        break;
    default:
        unexpected(p, t);
    }

    t = must_be_eos(p);
    return create_def(p->jade, between(kw.pos, t.pos), create_identifier(p->jade, name.pos, name.value), x);
}

#define CONSUME_NEWLINE(p)  (expect((p), TOKEN_NEWLINE, NULL))

static JadeNode *parse_if(JadeParser *, Token);

static JadeNode *parse_if_latter(JadeParser *p){
    Token nl, t;
    int pre_nl_exists;

    pre_nl_exists = expect(p, TOKEN_NEWLINE, &nl);
    t = next_token(p);

    switch (t.type) {
    case TOKEN_ELSIF:
        return parse_if(p, t);
    case TOKEN_ELSE:
        return parse_block(p);
    default:
        pushback(p, t);
        if (pre_nl_exists) pushback(p, nl);
        return NULL;
    }
}

static JadeNode *parse_if(JadeParser *p, Token kw){
    JadeNode *test, *body, *alt;
    Token t;

    test = parse_expression(p, PREC_LOWEST);
    CONSUME_NEWLINE(p);
    body = parse_block(p);
    alt = parse_if_latter(p);
    t = must_be_eos(p);

    return create_if(p->jade, between(kw.pos, t.pos), test, body, alt);
}   

static JadeNode *parse_unless(JadeParser *p, Token kw){
    JadeNode *test, *body = NULL, *alt;
    Token nl, t;
    int nl_exists;

    test = parse_expression(p, PREC_LOWEST);
    CONSUME_NEWLINE(p);
    alt = parse_block(p);

    nl_exists = expect(p, TOKEN_NEWLINE, &nl);
    if (expect(p, TOKEN_ELSE, &t)) {
        body = parse_block(p);
    } else if (nl_exists) {
        pushback(p, nl);
    }
    t = must_be_eos(p);

    return create_if(p->jade, between(kw.pos, t.pos), test, body, alt);
}

static JadeNode *parse_loop(JadeParser *p, Token kw){
    JadeNode *cond, *body;
    Token t;

    cond = parse_expression(p, PREC_LOWEST);
    CONSUME_NEWLINE(p);
    body = parse_block(p);
    t = must_be_eos(p);

    return (kw.type == TOKEN_WHILE ? create_while : create_until)(p->jade, between(kw.pos, t.pos), cond, body);
}

static JadeNode *parse_for_var(JadeParser *p){
    Token t = must_be(p, TOKEN_IDENTIFIER);
    return create_identifier(p->jade, t.pos, t.value);
}

static JadeNode *parse_for(JadeParser *p, Token kw){
    JadeNode *vars, *range, *body;
    Token t;

    vars = parse_comma_list(p, TOKEN_COLON, &t, parse_for_var);
    if (vars == NULL) unexpected(p, t);
    range = parse_expression(p, PREC_LOWEST);
    CONSUME_NEWLINE(p);
    body = parse_block(p);
    t = must_be_eos(p);

    return create_for(p->jade, between(kw.pos, t.pos), vars, range, body);
}

static void parse_rescue(JadeParser *p, JadeTryNodeAttributes *a){
    Token t;
    int nl_exists;

    nl_exists = expect(p, TOKEN_NEWLINE, &t);
    if (!expect(p, TOKEN_RESCUE, NULL)) {
        if (nl_exists) pushback(p, t);
        return;
    }
    must_be(p, TOKEN_COLON);
    t = must_be(p, TOKEN_IDENTIFIER);
    a->variable = create_identifier(p->jade, t.pos, t.value);
    CONSUME_NEWLINE(p);
    a->rescue = parse_block(p);
}

static void parse_ensure(JadeParser *p, JadeTryNodeAttributes *a){
    Token t;
    int nl_exists;

    nl_exists = expect(p, TOKEN_NEWLINE, &t);
    if (!expect(p, TOKEN_ENSURE, NULL)) {
        if (nl_exists) pushback(p, t);
        return;
    }
    a->ensure = parse_block(p);
}

static JadeNode *parse_try(JadeParser *p, Token kw){
    JadeTryNodeAttributes a = {NULL, NULL, NULL, NULL};
    Token t;

    a.body = parse_block(p);
    parse_rescue(p, &a);
    parse_ensure(p, &a);
    t = must_be_eos(p);
    return create_try(p->jade, between(kw.pos, t.pos), a.body, a.variable, a.rescue, a.ensure);
}

static JadeNode *parse_control_transfer(JadeParser *p, Token kw){
    JadeNode *(*fn)(JadeState *, JadePosition, JadeNode *);
    JadeNode *x = NULL;
    Token t;

    switch (kw.type) {
    case TOKEN_BREAK:
        fn = create_break;
        break;
    case TOKEN_CONTINUE:
        fn = create_continue;
        break;
    case TOKEN_RETURN:
        fn = create_return;
        break;
    case TOKEN_THROW:
        fn = create_throw;
        break;
    default:
        assert(0);
    }

    if (!expect_eos(p, &t)) {
        x = parse_expression(p, PREC_LOWEST);
        t = must_be_eos(p);
    }

    return fn(p->jade, between(kw.pos, t.pos), x);
}

static JadeNode *parse_statement(JadeParser *p){
    Token t = next_token(p);
    switch (t.type) {
    case TOKEN_SEMICOLON:
        return NULL;
    case TOKEN_DEF:
        return parse_def(p, t);
    case TOKEN_IF:
        return parse_if(p, t);
    case TOKEN_UNLESS:
        return parse_unless(p, t);
    case TOKEN_WHILE:
        return parse_loop(p, t);
    case TOKEN_UNTIL:
        return parse_loop(p, t);
    case TOKEN_FOR:
        return parse_for(p, t);
    case TOKEN_TRY:
        return parse_try(p, t);
    case TOKEN_BREAK:
    case TOKEN_CONTINUE:
    case TOKEN_RETURN:
    case TOKEN_THROW:
        return parse_control_transfer(p, t);
    default:
        pushback(p, t);
        return parse_expression_statement(p);
    }
}

static JadeNode *parse_identifier(JadeParser *p, Token t){
    return create_identifier(p->jade, t.pos, t.value);
}

static JadeNode *parse_int_literal(JadeParser *p, Token t){
    int v;
    sscanf(t.value, "%d", &v);
    free(t.value);
    return create_int_literal(p->jade, t.pos, v);
}

static JadeNode *parse_float_literal(JadeParser *p, Token t){
    double v;
    sscanf(t.value, "%lf", &v);
    free(t.value);
    return create_float_literal(p->jade, t.pos, v);
}

static JadeNode *parse_string_literal(JadeParser *p, Token t){
     return create_string_literal(p->jade, t.pos, t.value);
}

static JadeNode *parse_boolean_literal(JadeParser *p, Token t){
     return create_boolean_literal(p->jade, t.pos, t.type == TOKEN_TRUE);
}

static JadeNode *parse_nil_literal(JadeParser *p, Token t){
    return create_nil_literal(p->jade, t.pos);
}

static JadeNode *parse_grouped(JadeParser *p, Token t){
    JadeNode *x = parse_expression(p, PREC_LOWEST);
    must_be(p, TOKEN_RP);
    return x;
}

static JadeNode *parse_array_element(JadeParser *p){
    return parse_expression(p, PREC_LOWEST);
}

static JadeNode *parse_array_literal(JadeParser *p, Token lb){
    JadeNode *elems;
    Token t;

    elems = parse_comma_list(p, TOKEN_RB, &t, parse_array_element);
    return create_array_literal(p->jade, between(lb.pos, t.pos), elems);
}

static JadeNode *parse_table_entry(JadeParser *p){
    JadeNode *k, *v;
    k = parse_expression(p, PREC_LOWEST);
    must_be(p, TOKEN_COLON);
    v = parse_expression(p, PREC_LOWEST);
    return create_table_entry(p->jade, between(k->pos, v->pos), k, v);
}

static JadeNode *parse_table_literal(JadeParser *p, Token lc){
    JadeNode *entries;
    Token t;

    entries = parse_comma_list(p, TOKEN_RC, &t, parse_table_entry);
    return create_table_literal(p->jade, between(lc.pos, t.pos), entries);
}

static JadeNode *parse_parameter(JadeParser *p){
    Token t = must_be(p, TOKEN_IDENTIFIER);
    return create_identifier(p->jade, t.pos, t.value);
}

static JadeNode *parse_function_literal(JadeParser *p, Token arrow){
    JadeNode *params = NULL;
    JadeNode *body;

    if (expect(p, TOKEN_LP, NULL)) {
        params = parse_comma_list(p, TOKEN_RP, NULL, parse_parameter);
        expect(p, TOKEN_NEWLINE, NULL);
    }
    
    body = parse_block(p);
    return create_function_literal(p->jade, between(arrow.pos, body->pos), params, body);
}

static JadeNode *parse_match_body(JadeParser *p){
    Token t = next_token(p);
    pushback(p, t);

    switch (t.type) {
    case TOKEN_WHEN:
    case TOKEN_ELSE:
    case TOKEN_RC:
        return NULL;
    case TOKEN_LC:
        return parse_block(p);
    default:
        break;
    }
    return parse_statement(p);
}

static JadeNode *parse_match_pattern_expression(JadeParser *p){
    return parse_expression(p, PREC_LOWEST);
}

static JadeNode *parse_match_clause(JadeParser *p, Token kw){
    JadeNode *xs;
    JadeNode *body;

    xs = parse_comma_list(p, TOKEN_COLON, NULL, parse_match_pattern_expression);
    body = parse_match_body(p);

    return create_match_clause(p->jade, between(kw.pos, body->pos), xs, body);
}

static JadeNode *parse_match(JadeParser *p, Token kw){
    JadeNode *list = NULL;
    JadeNode *operand, *variable = NULL, *otherwise = NULL;
    JadePosition pos;

    operand = parse_expression(p, PREC_LOWEST);
    CONSUME_NEWLINE(p);

    if (expect(p, TOKEN_COLON, NULL)) {
        Token t = must_be(p, TOKEN_IDENTIFIER);
        variable = create_identifier(p->jade, t.pos, t.value);
        CONSUME_NEWLINE(p);
    }

    must_be(p, TOKEN_LC);
    for (;;) {
        Token t = next_token(p);
        switch (t.type) {
        case TOKEN_WHEN:
            list = create_list_entry(p->jade, list, parse_match_clause(p, t));
            break;
        case TOKEN_ELSE:
            must_be(p, TOKEN_COLON);
            otherwise = parse_match_body(p);
            break;
        case TOKEN_RC:
            pos = between(kw.pos, t.pos);
            goto END;
        default:
            unexpected(p, t);
        }
    }

END:
    return create_match(p->jade, pos, operand, variable, list, otherwise);
}

static JadeNode *parse_prefixed(JadeParser *p, Token op){
    JadeNode *x;
    JadeNode *(*f)(JadeState *, JadePosition, JadeNode *);

    switch (op.type) {
    case TOKEN_BANG:
        f = create_not;
        break;
    case TOKEN_SUB:
        f = create_minus;
        break;
    default:
        assert(0);
    }

    x = parse_expression(p, PREC_LOWEST);
    return f(p->jade, between(op.pos, x->pos), x);
}

static Precedence token_type_precedence(TokenType type){
    switch (type) {
    case TOKEN_LOR:
        return PREC_LOR;
    case TOKEN_LAND:
        return PREC_LAND;
    case TOKEN_EQ:
    case TOKEN_NE:
        return PREC_EQUALITY;
    case TOKEN_LT:
    case TOKEN_GT:
    case TOKEN_LE:
    case TOKEN_GE:
        return PREC_COMPARE;
    case TOKEN_ADD:
    case TOKEN_SUB:
        return PREC_ADDITIVE;
    case TOKEN_MUL:
    case TOKEN_DIV:
    case TOKEN_MOD:
        return PREC_MULTIVE;
    case TOKEN_LP:
    case TOKEN_LB:
        return PREC_CALL;
    case TOKEN_LET:
    case TOKEN_ADD_LET:
    case TOKEN_SUB_LET:
    case TOKEN_MUL_LET:
    case TOKEN_DIV_LET:
    case TOKEN_MOD_LET:
        return PREC_HIGHEST;
    default:
        return PREC_INVALID;
    }
}

static JadeNode *parse_infixed(JadeParser *p, JadeNode *left, Token op){
    JadeNodeType type;
    JadeNode *right = parse_expression(p, token_type_precedence(op.type));

    switch (op.type) {
    case TOKEN_LOR:
        type = JADE_LOGICAL_OR_NODE;
        break;
    case TOKEN_LAND:
        type = JADE_LOGICAL_AND_NODE;
        break;
    case TOKEN_EQ:
        type = JADE_EQUAL_NODE;
        break;
    case TOKEN_NE:
        type = JADE_NOT_EQUAL_NODE;
        break;
    case TOKEN_LT:
        type = JADE_LESS_THAN_NODE;
        break;
    case TOKEN_GT:
        type = JADE_GREATER_THAN_NODE;
        break;
    case TOKEN_LE:
        type = JADE_LESS_THAN_EQUAL_NODE;
        break;
    case TOKEN_GE:
        type = JADE_GREATER_THAN_EQUAL_NODE;
        break;
    case TOKEN_ADD:
        type = JADE_ADDITION_NODE;
        break;
    case TOKEN_SUB:
        type = JADE_SUBTRACTION_NODE;
        break;
    case TOKEN_MUL:
        type = JADE_MULTIPLICATION_NODE;
        break;
    case TOKEN_DIV:
        type = JADE_DIVISION_NODE;
        break;
    case TOKEN_MOD:
        type = JADE_MODULO_NODE;
        break;
    default:
        assert(0);
    }

    return create_binary_expression(p->jade, between(left->pos, right->pos), type, left, right);
}

static JadeNode *parse_argument(JadeParser *p){
    return parse_expression(p, PREC_LOWEST);
}

static JadeNode *parse_call(JadeParser *p, JadeNode *fn, Token lp){
    JadeNode *args;
    Token t;

    args = parse_comma_list(p, TOKEN_RP, &t, parse_argument);
    return create_call(p->jade, between(fn->pos, t.pos), fn, args);
}

static JadeNode *parse_fetch(JadeParser *p, JadeNode *left, Token op){
    Token rb;
    JadeNode *k = parse_expression(p, PREC_LOWEST);
    expect(p, TOKEN_NEWLINE, NULL);
    rb = must_be(p, TOKEN_RB);
    return create_fetch(p->jade, between(left->pos, rb.pos), left, k);
}

static JadeNode *create_assign(JadeParser *p, JadeNode *left, JadeNode *right){
    switch (left->type) {
    case JADE_IDENTIFIER_NODE:
        return create_let(p->jade, between(left->pos, right->pos), left, right);
    case JADE_FETCH_NODE:
        return create_store(p->jade, between(left->pos, right->pos), left, right);
    default:
        jade_raise(
            p->jade, "(%d:%d):(%d:%d): invalid lvalue",
            left->pos.start_line, left->pos.start_column,
            left->pos.end_line, left->pos.end_column
        );
    }
    return NULL;
}

static JadeNode *parse_let(JadeParser *p, JadeNode *left, Token op){
    return create_assign(p, left, parse_expression(p, PREC_HIGHEST));
}

static JadeNode *parse_selflet(JadeParser *p, JadeNode *left, Token op){
    JadeNodeType type;
    JadeNode *right;

    switch (op.type) {
    case TOKEN_ADD_LET:
        type = JADE_ADDITION_NODE;
        break;
    case TOKEN_SUB_LET:
        type = JADE_SUBTRACTION_NODE;
        break;
    case TOKEN_MUL_LET:
        type = JADE_MULTIPLICATION_NODE;
        break;
    case TOKEN_DIV_LET:
        type = JADE_DIVISION_NODE;
        break;
    case TOKEN_MOD_LET:
        type = JADE_MODULO_NODE;
        break;
    default:
        assert(0);
    }

    right = parse_expression(p, PREC_HIGHEST);
    return create_assign(p, left, create_binary_expression(p->jade, between(left->pos, right->pos), type, left, right));
}

static JadeNode *parse_expression(JadeParser *p, Precedence prec){
    JadeNode *left;
    Token t = next_token(p);

    switch (t.type) {
    case TOKEN_IDENTIFIER:
        left = parse_identifier(p, t);
        break;
    case TOKEN_INT_LITERAL:
        left = parse_int_literal(p, t);
        break;
    case TOKEN_FLOAT_LITERAL:
        left = parse_float_literal(p, t);
        break;
    case TOKEN_STRING_LITERAL:
        left = parse_string_literal(p, t);
        break;
    case TOKEN_TRUE:
    case TOKEN_FALSE:
        left = parse_boolean_literal(p, t);
        break;
    case TOKEN_NIL:
        left = parse_nil_literal(p, t);
        break;
    case TOKEN_LP:
        left = parse_grouped(p, t);
        break;
    case TOKEN_LB:
        left = parse_array_literal(p, t);
        break;
    case TOKEN_LC:
        left = parse_table_literal(p, t);
        break;
    case TOKEN_ARROW:
        left = parse_function_literal(p, t);
        break;
    case TOKEN_MATCH:
        left = parse_match(p, t);
        break;
    case TOKEN_SUB:
    case TOKEN_BANG:
        left = parse_prefixed(p, t);
        break;
    default:
        parse_error(p, t, "invalid prefix of expression -");
    }

    for (;;) {
        Precedence next;
        t = next_token(p);
        next = token_type_precedence(t.type);

        if (next == PREC_INVALID || (prec != PREC_HIGHEST && prec >= next)) {
            pushback(p, t);
            break;
        }

        switch (t.type) {
        case TOKEN_LOR:
        case TOKEN_LAND:
        case TOKEN_EQ:
        case TOKEN_NE:
        case TOKEN_LT:
        case TOKEN_GT:
        case TOKEN_LE:
        case TOKEN_GE:
        case TOKEN_ADD:
        case TOKEN_SUB:
        case TOKEN_MUL:
        case TOKEN_DIV:
        case TOKEN_MOD:
            left = parse_infixed(p, left, t);
            break;
        case TOKEN_LP:
            left = parse_call(p, left, t);
            break;
        case TOKEN_LB:
            left = parse_fetch(p, left, t);
            break;
        case TOKEN_LET:
            left = parse_let(p, left, t);
            break;
        case TOKEN_ADD_LET:
        case TOKEN_SUB_LET:
        case TOKEN_MUL_LET:
        case TOKEN_DIV_LET:
        case TOKEN_MOD_LET:
            left = parse_selflet(p, left, t);
            break;
        default:
            assert(0);
        }
    }

    return left;
}
