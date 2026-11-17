#include <jade.h>
#include <jade/parser.h>
#include <jade/ast.h>
#include <stdio.h>

int main(void){
    JadeState *jade = NULL;
    JadeParser *p = NULL;

    /*
    JadeProgramNode program;
    JadeNode *nd = (JadeNode *)(&program);

    program.header.pos.start_line = 1;
    program.header.pos.start_column = 1;
    program.header.pos.end_line = 1;
    program.header.pos.end_column = 1;
    program.header.type = JADE_PROGRAM_NODE_TYPE;
    program.filename = "hoge.jd";
    program.root = NULL;

    printf("<%s>\n", JADE_PROGRAM_NODE(nd)->filename);
    */

    jade = jade_new();
    p = jade_parser_new(jade);

    jade_parser_parse_string(p, "test.jd", 1, "hoge = 1\n");

    jade_parser_dispose(p);
    jade_dispose(jade);
    return 0;
}
