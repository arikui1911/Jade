#include "../Unity/src/unity.h"
#include <jade.h>

void setUp(void) {}

void tearDown(void) {}
 
void test_intern(void) {
    JadeState *jade;
    JadeID id, id2;

    jade = jade_new();

    id = jade_intern(jade, "hoge");
    // TEST_ASSERT_EQUAL(0, id);

    id2 = jade_intern(jade, "piyo");
    TEST_ASSERT_EQUAL(id + 1, id2);

    id2 = jade_intern(jade, "hoge");
    TEST_ASSERT_EQUAL(id, id2);

    jade_dispose(jade);
}

void test_id_name(void) {
    JadeState *jade;
    JadeID id;

    jade = jade_new();

    id = jade_intern(jade, "hoge");
    TEST_ASSERT_EQUAL_STRING("hoge", jade_id_name(jade, id));

    id = jade_intern(jade, "piyo");
    TEST_ASSERT_EQUAL_STRING("piyo", jade_id_name(jade, id));

    TEST_ASSERT_NULL(jade_id_name(jade, 666));

    jade_dispose(jade);
}
