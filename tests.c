#define TEST_IMPLEMENTATION
#include "test.h"

void failing(void) {
  printf("This should be shown in stdout\n");
  fprintf(stderr, "This should be shown in stderr\n");
  assert(false);
}
void passing(void) {
  printf("This should not be shown\n");
}

int main(void) {
  test_context context = test_init();

  test_register(&context, "failing", &failing);
  test_register(&context, "passing", &passing);

  test_run(&context);
}
