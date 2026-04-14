#define TEST_IMPLEMENTATION
#include "test.h"

void test1(void) {
  assert(false);
}

int main(void) {
  test_context context = test_init();

  test_register(&context, &test1);

  free(test_run(&context));

  free(context.tests);
}
