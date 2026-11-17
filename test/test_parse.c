#include "../Unity/src/unity.h"
#include <jade.h>
#include <jade/parser.h>
#include "../src/state.h"

typedef struct {
    JadeState *jade;
    JadeParser *p;
} Fixture;

Fixture g_fixture;

void setUp(void) {
    g_fixture.jade = jade_new();
    g_fixture.p = jade_parser_new(g_fixture.jade);
}

void tearDown(void) {
    jade_parser_dispose(g_fixture.p);
    jade_dispose(g_fixture.jade);
}

static JadeNode *extract_1statement(const char *src){
    JadeNodeList *pos;

    jade_parser_parse_string(g_fixture.p, "test.jd", 1, src);

    TEST_ASSERT_NOT_NULL(g_fixture.jade->toplevel);
    TEST_ASSERT_EQUAL(JADE_BLOCK_NODE, g_fixture.jade->toplevel->type);

    pos = g_fixture.jade->toplevel->u.block.statements;
    TEST_ASSERT_NOT_NULL(pos);
    TEST_ASSERT_NULL(pos->next);
    TEST_ASSERT_NOT_NULL(pos->node);
    return pos->node;
}

void test_def(void){
    JadeNode *n = extract_1statement("def hoge = 666\n");

    TEST_ASSERT_EQUAL(JADE_DEF_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_IDENTIFIER_NODE, n->u.def.name->type);
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.def.init->type);
}

void test_def_noval(void){
    JadeNode *n = extract_1statement("def hoge\n");

    TEST_ASSERT_EQUAL(JADE_DEF_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_IDENTIFIER_NODE, n->u.def.name->type);
    TEST_ASSERT_NULL(n->u.def.init);
}

void test_def_function(void){
    JadeNode *n = extract_1statement(
        "def hoge(x, y){\n"
        "  x + y\n"
        "}\n"
    );

    TEST_ASSERT_EQUAL(JADE_DEF_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_IDENTIFIER_NODE, n->u.def.name->type);
    TEST_ASSERT_EQUAL(JADE_FUNCTION_LITERAL_NODE, n->u.def.init->type);
}

void test_if(void){
    JadeNode *n = extract_1statement(
        "if 1 {\n"
        "  2\n"
        "} else {\n"
        "  3\n"
        "}\n"
    );

    TEST_ASSERT_EQUAL(JADE_IF_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.if_.test->type);
    TEST_ASSERT_EQUAL(JADE_BLOCK_NODE, n->u.if_.body->type);
    TEST_ASSERT_EQUAL(JADE_BLOCK_NODE, n->u.if_.alt->type);
}

void test_if_noalt(void){
    JadeNode *n = extract_1statement(
        "if 1 {\n"
        "  2\n"
        "}\n"
    );

    TEST_ASSERT_EQUAL(JADE_IF_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.if_.test->type);
    TEST_ASSERT_EQUAL(JADE_BLOCK_NODE, n->u.if_.body->type);
    TEST_ASSERT_NULL(n->u.if_.alt);
}

void test_if_nonl(void){
    JadeNode *n = extract_1statement(
        "if i > 3 { break }\n"
    );

    TEST_ASSERT_EQUAL(JADE_IF_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_GREATER_THAN_NODE, n->u.if_.test->type);
    TEST_ASSERT_EQUAL(JADE_BLOCK_NODE, n->u.if_.body->type);
    TEST_ASSERT_NULL(n->u.if_.alt);
}

void test_break(void){
    JadeNode *n = extract_1statement("break 1\n");
    TEST_ASSERT_EQUAL(JADE_BREAK_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.control_transfer.result->type);
    TEST_ASSERT_EQUAL_INT(1, n->u.control_transfer.result->u.int_literal.value);
}

void test_break_noexpr(void){
    JadeNode *n = extract_1statement("break\n");
    TEST_ASSERT_EQUAL(JADE_BREAK_NODE, n->type);
    TEST_ASSERT_NULL(n->u.control_transfer.result);
}

void test_continue(void){
    JadeNode *n = extract_1statement("continue 1\n");
    TEST_ASSERT_EQUAL(JADE_CONTINUE_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.control_transfer.result->type);
    TEST_ASSERT_EQUAL_INT(1, n->u.control_transfer.result->u.int_literal.value);
}

void test_continue_noexpr(void){
    JadeNode *n = extract_1statement("continue\n");
    TEST_ASSERT_EQUAL(JADE_CONTINUE_NODE, n->type);
    TEST_ASSERT_NULL(n->u.control_transfer.result);
}

void test_return(void){
    JadeNode *n = extract_1statement("return 1\n");
    TEST_ASSERT_EQUAL(JADE_RETURN_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.control_transfer.result->type);
    TEST_ASSERT_EQUAL_INT(1, n->u.control_transfer.result->u.int_literal.value);
}

void test_return_noexpr(void){
    JadeNode *n = extract_1statement("return\n");
    TEST_ASSERT_EQUAL(JADE_RETURN_NODE, n->type);
    TEST_ASSERT_NULL(n->u.control_transfer.result);
}

void test_throw(void){
    JadeNode *n = extract_1statement("throw 1\n");
    TEST_ASSERT_EQUAL(JADE_THROW_NODE, n->type);
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.control_transfer.result->type);
    TEST_ASSERT_EQUAL_INT(1, n->u.control_transfer.result->u.int_literal.value);
}

void test_throw_noexpr(void){
    JadeNode *n = extract_1statement("throw\n");
    TEST_ASSERT_EQUAL(JADE_THROW_NODE, n->type);
    TEST_ASSERT_NULL(n->u.control_transfer.result);
}

static JadeNode *extract_1expression(const char *src){
    JadeNode *s = extract_1statement(src);
    TEST_ASSERT_EQUAL(JADE_EXPRESSION_STATEMENT_NODE, s->type);
    return s->u.expression_statement.expression;
}

void test_parse_identifier(void){
    JadeNode *n = extract_1expression("hoge\n");

    TEST_ASSERT_EQUAL(JADE_IDENTIFIER_NODE, n->type);
    TEST_ASSERT_EQUAL_STRING("hoge", n->u.identifier.name);
}

void test_parse_int_literal(void){
    JadeNode *n = extract_1expression("666");

    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->type);
    TEST_ASSERT_EQUAL_INT(666, n->u.int_literal.value);
}

void test_parse_float_literal(void){
    JadeNode *n = extract_1expression("12.3");

    TEST_ASSERT_EQUAL(JADE_FLOAT_LITERAL_NODE, n->type);
    TEST_ASSERT_EQUAL(12.3, n->u.float_literal.value);
}

void test_parse_string_literal(void){
    JadeNode *n = extract_1expression("\"Hello!\"");

    TEST_ASSERT_EQUAL(JADE_STRING_LITERAL_NODE, n->type);
    TEST_ASSERT_EQUAL_STRING("Hello!", n->u.string_literal.value);
}

void test_parse_true_literal(void){
    JadeNode *n = extract_1expression("true");

    TEST_ASSERT_EQUAL(JADE_BOOLEAN_LITERAL_NODE, n->type);
    TEST_ASSERT_TRUE(n->u.boolean_literal.value);
}

void test_parse_false_literal(void){
    JadeNode *n = extract_1expression("false");

    TEST_ASSERT_EQUAL(JADE_BOOLEAN_LITERAL_NODE, n->type);
    TEST_ASSERT_FALSE(n->u.boolean_literal.value);
}

void test_parse_nil_literal(void){
    JadeNode *n = extract_1expression("nil");
    TEST_ASSERT_EQUAL(JADE_NIL_LITERAL_NODE, n->type);
}

void test_parse_grouped(void){
    JadeNode *n = extract_1expression("(nil)");
    TEST_ASSERT_EQUAL(JADE_NIL_LITERAL_NODE, n->type);
}

void test_parse_array_literal(void){
    JadeNodeList *pos;
    JadeNode *n = extract_1expression("[1, 2, 3]");
    TEST_ASSERT_EQUAL(JADE_ARRAY_LITERAL_NODE, n->type);

    pos = n->u.array_literal.elements;
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, pos->node->type);
    TEST_ASSERT_EQUAL_INT(1, pos->node->u.int_literal.value);

    pos = pos->next;
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, pos->node->type);
    TEST_ASSERT_EQUAL_INT(2, pos->node->u.int_literal.value);

    pos = pos->next;
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, pos->node->type);
    TEST_ASSERT_EQUAL_INT(3, pos->node->u.int_literal.value);

    TEST_ASSERT_NULL(pos->next);
}

void test_parse_array_literal_empty(void){
    JadeNode *n = extract_1expression("[]");
    TEST_ASSERT_EQUAL(JADE_ARRAY_LITERAL_NODE, n->type);
    TEST_ASSERT_NULL(n->u.array_literal.elements);
}

void test_parse_table_literal(void){
    JadeNodeList *pos;

    JadeNode *n = extract_1expression("{\"x\": 1, \"y\": 2, \"z\": 3}");
    TEST_ASSERT_EQUAL(JADE_TABLE_LITERAL_NODE, n->type);

    pos = n->u.table_literal.entries;
    TEST_ASSERT_EQUAL(JADE_TABLE_ENTRY_NODE, pos->node->type);

    pos = pos->next;
    TEST_ASSERT_EQUAL(JADE_TABLE_ENTRY_NODE, pos->node->type);

    pos = pos->next;
    TEST_ASSERT_EQUAL(JADE_TABLE_ENTRY_NODE, pos->node->type);

    TEST_ASSERT_NULL(pos->next);
}

void test_parse_table_literal_empty(void){
    JadeNode *n = extract_1expression("{}");
    TEST_ASSERT_EQUAL(JADE_TABLE_LITERAL_NODE, n->type);
    TEST_ASSERT_NULL(n->u.table_literal.entries);
}

void test_parse_function_literal(void){
    JadeNode *n = extract_1expression("->(x, y){}");
    TEST_ASSERT_EQUAL(JADE_FUNCTION_LITERAL_NODE, n->type);
}

void test_parse_not(void){
    JadeNode *n = extract_1expression("!true");
    TEST_ASSERT_EQUAL(JADE_NOT_NODE, n->type);

    n = n->u.unary_expression.operand;
    TEST_ASSERT_EQUAL(JADE_BOOLEAN_LITERAL_NODE, n->type);
    TEST_ASSERT_TRUE(n->u.int_literal.value);
}

void test_parse_minus(void){
    JadeNode *n = extract_1expression("-666");
    TEST_ASSERT_EQUAL(JADE_MINUS_NODE, n->type);

    n = n->u.unary_expression.operand;
    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->type);
    TEST_ASSERT_EQUAL_INT(666, n->u.int_literal.value);
}

void test_parse_addition(void){
    JadeNode *n = extract_1expression("1 + 2");
    TEST_ASSERT_EQUAL(JADE_ADDITION_NODE, n->type);
}

void test_parse_complex_expr(void){
    JadeNode *n = extract_1expression("1 + 2 * 3");
    TEST_ASSERT_EQUAL(JADE_ADDITION_NODE, n->type);

    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.binary_expression.left->type);
    TEST_ASSERT_EQUAL_INT(1, n->u.binary_expression.left->u.int_literal.value);

    n = n->u.binary_expression.right;
    TEST_ASSERT_EQUAL(JADE_MULTIPLICATION_NODE, n->type);
}

void test_call(void){
    JadeNode *n = extract_1expression("f(1, 2)");
    TEST_ASSERT_EQUAL(JADE_CALL_NODE, n->type);
}

void test_fetch(void){
    JadeNode *n = extract_1expression("a[1]");
    TEST_ASSERT_EQUAL(JADE_FETCH_NODE, n->type);

    TEST_ASSERT_EQUAL(JADE_IDENTIFIER_NODE, n->u.fetch.container->type);
    TEST_ASSERT_EQUAL_STRING("a", n->u.fetch.container->u.string_literal.value);

    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.fetch.key->type);
    TEST_ASSERT_EQUAL_INT(1, n->u.fetch.key->u.int_literal.value);
}

void test_let(void){
    JadeNode *n = extract_1expression("x = 1");
    TEST_ASSERT_EQUAL(JADE_LET_NODE, n->type);

    TEST_ASSERT_EQUAL(JADE_IDENTIFIER_NODE, n->u.let.name->type);
    TEST_ASSERT_EQUAL_STRING("x", n->u.let.name->u.identifier.name);

    TEST_ASSERT_EQUAL(JADE_INT_LITERAL_NODE, n->u.let.operand->type);
    TEST_ASSERT_EQUAL_INT(1, n->u.let.operand->u.int_literal.value);
}

void test_store(void){
    JadeNode *n = extract_1expression("a[0] = 1");
    TEST_ASSERT_EQUAL(JADE_STORE_NODE, n->type);
}

void test_match(void){
    JadeNode *n = extract_1expression(
        "match 666 : it {\n"
        "  when it > 100: print(it)\n"
        "}\n"
    );
    TEST_ASSERT_EQUAL(JADE_MATCH_NODE, n->type);
}
