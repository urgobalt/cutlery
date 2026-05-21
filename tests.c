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

void flags_parse_with_output(flags_context* flags, flags_string_list* args, const int argc, char* const* argv) {
  flags_parse(flags, args, argc, argv);

  fprintf(stderr, "Flag parse error:\n");
  for (size_t i = 0; i < flags->error_list.count; i += 1) {
    fprintf(stderr, "%s\n", flags->error_list.content[i]);
  }
}

void bool_flags(void) {
  const size_t argc = 7;
  const char* args[] = { "program", "--enable-feature", "--other=false", "--other2=0", "--tother=true", "--tother2=1", "-sd" };

  flags_context flags = {0};
  assert(flags_init(&flags) == 0);

  bool* feature = flags_bool(&flags, "enable-feature", '\0', false, "");
  bool* other   = flags_bool(&flags, "other", '\0', false, "");
  bool* other2  = flags_bool(&flags, "other2", '\0', false, "");
  bool* tother  = flags_bool(&flags, "tother", '\0', false, "");
  bool* tother2 = flags_bool(&flags, "tother2", '\0', false, "");
  bool* s       = flags_bool(&flags, "some-other-feature", 's', false, "");
  bool* d       = flags_bool(&flags, "dome-other-feature", 'd', false, "");

  flags_parse_with_output(&flags, NULL, argc, (char**)args);

  assert(*feature == true);
  assert(*other   == false);
  assert(*other2  == false);
  assert(*tother  == true);
  assert(*tother2 == true);
  assert(*s       == true);
  assert(*d       == true);

  flags_deinit(&flags);
}

int main(int argc, char* argv[]) {
  flags_context flags = {0};
  assert(flags_init(&flags) == 0);

  flags_string_list* skip = flags_multi_str(&flags, "skip", 's', "List of test names to skip");

  int err;
  if ((err = flags_parse(&flags, NULL, argc, argv)) != 0) {
    printf("Flag parse error:\n");
    for (size_t i = 0; i < flags.error_list.count; i += 1) {
      printf("%s\n", flags.error_list.content[i]);
    }
    printf("\n%s", flags_usage(&flags));
  }

  test_context context = test_init();

  test_register(&context, "expect_failing", &failing, true);
  test_register(&context, "failing", &failing, false);
  test_register(&context, "passing", &passing, false);

  test_register(&context, "bool_flags", &bool_flags, false);

  test_skip(&context, "failing");

  for (size_t i = 0; i < skip->count; i += 1) {
    test_skip(&context, skip->content[i]);
  }

  test_run(&context);

  flags_deinit(&flags);
  test_deinit(&context);
}
