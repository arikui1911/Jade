#ifndef UTIL_PRIVATE_H
#define UTIL_PRIVATE_H
#include <jade.h>
#include <stddef.h>

#define jmalloc(jade_, size_)   (jrealloc((jade_), NULL, (size_)))
void *jrealloc(JadeState *, void *, size_t);
char *jstrdup(JadeState *, const char *);
char *jsprintf(JadeState *, const char *, ...);

#endif /* UTIL_PRIVATE_H */
