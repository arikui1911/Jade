#include <jade.h>
#include <jade/ast.h>
#include "./state.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "./vector_using.h"
#include <setjmp.h>

static JadeValue eval_block(JadeState *jade, JadeNode *x, Environment *env){
    JadeNode *pos;
    JadeValue r = jade_nil_value();

    for (pos = x->u.block.statements; pos != NULL; pos = pos->u.list.next) {
        r = eval(jade, pos->u.list.node, env);
    }

    return r;
}

static JadeValue eval_def(JadeState *jade, JadeNode *x, Environment *env){
    JadeID var;
    JadeValue init = jade_nil_value();

    if (x->u.def.init != NULL) init = eval(jade, x->u.def.init, env);
    var = jade_intern(jade, x->u.def.name->u.identifier.name);

    env_store(env, var, init);
    return init;
}

static JadeValue eval_if(JadeState *jade, JadeNode *x, Environment *env){
    JadeNode *n = (jade_test(eval(jade, x->u.if_.test, env)) ? x->u.if_.body : x->u.if_.alt);

    if (n == NULL) return jade_nil_value();
    return eval(jade, n, env);
}

static JadeValue eval_while(JadeState *jade, JadeNode *x, Environment *env) {
    JadeValue r = jade_nil_value();

    while (jade_test(eval(jade, x->u.loop.cond, env))) {
        Jump old = jade->current_jump;
        
        if (setjmp(jade->current_jump.buf) == 0) {
            r = eval(jade, x->u.loop.body, env);
        } else {
            if (jade->current_jump.type == JUMP_BREAK) {
                r = jade->current_jump.value;
                break;
            }
            if (jade->current_jump.type == JUMP_THROW || jade->current_jump.type == JUMP_RETURN) longjmp(old.buf, 1);
        }

        jade->current_jump = old;
    }
    return r;
}

static JadeValue eval_control_transfer(JadeState *jade, JadeNode *x, JumpType type, Environment *env){
    JadeValue r = jade_nil_value();
    if (x->u.control_transfer.result != NULL) {
        r = eval(jade, x->u.control_transfer.result, env);
    }
    jade->current_jump.type = type;
    jade->current_jump.value = r;
    longjmp(jade->current_jump.buf, 1);
    return jade_undefined_value();
}

static JadeValue eval_identifier(JadeState *jade, JadeNode *x, Environment *env){
    JadeID var;
    Environment *e;

    var = jade_intern(jade, x->u.identifier.name);
    e = env_find(env, var);
    if (e == NULL) {
        jade_raise(
            jade, "%s:(%d:%d):(%d:%d): undefined variable - '%s'",
            jade->source_filename,
            x->pos.start_line, x->pos.start_column, x->pos.end_line, x->pos.end_column,
            x->u.identifier.name
        );
    }

    return env_fetch(e, var);
}

static JadeValue eval_let(JadeState *jade, JadeNode *x, Environment *env){
    JadeID var;
    JadeValue v;
    Environment *e;

    var = jade_intern(jade, x->u.let.name->u.identifier.name);
    v = eval(jade, x->u.let.operand, env);
    e = env_find(env, var);
    if (e == NULL) e = env;

    env_store(e, var, v);
    return v;
}

static JadeValue eval_array_literal(JadeState *jade, JadeNode *x, Environment *env){
    JadeNode *pos;
    JadeValue a = jade_array_new(jade);

    for (pos = x->u.array_literal.elements; pos != NULL; pos = pos->u.list.next) {
        jade_array_push(jade, a, eval(jade, pos->u.list.node, env));
    }

    return a;
}

static JadeValue eval_table_literal(JadeState *jade, JadeNode *x, Environment *env){
    JadeNode *pos;
    JadeValue t = jade_table_new(jade);

    for (pos = x->u.table_literal.entries; pos != NULL; pos = pos->u.list.next) {
        JadeValue k, v;

        assert(pos->u.list.node->type == JADE_TABLE_ENTRY_NODE);
        k = eval(jade, pos->u.list.node->u.table_entry.key, env);
        v = eval(jade, pos->u.list.node->u.table_entry.value, env);
        jade_table_store(jade, t, k, v);
    }

    return t;
}

static JadeValue eval_function_literal(JadeState *jade, JadeNode *x, Environment *env){
    return function_new(jade, x->u.function_literal.parameters, x->u.function_literal.body);
}

static JadeValue eval_minus_expression(JadeState *jade, JadeNode *x, Environment *env){
    JadeValue v = eval(jade, x->u.unary_expression.operand, env);
    switch (v.type) {
    case JADE_INT:
        return jade_int_value(v.u.int_value);
    case JADE_FLOAT:
        return jade_float_value(v.u.float_value);
    default:
        jade_raise(jade,
            "(%d:%d):(%d:%d): invalid operation for %s",
            x->pos.start_line, x->pos.start_column, x->pos.end_line, x->pos.end_column,
            jade_value_type_name(v.type)
        );
    }
    return jade_undefined_value();
}

static JadeValue eval_not_expression(JadeState *jade, JadeNode *x, Environment *env){
    JadeValue v = eval(jade, x->u.unary_expression.operand, env);
    return jade_bool_value(!jade_test(v));
}

static JadeValue eval_logical_expression(JadeState *jade, JadeNode *x, Environment *env){
    int r;
    JadeValue v = eval(jade, x->u.binary_expression.left, env);
    r = jade_test(v);
    if (x->type == JADE_LOGICAL_AND_NODE) r = !r;
    if (r) return v;
    return eval(jade, x->u.binary_expression.right, env);
}

static JadeValue eval_equality_expression(JadeState *jade, JadeNode *x, Environment *env){
    int r;
    JadeValue a, b;

    a = eval(jade, x->u.binary_expression.left, env);
    b = eval(jade, x->u.binary_expression.right, env);
    r = jade_equal(a, b);
    if (x->type == JADE_NOT_EQUAL_NODE) r = !r;

    return jade_bool_value(r);
}

static JadeValue eval_compare_expression(JadeState *jade, JadeNode *x, Environment *env){
    JadeValue a, b;
    int r;

    a = eval(jade, x->u.binary_expression.left, env);
    b = eval(jade, x->u.binary_expression.right, env);
    r = jade_compare(a, b);

    if (r < -1 || r > 1) {
        jade_raise(jade,
            "invalid compare expression - %s and %s",
            jade_value_type_name(a.type), jade_value_type_name(b.type)
        );
    }

    switch (x->type) {
    case JADE_LESS_THAN_NODE:
        return jade_bool_value(r == -1);
    case JADE_GREATER_THAN_NODE:
        return jade_bool_value(r == 1);
    case JADE_LESS_THAN_EQUAL_NODE:
        return jade_bool_value(r <= 0);
    case JADE_GREATER_THAN_EQUAL_NODE:
        return jade_bool_value(r >= 0);
    default:
        assert(0);
    }
    return jade_undefined_value();
}

static JadeValue int_arith_op(JadeState *jade, JadeNodeType op, int l, int r){
    int v;

    switch (op) {
    case JADE_ADDITION_NODE:
        v = l + r;
        break;
    case JADE_SUBTRACTION_NODE:
        v = l - r;
        break;
    case JADE_MULTIPLICATION_NODE:
        v = l * r;
        break;
    case JADE_DIVISION_NODE:
        if (r == 0) jade_raise(jade, "divided by zero");
        v = l / r;
        break;
    case JADE_MODULO_NODE:
        if (r == 0) jade_raise(jade, "divided by zero");
        v = l % r;
        break;
    default:
        assert(0);
    }

    return jade_int_value(v);
}

static JadeValue float_arith_op(JadeState *jade, JadeNodeType op, double l, double r){
    double v;

    switch (op) {
    case JADE_ADDITION_NODE:
        v = l + r;
        break;
    case JADE_SUBTRACTION_NODE:
        v = l - r;
        break;
    case JADE_MULTIPLICATION_NODE:
        v = l * r;
        break;
    case JADE_DIVISION_NODE:
        if (r == 0) {
            if (l == 0) {
                v = NAN;
            } else {
                v = (l > 0 ? INFINITY : -INFINITY);
            }
        } else {
            v = l / r;
        }
        break;
    case JADE_MODULO_NODE:
        if (r == 0) jade_raise(jade, "divided by zero");
        v = (double)((int)l % (int)r);
        break;
    default:
        assert(0);
    }

    return jade_float_value(v);
}

static JadeValue eval_arith_expression(JadeState *jade, JadeNode *x, Environment *env){
    JadeValue a, b;

    a = eval(jade, x->u.binary_expression.left, env);
    b = eval(jade, x->u.binary_expression.right, env);

    if (x->type == JADE_ADDITION_NODE) {
        switch (a.type) {
        case JADE_STRING:
            return jade_string_concat(jade, a, b);
        case JADE_ARRAY:
            return jade_array_concat(jade, a, b);
        default:
            break;
        }
    }

    switch (a.type) {
    case JADE_INT:
        switch (b.type) {
        case JADE_INT:
            return int_arith_op(jade, x->type, a.u.int_value, b.u.int_value);
        case JADE_FLOAT:
            return float_arith_op(jade, x->type, a.u.int_value, b.u.float_value);
        default:
            break;
        }
        break;
    case JADE_FLOAT:
        switch (b.type) {
        case JADE_INT:
            return float_arith_op(jade, x->type, a.u.float_value, b.u.int_value);
        case JADE_FLOAT:
            return float_arith_op(jade, x->type, a.u.float_value, b.u.float_value);
        default:
            break;
        }
        break;
    default:
        break;
    }

    jade_raise(jade,
        "invalid arithmetic operation - %s and %s",
        jade_value_type_name(a.type), jade_value_type_name(b.type)
    );

    return jade_undefined_value();
}

static JadeValue eval_fetch(JadeState *jade, JadeNode *x, Environment *env){
    JadeValue c, k;

    c = eval(jade, x->u.fetch.container, env);
    k = eval(jade, x->u.fetch.key, env);

    switch (c.type) {
    case JADE_ARRAY:
        return jade_array_fetch(jade, c, k);
    case JADE_TABLE:
        return jade_table_fetch(jade, c, k);
    default:
        jade_raise(jade, "not a container - %s", jade_value_type_name(c.type));
    }

    return jade_undefined_value();
}

static JadeValue eval_store(JadeState *jade, JadeNode *x, Environment *env){
    JadeValue c, k, v;

    c = eval(jade, x->u.store.fetch->u.fetch.container, env);
    k = eval(jade, x->u.store.fetch->u.fetch.key, env);
    v = eval(jade, x->u.store.value, env);

    switch (c.type) {
    case JADE_ARRAY:
        return jade_array_store(jade, c, k, v);
    case JADE_TABLE:
        return jade_table_store(jade, c, k, v);
    default:
        jade_raise(jade, "not a container - %s", jade_value_type_name(c.type));
    }

    return jade_undefined_value();
}

static JadeValue eval_call(JadeState *jade, JadeNode *x, Environment *env){
    JadeNode *pos;
    JadeValue fn, ret = jade_undefined_value();
    Environment *fe;
    int i;
    Jump old;
    vector_t(JadeValue) args;
    vector_init(args);

    fn = eval(jade, x->u.call.function, env);
    for (pos = x->u.call.arguments; pos != NULL; pos = pos->u.list.next) {
        vector_push(jade, args, JadeValue, eval(jade, pos->u.list.node, env));
    }

    if (fn.type != JADE_FUNCTION) {
        jade_raise(jade,
            "%s:(%d:%d):(%d:%d): not a function - %s",
            jade->source_filename,
            x->u.call.function->pos.start_line,
            x->u.call.function->pos.start_column,
            x->u.call.function->pos.end_line,
            x->u.call.function->pos.end_column,
            jade_value_type_name(fn.type)
        );
    }

    switch (fn.u.object->u.function.type) {
    case JADE_USER_FUNCTION:
        if (vector_len(args) != vector_len(fn.u.object->u.function.u.user.parameters)) {
            jade_raise(jade,
                "%s:(%d:%d):(%d:%d): wrong number of arguments - given %d, expected %d",
                jade->source_filename,
                x->u.call.function->pos.start_line,
                x->u.call.function->pos.start_column,
                x->u.call.function->pos.end_line,
                x->u.call.function->pos.end_column,
                vector_len(args),
                vector_len(fn.u.object->u.function.u.user.parameters)
            );
        }
        fe = env_new(jade, env);
        for (i = 0; i < vector_len(args); i++) {
            env_store(fe, vector_get(fn.u.object->u.function.u.user.parameters, i), vector_get(args, i));
        }
        jade->current = fe;
        old = jade->current_jump;
        if (setjmp(jade->current_jump.buf) == 0) {
            ret = eval(jade, fn.u.object->u.function.u.user.body, fe);
        } else {
            if (jade->current_jump.type == JUMP_BREAK) jade_raise(jade, "invalid break");                
            if (jade->current_jump.type == JUMP_CONTINUE) jade_raise(jade, "invalid continue");
            if (jade->current_jump.type == JUMP_THROW) longjmp(old.buf, 1);
            if (jade->current_jump.type == JUMP_RETURN) ret = jade->current_jump.value;
        }
        jade->current_jump = old;
        jade->current = env;
        env_dispose(fe);
        break;
    case JADE_NATIVE_FUNCTION:
        ret = fn.u.object->u.function.u.native.body(jade, vector_ptr(args), vector_len(args));
        break;
    default:
        assert(0);
    }

    vector_destroy(jade, args);
    return ret;
}

JadeValue eval(JadeState *jade, JadeNode *x, Environment *env){
    switch (x->type) {
    case JADE_BLOCK_NODE:
        return eval_block(jade, x, env);
    case JADE_EXPRESSION_STATEMENT_NODE:
        return eval(jade, x->u.expression_statement.expression, env);
    case JADE_DEF_NODE:
        return eval_def(jade, x, env);
    case JADE_IF_NODE:
        return eval_if(jade, x, env);
    case JADE_WHILE_NODE:
        return eval_while(jade, x, env);
    case JADE_UNTIL_NODE:
    case JADE_FOR_NODE:
        assert(0);
    case JADE_BREAK_NODE:
        return eval_control_transfer(jade, x, JUMP_BREAK, env);
    case JADE_CONTINUE_NODE:
        return eval_control_transfer(jade, x, JUMP_CONTINUE, env);
    case JADE_RETURN_NODE:
        return eval_control_transfer(jade, x, JUMP_RETURN, env);
    case JADE_THROW_NODE:
        return eval_control_transfer(jade, x, JUMP_THROW, env);
    case JADE_TRY_NODE:
        assert(0);
    case JADE_IDENTIFIER_NODE:
        return eval_identifier(jade, x, env);
    case JADE_INT_LITERAL_NODE:
        return jade_int_value(x->u.int_literal.value);
    case JADE_FLOAT_LITERAL_NODE:
        return jade_float_value(x->u.float_literal.value);
    case JADE_STRING_LITERAL_NODE:
        return jade_string_new(jade, x->u.string_literal.value);
    case JADE_BOOLEAN_LITERAL_NODE:
        return jade_bool_value(x->u.boolean_literal.value);
    case JADE_NIL_LITERAL_NODE:
        return jade_nil_value();
    case JADE_ARRAY_LITERAL_NODE:
        return eval_array_literal(jade, x, env);
    case JADE_TABLE_LITERAL_NODE:
        return eval_table_literal(jade, x, env);
    case JADE_FUNCTION_LITERAL_NODE:
        return eval_function_literal(jade, x, env);
    case JADE_MINUS_NODE:
        return eval_minus_expression(jade, x, env);
    case JADE_NOT_NODE:
        return eval_not_expression(jade, x, env);
    case JADE_LOGICAL_OR_NODE:
        return eval_logical_expression(jade, x, env);
    case JADE_LOGICAL_AND_NODE:
        return eval_logical_expression(jade, x, env);
    case JADE_EQUAL_NODE:
    case JADE_NOT_EQUAL_NODE:
        return eval_equality_expression(jade, x, env);
    case JADE_LESS_THAN_NODE:
    case JADE_GREATER_THAN_NODE:
    case JADE_LESS_THAN_EQUAL_NODE:
    case JADE_GREATER_THAN_EQUAL_NODE:
        return eval_compare_expression(jade, x, env);
    case JADE_ADDITION_NODE:
    case JADE_SUBTRACTION_NODE:
    case JADE_MULTIPLICATION_NODE:
    case JADE_DIVISION_NODE:
    case JADE_MODULO_NODE:
        return eval_arith_expression(jade, x, env);
    case JADE_LET_NODE:
        return eval_let(jade, x, env);
    case JADE_CALL_NODE:
        return eval_call(jade, x, env);
    case JADE_FETCH_NODE:
        return eval_fetch(jade, x, env);
    case JADE_STORE_NODE:
        return eval_store(jade, x, env);
    case JADE_MATCH_NODE:
    default:
        assert(0);
    }
}
