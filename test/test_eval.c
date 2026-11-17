#include "../Unity/src/unity.h"
#include <jade.h>
#include <jade/parser.h>
#include <stdio.h>

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

void test_eval_continue(void){
    const char *src = ""    \
        "def i = 0\n"       \
        "while i < 5 {\n"   \
        "  if i == 2 {\n"   \
        "    i += 1\n"      \
        "    continue\n"    \
        "  }\n"             \
        "  print(i)\n"      \
        "  i += 1\n"        \
        "}\n"               \
    "";

    jade_parser_parse_string(g_fixture.p, "test.jd", 1, src);
    jade_dump_ast(g_fixture.jade, stderr);
    jade_run(g_fixture.jade);
}

void test_eval_call(void){
    const char *src = ""        \
        "def sum = ->(x, y){\n" \
        "  while true {\n"      \
        "    return x + y\n"    \
        "  }\n"                 \
        "}\n"                   \
        "print(sum(10, 20))\n"       \
    "";

    jade_parser_parse_string(g_fixture.p, "test.jd", 1, src);
    jade_dump_ast(g_fixture.jade, stderr);
    jade_run(g_fixture.jade);
}
