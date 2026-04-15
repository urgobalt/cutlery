#define TEST_IMPLEMENTATION
#include "test.h"

void test1(void) {
  printf("This should not be shown\n");
}

int main(void) {
  test_context context = test_init();

  test_register(&context, &test1);

  test_run(&context);
}
