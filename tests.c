#define TEST_IMPLEMENTATION
#include "test.h"

void test1(test_context *context) {

}

int main(void) {
  test_context context = test_init();

  test_register(&context, &test1);

  test_run(&context);
}
