#include <jade.h>
#include <jade/ast.h>
#include "./util.h"
#include "./node.h"
#include "./state.h"
#include "./vector_using.h"
#include <stdlib.h>
#include <assert.h>

static JadeNode *create(JadeState *jade, JadeNodeType type, JadePosition pos){
    JadeNode *n = jmalloc(jade, sizeof(JadeNode));
    n->type = type;
    n->pos = pos;

    vector_push(jade, jade->nodes, JadeNode*, n);
    return n;
}

JadeNode *create_list_entry(JadeState *jade, JadeNode *list, JadeNode *node){
    JadeNode *pos;
    JadeNode *n = create(jade, JADE_LIST_NODE, node->pos);

    n->u.list.next = NULL;
    n->u.list.node = node;
    if (list == NULL) return n;

    for (pos = list; pos->u.list.next != NULL; pos = pos->u.list.next)
        ;
    
    pos->u.list.next = n;
    return list;
}

JadeNode *create_block(JadeState *jade, JadePosition pos, JadeNode *statements){
    JadeNode *n = create(jade, JADE_BLOCK_NODE, pos);
    n->u.block.statements = statements;
    return n;
}

JadeNode *create_def(JadeState *jade, JadePosition pos, JadeNode *name, JadeNode *init){
    JadeNode *n = create(jade, JADE_DEF_NODE, pos);
    n->u.def.name = name;
    n->u.def.init = init;
    return n;
}

JadeNode *create_if(JadeState *jade, JadePosition pos, JadeNode *test, JadeNode *body, JadeNode *alt){
    JadeNode *n = create(jade, JADE_IF_NODE, pos);
    n->u.if_.test = test;
    n->u.if_.body = body;
    n->u.if_.alt = alt;
    return n;
}

JadeNode *create_while(JadeState *jade, JadePosition pos, JadeNode *cond, JadeNode *body){
    JadeNode *n = create(jade, JADE_WHILE_NODE, pos);
    n->u.loop.cond = cond;
    n->u.loop.body = body;
    return n;
}

JadeNode *create_until(JadeState *jade, JadePosition pos, JadeNode *cond, JadeNode *body){
    JadeNode *n = create(jade, JADE_UNTIL_NODE, pos);
    n->u.loop.cond = cond;
    n->u.loop.body = body;
    return n;
}

JadeNode *create_for(JadeState *jade, JadePosition pos, JadeNode *vars, JadeNode *range, JadeNode *body){
    JadeNode *n = create(jade, JADE_FOR_NODE, pos);
    n->u.for_.variables = vars;
    n->u.for_.range = range;
    n->u.for_.body = body;
    return n;
}

JadeNode *create_try(JadeState *jade, JadePosition pos, JadeNode *body, JadeNode *var, JadeNode *rescue, JadeNode *ensure){
    JadeNode *n = create(jade, JADE_TRY_NODE, pos);
    n->u.try.body = body;
    n->u.try.variable = var;
    n->u.try.rescue = rescue;
    n->u.try.ensure = ensure;
    return n;
}

JadeNode *create_break(JadeState *jade, JadePosition pos, JadeNode *result){
    JadeNode *n = create(jade, JADE_BREAK_NODE, pos);
    n->u.control_transfer.result = result;
    return n;
}

JadeNode *create_continue(JadeState *jade, JadePosition pos, JadeNode *result){
    JadeNode *n = create(jade, JADE_CONTINUE_NODE, pos);
    n->u.control_transfer.result = result;
    return n;
}

JadeNode *create_return(JadeState *jade, JadePosition pos, JadeNode *result){
    JadeNode *n = create(jade, JADE_RETURN_NODE, pos);
    n->u.control_transfer.result = result;
    return n;
}

JadeNode *create_throw(JadeState *jade, JadePosition pos, JadeNode *result){
    JadeNode *n = create(jade, JADE_THROW_NODE, pos);
    n->u.control_transfer.result = result;
    return n;
}

JadeNode *create_expression_statement(JadeState *jade, JadePosition pos, JadeNode *expr){
    JadeNode *n = create(jade, JADE_EXPRESSION_STATEMENT_NODE, pos);
    n->u.expression_statement.expression = expr;
    return n;
}

JadeNode *create_identifier(JadeState *jade, JadePosition pos, char *name){
    JadeNode *n = create(jade, JADE_IDENTIFIER_NODE, pos);
    n->u.identifier.name = name;
    return n;
}

JadeNode *create_int_literal(JadeState *jade, JadePosition pos, int val){
    JadeNode *n = create(jade, JADE_INT_LITERAL_NODE, pos);
    n->u.int_literal.value = val;
    return n;
}

JadeNode *create_float_literal(JadeState *jade, JadePosition pos, double val){
    JadeNode *n = create(jade, JADE_FLOAT_LITERAL_NODE, pos);
    n->u.float_literal.value = val;
    return n;
}

JadeNode *create_string_literal(JadeState *jade, JadePosition pos, char *val){
    JadeNode *n = create(jade, JADE_STRING_LITERAL_NODE, pos);
    n->u.string_literal.value = val;
    return n;
}

JadeNode *create_boolean_literal(JadeState *jade, JadePosition pos, int val){
    JadeNode *n = create(jade, JADE_BOOLEAN_LITERAL_NODE, pos);
    n->u.boolean_literal.value = val;
    return n;
}

JadeNode *create_nil_literal(JadeState *jade, JadePosition pos){
    return create(jade, JADE_NIL_LITERAL_NODE, pos);
}

JadeNode *create_array_literal(JadeState *jade, JadePosition pos, JadeNode *elements){
    JadeNode *n = create(jade, JADE_ARRAY_LITERAL_NODE, pos);
    n->u.array_literal.elements = elements;
    return n;
}

JadeNode *create_table_literal(JadeState *jade, JadePosition pos, JadeNode *entries){
    JadeNode *n = create(jade, JADE_TABLE_LITERAL_NODE, pos);
    n->u.table_literal.entries = entries;
    return n;
}

JadeNode *create_table_entry(JadeState *jade, JadePosition pos, JadeNode *key, JadeNode *val){
    JadeNode *n = create(jade, JADE_TABLE_ENTRY_NODE, pos);
    n->u.table_entry.key = key;
    n->u.table_entry.value = val;
    return n;
}

JadeNode *create_function_literal(JadeState *jade, JadePosition pos, JadeNode *params, JadeNode *body){
    JadeNode *n = create(jade, JADE_FUNCTION_LITERAL_NODE, pos);
    n->u.function_literal.parameters = params;
    n->u.function_literal.body = body;
    return n;
}

JadeNode *create_not(JadeState *jade, JadePosition pos, JadeNode *operand){
    JadeNode *n = create(jade, JADE_NOT_NODE, pos);
    n->u.unary_expression.operand = operand;
    return n;
}

JadeNode *create_minus(JadeState *jade, JadePosition pos, JadeNode *operand){
    JadeNode *n = create(jade, JADE_MINUS_NODE, pos);
    n->u.unary_expression.operand = operand;
    return n;
}

JadeNode *create_binary_expression(JadeState *jade, JadePosition pos, JadeNodeType type, JadeNode *left, JadeNode *right){
    JadeNode *n = create(jade, type, pos);
    n->u.binary_expression.left = left;
    n->u.binary_expression.right = right;
    return n;
}

JadeNode *create_let(JadeState *jade, JadePosition pos, JadeNode *name, JadeNode *val){
    JadeNode *n = create(jade, JADE_LET_NODE, pos);
    n->u.let.name = name;
    n->u.let.operand = val;
    return n;
}

JadeNode *create_call(JadeState *jade, JadePosition pos, JadeNode *fn, JadeNode *args){
    JadeNode *n = create(jade, JADE_CALL_NODE, pos);
    n->u.call.function = fn;
    n->u.call.arguments = args;
    return n;
}

JadeNode *create_fetch(JadeState *jade, JadePosition pos, JadeNode *container, JadeNode *key){
    JadeNode *n = create(jade, JADE_FETCH_NODE, pos);
    n->u.fetch.container = container;
    n->u.fetch.key = key;
    return n;
}

JadeNode *create_store(JadeState *jade, JadePosition pos, JadeNode *fetch, JadeNode *val){
    JadeNode *n = create(jade, JADE_STORE_NODE, pos);
    n->u.store.fetch = fetch;
    n->u.store.value = val;
    return n;
}

JadeNode *create_match(
    JadeState *jade,
    JadePosition pos,
    JadeNode *operand,
    JadeNode *variable,
    JadeNode *clauses,
    JadeNode *otherwise
){
    JadeNode *n = create(jade, JADE_MATCH_NODE, pos);
    n->u.match.operand = operand;
    n->u.match.variable = variable;
    n->u.match.clauses = clauses;
    n->u.match.otherwise = otherwise;
    return n;
}

JadeNode *create_match_clause(JadeState *jade, JadePosition pos, JadeNode *pattern, JadeNode *body){
    JadeNode *n = create(jade, JADE_MATCH_CLAUSE_NODE, pos);
    n->u.match_clause.pattern = pattern;
    n->u.match_clause.body = body;
    return n;
}

void dispose_node(JadeNode *n){
    if (n == NULL) return;
    if (n->type == JADE_STRING_LITERAL_NODE) free(n->u.string_literal.value);
    free(n);
}

static void dump_nodelist(JadeNode *list, FILE *fp, int lv){
    JadeNode *pos;
    for (pos = list; pos != NULL; pos = pos->u.list.next) {
        dump_node(pos->u.list.node, fp, lv);
    }
}

#define INDENT(n_, fp_) do {                            \
    int i;                                              \
    for (i = 0; i < (n_); i++) fprintf((fp_), "  ");    \
} while(0);

#define DUMP_CAPTION(n_, name_, fp_)    (fprintf((fp_), "%s:(%d:%d):(%d:%d):", (name_), \
    (n_)->pos.start_line, (n_)->pos.start_column,   \
    (n_)->pos.end_line, (n_)->pos.end_column))

static void dump_control_transfer(JadeNode *n, FILE *fp, int lv){
    INDENT(lv, fp);
    switch (n->type) {
    case JADE_BREAK_NODE:
        DUMP_CAPTION(n, "BREAK", fp);
        break;
    case JADE_CONTINUE_NODE:
        DUMP_CAPTION(n, "CONTINUE", fp);
        break;
    case JADE_RETURN_NODE:
        DUMP_CAPTION(n, "RETURN", fp);
        break;
    case JADE_THROW_NODE:
        DUMP_CAPTION(n, "THROW", fp);
        break;
    default:
        assert(0);
    }
    fputc('\n', fp);
    if (n->u.control_transfer.result == NULL) {
        INDENT(lv + 1, fp);
        fputs("(NULL)\n", fp);
    } else {
        dump_node(n->u.control_transfer.result, fp, lv + 1);
    }
}

static void dump_binary_expression(JadeNode *n, FILE *fp, int lv){
    INDENT(lv, fp);
    switch (n->type) {
    case JADE_LOGICAL_OR_NODE:
        DUMP_CAPTION(n, "LOGICAL_OR", fp);
        break;
    case JADE_LOGICAL_AND_NODE:
        DUMP_CAPTION(n, "LOGICAL_AND", fp);
        break;
    case JADE_EQUAL_NODE:
        DUMP_CAPTION(n, "EQUAL", fp);
        break;
    case JADE_NOT_EQUAL_NODE:
        DUMP_CAPTION(n, "NOT_EQUAL", fp);
        break;
    case JADE_LESS_THAN_NODE:
        DUMP_CAPTION(n, "LESS_THAN", fp);
        break;
    case JADE_GREATER_THAN_NODE:
        DUMP_CAPTION(n, "GREATER_THAN", fp);
        break;
    case JADE_LESS_THAN_EQUAL_NODE:
        DUMP_CAPTION(n, "LESS_THAN_EQUAL", fp);
        break;
    case JADE_GREATER_THAN_EQUAL_NODE:
        DUMP_CAPTION(n, "GREATER_THAN_EQUAL", fp);
        break;
    case JADE_ADDITION_NODE:
        DUMP_CAPTION(n, "ADDITION", fp);
        break;
    case JADE_SUBTRACTION_NODE:
        DUMP_CAPTION(n, "SUBTRACTION", fp);
        break;
    case JADE_MULTIPLICATION_NODE:
        DUMP_CAPTION(n, "MULTIPLICATION", fp);
        break;
    case JADE_DIVISION_NODE:
        DUMP_CAPTION(n, "DIVISION", fp);
        break;
    case JADE_MODULO_NODE:
        DUMP_CAPTION(n, "MODULO", fp);
        break;
    default:
        assert(0);
    }
    fputc('\n', fp);
    INDENT(lv + 1, fp);
    fputs("@LEFT\n", fp);
    dump_node(n->u.binary_expression.left, fp, lv + 1);
    INDENT(lv + 1, fp);
    fputs("@RIGHT\n", fp);
    dump_node(n->u.binary_expression.right, fp, lv + 1);
}

void dump_node(JadeNode *n, FILE *fp, int lv){
    switch (n->type) {
    case JADE_BLOCK_NODE:
        dump_nodelist(n->u.block.statements, fp, lv);
        break;
    case JADE_EXPRESSION_STATEMENT_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "EXPRESSION_STATEMENT", fp);
        fputc('\n', fp);
        dump_node(n->u.expression_statement.expression, fp, lv + 1);
        break;
    case JADE_DEF_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "DEF", fp);
        fputc('\n', fp);
        INDENT(lv + 1, fp);
        fputs("@NAME\n", fp);
        dump_node(n->u.def.name, fp, lv + 1);
        INDENT(lv + 1, fp);
        fputs("@INIT\n", fp);
        dump_node(n->u.def.init, fp, lv + 1);
        break;
    case JADE_IF_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "IF", fp);
        fputc('\n', fp);
        INDENT(lv + 1, fp);
        fputs("@TEST\n", fp);
        dump_node(n->u.if_.test, fp, lv + 1);
        INDENT(lv + 1, fp);
        fputs("@BODY\n", fp);
        dump_node(n->u.if_.body, fp, lv + 1);
        INDENT(lv + 1, fp);
        fputs("@ALT\n", fp);
        if (n->u.if_.alt == NULL) {
            INDENT(lv + 1, fp);
            fputs("(NULL)\n", fp);
        } else {
            dump_node(n->u.if_.alt, fp, lv + 1);
        }
        break;
    case JADE_WHILE_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "WHILE", fp);
        fputc('\n', fp);
        INDENT(lv + 1, fp);
        fputs("@COND\n", fp);
        dump_node(n->u.loop.cond, fp, lv + 1);
        INDENT(lv + 1, fp);
        fputs("@BODY\n", fp);
        dump_node(n->u.loop.body, fp, lv + 1);
        break;
    case JADE_UNTIL_NODE:
    case JADE_FOR_NODE:
        assert(0);
    case JADE_BREAK_NODE:
    case JADE_CONTINUE_NODE:
    case JADE_RETURN_NODE:
    case JADE_THROW_NODE:
        dump_control_transfer(n, fp, lv);
        break;
    case JADE_TRY_NODE:
        assert(0);
    case JADE_IDENTIFIER_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "IDENTIFIER", fp);
        fprintf(fp, " %s\n", n->u.identifier.name);
        break;
    case JADE_INT_LITERAL_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "INT_LITERAL", fp);
        fprintf(fp, " %d\n", n->u.int_literal.value);
        break;
    case JADE_FLOAT_LITERAL_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "FLOAT_LITERAL", fp);
        fprintf(fp, " %lf\n", n->u.float_literal.value);
        break;
    case JADE_STRING_LITERAL_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "STRING_LITERAL", fp);
        fprintf(fp, " %s\n", n->u.string_literal.value);
        break;
    case JADE_BOOLEAN_LITERAL_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "BOOLEAN_LITERAL", fp);
        fprintf(fp, " %s\n", (n->u.boolean_literal.value ? "true" : "false"));
        break;
    case JADE_NIL_LITERAL_NODE:
    case JADE_ARRAY_LITERAL_NODE:
    case JADE_TABLE_LITERAL_NODE:
    case JADE_TABLE_ENTRY_NODE:
        assert(0);
    case JADE_FUNCTION_LITERAL_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "FUNCTION_LITERAL", fp);
        fputc('\n', fp);
        INDENT(lv + 1, fp);
        fputs("@PARAMETERS\n", fp);
        dump_nodelist(n->u.function_literal.parameters, fp, lv + 1);
        INDENT(lv + 1, fp);
        fputs("@BODY\n", fp);
        dump_node(n->u.function_literal.body, fp, lv + 1);
        break;
    case JADE_MINUS_NODE:
    case JADE_NOT_NODE:
        assert(0);
    case JADE_LOGICAL_OR_NODE:
    case JADE_LOGICAL_AND_NODE:
    case JADE_EQUAL_NODE:
    case JADE_NOT_EQUAL_NODE:
    case JADE_LESS_THAN_NODE:
    case JADE_GREATER_THAN_NODE:
    case JADE_LESS_THAN_EQUAL_NODE:
    case JADE_GREATER_THAN_EQUAL_NODE:
    case JADE_ADDITION_NODE:
    case JADE_SUBTRACTION_NODE:
    case JADE_MULTIPLICATION_NODE:
    case JADE_DIVISION_NODE:
    case JADE_MODULO_NODE:
        dump_binary_expression(n, fp, lv);
        break;
    case JADE_LET_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "LET", fp);
        fputc('\n', fp);
        INDENT(lv + 1, fp);
        fputs("@NAME\n", fp);
        dump_node(n->u.let.name, fp, lv + 1);
        INDENT(lv + 1, fp);
        fputs("@OPERAND\n", fp);
        dump_node(n->u.let.operand, fp, lv + 1);
        break;
    case JADE_CALL_NODE:
        INDENT(lv, fp);
        DUMP_CAPTION(n, "CALL", fp);
        fputc('\n', fp);
        INDENT(lv + 1, fp);
        fputs("@FUNCTION\n", fp);
        dump_node(n->u.call.function, fp, lv + 1);
        INDENT(lv + 1, fp);
        fputs("@ARGUMENTS\n", fp);
        dump_nodelist(n->u.call.arguments, fp, lv + 1);
        break;
    case JADE_FETCH_NODE:
    case JADE_STORE_NODE:
    case JADE_MATCH_NODE:
    case JADE_MATCH_CLAUSE_NODE:
    default:
        fprintf(stderr, "<<<%d>>>\n", n->type);
        assert(0);
    }
}
