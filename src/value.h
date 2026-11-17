#ifndef JADE_PRIVATE_VALUE_H
#define JADE_PRIVATE_VALUE_H
#include <jade.h>
#include <khash.h>

khint_t hash_value(JadeValue);

khint_t string_hash_value(JadeValue);
khint_t array_hash_value(JadeValue);
khint_t table_hash_value(JadeValue);

int string_equal(JadeValue, JadeValue);
int array_equal(JadeValue, JadeValue);
int table_equal(JadeValue, JadeValue);

int string_compare(JadeValue, JadeValue);

JadeValue tostring(JadeState *, JadeValue, int);
JadeValue string_tostring(JadeState *, JadeValue, int);
JadeValue array_tostring(JadeState *, JadeValue, int);
JadeValue table_tostring(JadeState *, JadeValue, int);
JadeValue function_tostring(JadeState *, JadeValue);

#endif /* JADE_PRIVATE_VALUE_H */
