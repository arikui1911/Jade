#include <jade.h>
#include <jade/ast.h>
#include "./util.h"
#include "./node.h"
#include <stdlib.h>
#include <assert.h>

static JadeNode *create(JadeState *jade, JadeNodeType type, JadePosition pos){
    JadeNode *n = jmalloc(jade, sizeof(JadeNode));
    n->type = type;
    n->pos = pos;
    return n;
}

JadeNode *create_block(JadeState *jade, JadePosition pos, JadeNodeList *statements){
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
    JadeNode *n = create(jade, JADE_WHILE_NODE, pos);
    n->u.loop.cond = cond;
    n->u.loop.body = body;
    return n;
}

JadeNode *create_for(JadeState *jade, JadePosition pos, JadeNodeList *vars, JadeNode *range, JadeNode *body){
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

JadeNode *create_array_literal(JadeState *jade, JadePosition pos, JadeNodeList *elements){
    JadeNode *n = create(jade, JADE_ARRAY_LITERAL_NODE, pos);
    n->u.array_literal.elements = elements;
    return n;
}

JadeNode *create_table_literal(JadeState *jade, JadePosition pos, JadeNodeList *entries){
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

JadeNode *create_function_literal(JadeState *jade, JadePosition pos, JadeNodeList *params, JadeNode *body){
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

JadeNode *create_call(JadeState *jade, JadePosition pos, JadeNode *fn, JadeNodeList *args){
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
    JadeNodeList *clauses,
    JadeNode *otherwise
){
    JadeNode *n = create(jade, JADE_MATCH_NODE, pos);
    n->u.match.operand = operand;
    n->u.match.variable = variable;
    n->u.match.clauses = clauses;
    n->u.match.otherwise = otherwise;
    return n;
}

JadeNode *create_match_clause(JadeState *jade, JadePosition pos, JadeNodeList *pattern, JadeNode *body){
    JadeNode *n = create(jade, JADE_MATCH_CLAUSE_NODE, pos);
    n->u.match_clause.pattern = pattern;
    n->u.match_clause.body = body;
    return n;
}

static void dispose_nodelist(JadeNodeList *list){
    JadeNodeList *pos, *next;
    for (pos = list; pos != NULL; pos = next) {
        next = pos->next;
        dispose_node(pos->node);
        free(pos);
    }
}

void dispose_node(JadeNode *n){
    if (n == NULL) return;
    switch (n->type) {
    case JADE_BLOCK_NODE:
        dispose_nodelist(n->u.block.statements);
        break;
    case JADE_EXPRESSION_STATEMENT_NODE:
        dispose_node(n->u.expression_statement.expression);
        break;
    case JADE_DEF_NODE:
        dispose_node(n->u.def.name);
        dispose_node(n->u.def.init);
        break;
    case JADE_IF_NODE:
        dispose_node(n->u.if_.test);
        dispose_node(n->u.if_.body);
        dispose_node(n->u.if_.alt);
        break;
    case JADE_WHILE_NODE:
    case JADE_UNTIL_NODE:
        dispose_node(n->u.loop.cond);
        dispose_node(n->u.loop.body);
        break;
    case JADE_FOR_NODE:
        dispose_nodelist(n->u.for_.variables);
        dispose_node(n->u.for_.range);
        dispose_node(n->u.for_.body);
        break;
    case JADE_BREAK_NODE:
    case JADE_CONTINUE_NODE:
    case JADE_RETURN_NODE:
    case JADE_THROW_NODE:
        dispose_node(n->u.control_transfer.result);
        break;
    case JADE_TRY_NODE:
        dispose_node(n->u.try.body);
        dispose_node(n->u.try.variable);
        dispose_node(n->u.try.rescue);
        dispose_node(n->u.try.ensure);
        break;
    case JADE_IDENTIFIER_NODE:
        free(n->u.identifier.name);
        break;
    case JADE_INT_LITERAL_NODE:
    case JADE_FLOAT_LITERAL_NODE:
        break;
    case JADE_STRING_LITERAL_NODE:
        free(n->u.string_literal.value);
        break;
    case JADE_BOOLEAN_LITERAL_NODE:
    case JADE_NIL_LITERAL_NODE:
        break;
    case JADE_ARRAY_LITERAL_NODE:
        dispose_nodelist(n->u.array_literal.elements);
        break;
    case JADE_TABLE_LITERAL_NODE:
        dispose_nodelist(n->u.table_literal.entries);
        break;
    case JADE_TABLE_ENTRY_NODE:
        dispose_node(n->u.table_entry.key);
        dispose_node(n->u.table_entry.value);
        break;
    case JADE_FUNCTION_LITERAL_NODE:
        dispose_nodelist(n->u.function_literal.parameters);
        dispose_node(n->u.function_literal.body);
        break;
    case JADE_MINUS_NODE:
    case JADE_NOT_NODE:
        dispose_node(n->u.unary_expression.operand);
        break;
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
        dispose_node(n->u.binary_expression.left);
        dispose_node(n->u.binary_expression.right);
        break;
    case JADE_LET_NODE:
        dispose_node(n->u.let.name);
        dispose_node(n->u.let.operand);
        break;
    case JADE_CALL_NODE:
        dispose_nodelist(n->u.call.arguments);
        dispose_node(n->u.call.function);
        break;
    case JADE_FETCH_NODE:
        dispose_node(n->u.fetch.container);
        dispose_node(n->u.fetch.key);
        break;
    case JADE_STORE_NODE:
        dispose_node(n->u.store.fetch);
        dispose_node(n->u.store.value);
        break;
    case JADE_MATCH_NODE:
        dispose_node(n->u.match.operand);
        dispose_node(n->u.match.variable);
        dispose_nodelist(n->u.match.clauses);
        dispose_node(n->u.match.otherwise);
        break;
    case JADE_MATCH_CLAUSE_NODE:
        dispose_nodelist(n->u.match_clause.pattern);
        dispose_node(n->u.match_clause.body);
        break;
    }
    free(n);
}

static JadeNodeList *copy_nodelist(JadeState *jade, JadeNodeList *list){
    JadeNodeList head = {NULL, NULL};
    JadeNodeList *pos = &head;

    for (; list != NULL; list = list->next) {
        pos->next = jmalloc(jade, sizeof(JadeNodeList));
        pos = pos->next;
        pos->next = NULL;
        pos->node = copy_node(jade, list->node);
    }

    return head.next;
}

JadeNode *copy_node(JadeState *jade, JadeNode *n){
    if (n == NULL) return n;
    switch (n->type) {
    case JADE_BLOCK_NODE:
        return create_block(jade, n->pos, copy_nodelist(jade, n->u.block.statements));
    case JADE_EXPRESSION_STATEMENT_NODE:
        return create_expression_statement(jade, n->pos, copy_node(jade, n->u.expression_statement.expression));
    case JADE_DEF_NODE:
        return create_def(jade, n->pos, copy_node(jade, n->u.def.name), copy_node(jade, n->u.def.init));
    case JADE_IF_NODE:
        return create_if(jade, n->pos, n->u.if_.test, n->u.if_.body, n->u.if_.alt);
    case JADE_WHILE_NODE:
        return create_while(jade, n->pos, n->u.loop.cond, n->u.loop.body);
    case JADE_UNTIL_NODE:
        return create_until(jade, n->pos, n->u.loop.cond, n->u.loop.body);
    case JADE_FOR_NODE:
        return create_for(jade, n->pos, n->u.for_.variables, n->u.for_.range, n->u.for_.body);
    case JADE_BREAK_NODE:
        return create_break(jade, n->pos, n->u.control_transfer.result);
    case JADE_CONTINUE_NODE:
        return create_continue(jade, n->pos, n->u.control_transfer.result);
    case JADE_RETURN_NODE:
        return create_return(jade, n->pos, n->u.control_transfer.result);
    case JADE_THROW_NODE:
        return create_throw(jade, n->pos, n->u.control_transfer.result);
    case JADE_TRY_NODE:
        return create_try(jade, n->pos, n->u.try.body, n->u.try.variable, n->u.try.rescue, n->u.try.ensure);
    case JADE_IDENTIFIER_NODE:
        return create_identifier(jade, n->pos, jstrdup(jade, n->u.identifier.name));
    case JADE_INT_LITERAL_NODE:
        return create_int_literal(jade, n->pos, n->u.int_literal.value);
    case JADE_FLOAT_LITERAL_NODE:
        return create_float_literal(jade, n->pos, n->u.float_literal.value);
    case JADE_STRING_LITERAL_NODE:
        return create_string_literal(jade, n->pos, jstrdup(jade, n->u.string_literal.value));
    case JADE_BOOLEAN_LITERAL_NODE:
        return create_boolean_literal(jade, n->pos, n->u.boolean_literal.value);
    case JADE_NIL_LITERAL_NODE:
        return create_nil_literal(jade, n->pos);
    case JADE_ARRAY_LITERAL_NODE:
        return create_array_literal(jade, n->pos, copy_nodelist(jade, n->u.array_literal.elements));
    case JADE_TABLE_LITERAL_NODE:
        return create_table_literal(jade, n->pos, copy_nodelist(jade, n->u.table_literal.entries));
    case JADE_TABLE_ENTRY_NODE:
        return create_table_entry(jade, n->pos, copy_node(jade, n->u.table_entry.key), copy_node(jade, n->u.table_entry.value));
    case JADE_FUNCTION_LITERAL_NODE:
        return create_function_literal(
            jade, n->pos,
            copy_nodelist(jade, n->u.function_literal.parameters),
            copy_node(jade, n->u.function_literal.body)
        );
    case JADE_MINUS_NODE:
        return create_minus(jade, n->pos, copy_node(jade, n->u.unary_expression.operand));
    case JADE_NOT_NODE:
        return create_not(jade, n->pos, copy_node(jade, n->u.unary_expression.operand));
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
        return create_binary_expression(
            jade, n->pos, n->type,
            n->u.binary_expression.left,
            n->u.binary_expression.right
        );
    case JADE_LET_NODE:
        return create_let(jade, n->pos, copy_node(jade, n->u.let.name), copy_node(jade, n->u.let.operand));
    case JADE_CALL_NODE:
        return create_call(
            jade, n->pos,
            copy_node(jade, n->u.call.function),
            copy_nodelist(jade, n->u.call.arguments)
        );
    case JADE_FETCH_NODE:
        return create_fetch(jade, n->pos, copy_node(jade, n->u.fetch.container), copy_node(jade, n->u.fetch.key));
    case JADE_STORE_NODE:
        return create_store(jade, n->pos, copy_node(jade, n->u.store.fetch), copy_node(jade, n->u.store.value));
    case JADE_MATCH_NODE:
        return create_match(
            jade, n->pos,
            copy_node(jade, n->u.match.operand),
            copy_node(jade, n->u.match.variable),
            copy_nodelist(jade, n->u.match.clauses),
            copy_node(jade, n->u.match.otherwise)
        );
    case JADE_MATCH_CLAUSE_NODE:
        return create_match_clause(jade, n->pos, copy_nodelist(jade, n->u.match_clause.pattern), copy_node(jade, n->u.match_clause.body));
    default:
        assert(0);
    }
}

static void dump_nodelist(JadeNodeList *list, FILE *fp, int lv){
    JadeNodeList *pos;
    for (pos = list; pos != NULL; pos = pos->next) {
        dump_node(pos->node, fp, lv);
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
