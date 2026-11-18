#ifndef JADE_PRIVATE_NODE_H
#define JADE_PRIVATE_NODE_H
#include <jade.h>
#include <jade/ast.h>
#include <stdio.h>

JadeNode *create_list_entry(JadeState *, JadeNode *, JadeNode *);

JadeNode *create_block(JadeState *, JadePosition, JadeNode *);

JadeNode *create_def(JadeState *, JadePosition, JadeNode *, JadeNode *);
JadeNode *create_if(JadeState *, JadePosition, JadeNode *, JadeNode *, JadeNode *);
JadeNode *create_while(JadeState *, JadePosition, JadeNode *, JadeNode *);
JadeNode *create_until(JadeState *, JadePosition, JadeNode *, JadeNode *);
JadeNode *create_for(JadeState *, JadePosition, JadeNode *, JadeNode *, JadeNode *);
JadeNode *create_try(JadeState *, JadePosition, JadeNode *, JadeNode *, JadeNode *, JadeNode *);
JadeNode *create_break(JadeState *, JadePosition, JadeNode *);
JadeNode *create_continue(JadeState *, JadePosition, JadeNode *);
JadeNode *create_return(JadeState *, JadePosition, JadeNode *);
JadeNode *create_throw(JadeState *, JadePosition, JadeNode *);

JadeNode *create_expression_statement(JadeState *, JadePosition, JadeNode *);

JadeNode *create_identifier(JadeState *, JadePosition, char *);
JadeNode *create_int_literal(JadeState *, JadePosition, int);
JadeNode *create_float_literal(JadeState *, JadePosition, double);
JadeNode *create_string_literal(JadeState *, JadePosition, char *);
JadeNode *create_boolean_literal(JadeState *, JadePosition, int);
JadeNode *create_nil_literal(JadeState *, JadePosition);

JadeNode *create_array_literal(JadeState *, JadePosition, JadeNode *);
JadeNode *create_table_literal(JadeState *, JadePosition, JadeNode *);
JadeNode *create_table_entry(JadeState *, JadePosition, JadeNode *, JadeNode *);

JadeNode *create_function_literal(JadeState *, JadePosition, JadeNode *, JadeNode *);

JadeNode *create_not(JadeState *, JadePosition, JadeNode *);
JadeNode *create_minus(JadeState *, JadePosition, JadeNode *);
JadeNode *create_binary_expression(JadeState *, JadePosition, JadeNodeType, JadeNode *, JadeNode *);

JadeNode *create_let(JadeState *, JadePosition, JadeNode *, JadeNode *);
JadeNode *create_call(JadeState *, JadePosition, JadeNode *, JadeNode *);
JadeNode *create_fetch(JadeState *, JadePosition, JadeNode *, JadeNode *);
JadeNode *create_store(JadeState *, JadePosition, JadeNode *, JadeNode *);

JadeNode *create_match(JadeState *, JadePosition, JadeNode *, JadeNode *, JadeNode *, JadeNode *);
JadeNode *create_match_clause(JadeState *, JadePosition, JadeNode *, JadeNode *);

void dispose_node(JadeNode *);

void dump_node(JadeNode *, FILE *, int);

#endif /* JADE_PRIVATE_NODE_H */
