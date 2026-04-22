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

int main(int argc, char* argv[]) {
  flags_context flags = {0};
  flags_init(&flags);

  int8_t* my_number = flags_i8(&flags, "number", 'n', 0, "Add number to program");
  // flags_string_list** skip = flags_strlist(&flags, "skip", 's', "List of test names to skip");

  int err;
  if ((err = flags_parse(&flags, NULL, argc, argv)) != 0) {
    printf("Flag parse error: %s", flags_fprint_err(&flags, err));
    printf("%s", flags_usage(&flags));
  }

  printf("number: %i\n", *my_number);
  // printf("string: %s\n", flags_next_string(skip));

  test_context context = test_init();

  test_register(&context, "expect_failing", &failing, true);
  test_register(&context, "failing", &failing, false);
  test_register(&context, "passing", &passing, false);

  test_skip(&context, "failing");

  test_run(&context);

  flags_deinit(&flags);
  test_deinit(&context);
}
