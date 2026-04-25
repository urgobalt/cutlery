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

  flags_string_list* skip = flags_multi_str(&flags, "skip", 's', "List of test names to skip");

  int err;
  if ((err = flags_parse(&flags, NULL, argc, argv)) != 0) {
    printf("Flag parse error:\n");
    for (size_t i = 0; i < flags.error_list.count; i += 1) {
      printf("%s", flags.error_list.content[i]);
    }
    printf("\n%s", flags_usage(&flags));
  }

  test_context context = test_init();

  test_register(&context, "expect_failing", &failing, true);
  test_register(&context, "failing", &failing, false);
  test_register(&context, "passing", &passing, false);

  test_skip(&context, "failing");

  for (size_t i = 0; i < skip->count; i += 1) {
    test_skip(&context, skip->content[i]);
  }

  test_run(&context);

  flags_deinit(&flags);
  test_deinit(&context);
}
