#include "../Unity/src/unity.h"
#include <jade.h>
#include <jade/parser.h>
#include "../src/parser.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    JadeState *jade;
    JadeParser *p;
    Token t;
} Fixture;

Fixture g_fixture;

void setUp(void) {
    g_fixture.jade = jade_new();
    g_fixture.p = jade_parser_new(g_fixture.jade);
    g_fixture.t.value = NULL;
}

void tearDown(void) {
    free(g_fixture.t.value);
    jade_parser_dispose(g_fixture.p);
    jade_dispose(g_fixture.jade);
}

static Token lex_token(const char *src) {
    set_string_source(g_fixture.p, "test.jd", 1, src);
    g_fixture.t = next_token(g_fixture.p);
    return g_fixture.t;
}

#define NEXT_TOKEN()    (next_token(g_fixture.p))

void test_lex_addition(void){
    Token t = lex_token("1 + 2");
    TEST_ASSERT_EQUAL(TOKEN_INT_LITERAL, t.type);
    TEST_ASSERT_EQUAL_STRING("1", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(1, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(1, t.pos.end_column);
    free(t.value);

    t = NEXT_TOKEN();
    TEST_ASSERT_EQUAL(TOKEN_ADD, t.type);
    TEST_ASSERT_NULL(t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(3, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(3, t.pos.end_column);

    t = NEXT_TOKEN();
    TEST_ASSERT_EQUAL(TOKEN_INT_LITERAL, t.type);
    TEST_ASSERT_EQUAL_STRING("2", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(5, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(5, t.pos.end_column);
}

void test_lex_array_literal(void){
    Token t = lex_token("[1, 2, 3]");
    TEST_ASSERT_EQUAL(TOKEN_LB, t.type);
    TEST_ASSERT_NULL(t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(1, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(1, t.pos.end_column);

    t = NEXT_TOKEN();
    TEST_ASSERT_EQUAL(TOKEN_INT_LITERAL, t.type);
    TEST_ASSERT_EQUAL_STRING("1", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(2, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(2, t.pos.end_column);
    free(t.value);

    t = NEXT_TOKEN();
    TEST_ASSERT_EQUAL(TOKEN_COMMA, t.type);
    TEST_ASSERT_NULL(t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(3, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(3, t.pos.end_column);


    t = NEXT_TOKEN();
    TEST_ASSERT_EQUAL(TOKEN_INT_LITERAL, t.type);
    TEST_ASSERT_EQUAL_STRING("2", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(5, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(5, t.pos.end_column);
}

void test_lex_identifier(void) {
    Token t = lex_token("hoge");
    TEST_ASSERT_EQUAL(TOKEN_IDENTIFIER, t.type);
    TEST_ASSERT_EQUAL_STRING("hoge", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(1, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(4, t.pos.end_column);

    t = next_token(g_fixture.p);
    TEST_ASSERT_EQUAL(TOKEN_NEWLINE, t.type);
}

void test_lex_int(void) {
    Token t = lex_token("123");
    TEST_ASSERT_EQUAL(TOKEN_INT_LITERAL, t.type);
    TEST_ASSERT_EQUAL_STRING("123", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(1, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(3, t.pos.end_column);
}

void test_lex_float(void) {
    Token t = lex_token("12.3");
    TEST_ASSERT_EQUAL(TOKEN_FLOAT_LITERAL, t.type);
    TEST_ASSERT_EQUAL_STRING("12.3", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(1, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(4, t.pos.end_column);
}

void test_lex_string(void) {
    Token t = lex_token("\"Hello\"");
    TEST_ASSERT_EQUAL(TOKEN_STRING_LITERAL, t.type);
    TEST_ASSERT_EQUAL_STRING("Hello", t.value);
    TEST_ASSERT_EQUAL(1, t.pos.start_line);
    TEST_ASSERT_EQUAL(1, t.pos.start_column);
    TEST_ASSERT_EQUAL(1, t.pos.end_line);
    TEST_ASSERT_EQUAL(7, t.pos.end_column);
}


#define DO_ST_TEST(_src, _type)   do {                  \
    Token t = lex_token(_src);                          \
    TEST_ASSERT_EQUAL(_type, t.type);                   \
    TEST_ASSERT_NULL(t.value);                          \
    TEST_ASSERT_EQUAL(1, t.pos.start_line);             \
    TEST_ASSERT_EQUAL(1, t.pos.start_column);           \
    TEST_ASSERT_EQUAL(1, t.pos.end_line);               \
    TEST_ASSERT_EQUAL(strlen(_src), t.pos.end_column);  \
} while(0)

void test_lex_if(void) { DO_ST_TEST("if", TOKEN_IF); }
void test_lex_elsif(void) { DO_ST_TEST("elsif", TOKEN_ELSIF); }
void test_lex_else(void){ DO_ST_TEST("else", TOKEN_ELSE); }
void test_lex_unless(void){ DO_ST_TEST("unless", TOKEN_UNLESS); }
void test_lex_while(void){ DO_ST_TEST("while", TOKEN_WHILE); }
void test_lex_until(void){ DO_ST_TEST("until", TOKEN_UNTIL); }
void test_lex_break(void){ DO_ST_TEST("break", TOKEN_BREAK); }
void test_lex_continue(void){ DO_ST_TEST("continue", TOKEN_CONTINUE); }
void test_lex_for(void){ DO_ST_TEST("for", TOKEN_FOR); }
void test_lex_def(void){ DO_ST_TEST("def", TOKEN_DEF); }
void test_lex_return(void){ DO_ST_TEST("return", TOKEN_RETURN); }
void test_lex_try(void){ DO_ST_TEST("try", TOKEN_TRY); }
void test_lex_rescue(void){ DO_ST_TEST("rescue", TOKEN_RESCUE); }
void test_lex_ensure(void){ DO_ST_TEST("ensure", TOKEN_ENSURE); }
void test_lex_throw(void){ DO_ST_TEST("throw", TOKEN_THROW); }
void test_lex_match(void){ DO_ST_TEST("match", TOKEN_MATCH); }
void test_lex_when(void){ DO_ST_TEST("when", TOKEN_WHEN); }
void test_lex_true(void){ DO_ST_TEST("true", TOKEN_TRUE); }
void test_lex_false(void){ DO_ST_TEST("false", TOKEN_FALSE); }
void test_lex_nil(void){ DO_ST_TEST("nil", TOKEN_NIL); }

void test_lex_lp(void){ DO_ST_TEST("(", TOKEN_LP); }
void test_lex_rp(void){ DO_ST_TEST(")", TOKEN_RP); }
void test_lex_lb(void){ DO_ST_TEST("[", TOKEN_LB); }
void test_lex_rb(void){ DO_ST_TEST("]", TOKEN_RB); }
void test_lex_lc(void){ DO_ST_TEST("{", TOKEN_LC); }
void test_lex_rc(void){ DO_ST_TEST("}", TOKEN_RC); }
void test_lex_eq(void){ DO_ST_TEST("==", TOKEN_EQ); }
void test_lex_ne(void){ DO_ST_TEST("!=", TOKEN_NE); }
void test_lex_gt(void){ DO_ST_TEST(">", TOKEN_GT); }
void test_lex_ge(void){ DO_ST_TEST(">=", TOKEN_GE); }
void test_lex_lt(void){ DO_ST_TEST("<", TOKEN_LT); }
void test_lex_le(void){ DO_ST_TEST("<=", TOKEN_LE); }
void test_lex_let(void){ DO_ST_TEST("=", TOKEN_LET); }
void test_lex_comma(void){ DO_ST_TEST(",", TOKEN_COMMA); }
void test_lex_colon(void){ DO_ST_TEST(":", TOKEN_COLON); }
void test_lex_semicolon(void){ DO_ST_TEST(";", TOKEN_SEMICOLON); }
void test_lex_arrow(void){ DO_ST_TEST("->", TOKEN_ARROW); }
void test_lex_bang(void){ DO_ST_TEST("!", TOKEN_BANG); }
void test_lex_land(void){ DO_ST_TEST("&&", TOKEN_LAND); }
void test_lex_lor(void){ DO_ST_TEST("||", TOKEN_LOR); }
void test_lex_add(void){ DO_ST_TEST("+", TOKEN_ADD); }
void test_lex_sub(void){ DO_ST_TEST("-", TOKEN_SUB); }
void test_lex_mul(void){ DO_ST_TEST("*", TOKEN_MUL); }
void test_lex_div(void){ DO_ST_TEST("/", TOKEN_DIV); }
void test_lex_mod(void){ DO_ST_TEST("%", TOKEN_MOD); }
void test_lex_add_let(void){ DO_ST_TEST("+=", TOKEN_ADD_LET); }
void test_lex_sub_let(void){ DO_ST_TEST("-=", TOKEN_SUB_LET); }
void test_lex_mul_let(void){ DO_ST_TEST("*=", TOKEN_MUL_LET); }
void test_lex_div_let(void){ DO_ST_TEST("/=", TOKEN_DIV_LET); }
void test_lex_mod_let(void){ DO_ST_TEST("%=", TOKEN_MOD_LET); }
