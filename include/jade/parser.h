#ifndef JADE_PARSER_H
#define JADE_PARSER_H
#include <jade.h>
#include <stdio.h>

typedef struct JadeParser JadeParser;

JadeParser *jade_parser_new(JadeState *);
void jade_parser_dispose(JadeParser *);

void jade_parser_parse_string(JadeParser *, const char*, int, const char*);
void jade_parser_parse_file(JadeParser *, const char*, int, FILE *);

#endif /* JADE_PARSER_H */
