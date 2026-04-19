#define TEST_IMPLEMENTATION
#define FLAGS_IMPLEMENTATION

#include "test.h"
#include "flags.h"

void failing(void) {
  printf("This should be shown in stdout\n");
  fprintf(stderr, "This should be shown in stderr\n");
  assert(false);
}
void passing(void) {
  printf("This should not be shown\n");
}

int main(void) {
  flags_container flags = flags_init();

  int8_t* my_feature = flags_i8(&flags, "number", 'n', 0, "Add number to program");

  int err;
  if ((err = flags_parse(&flags, NULL)) != 0) {
    printf("Flag parse error: %s", flags_fprint_err(err));
    printf("%s", flags_usage(&flags));
  }

  test_context context = test_init();

  test_register(&context, "failing", &failing, true);
  test_register(&context, "passing", &passing, false);

  test_run(&context);

  test_deinit(&context);
}
