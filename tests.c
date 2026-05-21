#include <stdint.h>
#include <string.h>
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

  if (flags->error_code != 0) {
    fprintf(stderr, "Flag parse error:\n");
    for (size_t i = 0; i < flags->error_list.count; i += 1) {
      fprintf(stderr, "%s\n", flags->error_list.content[i]);
    }
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

  assert(flags.error_code == 0);

  assert(*feature == true);
  assert(*other   == false);
  assert(*other2  == false);
  assert(*tother  == true);
  assert(*tother2 == true);
  assert(*s       == true);
  assert(*d       == true);

  flags_deinit(&flags);
}

void integer_flags(void) {
  const size_t argc = 4;
  const char* argv[] = {"program", "--i8", "5", "--i32=9", "--i64=-128"};

  flags_context flags = {0};
  assert(flags_init(&flags) == 0);

  int8_t* i8 = flags_i8(&flags, "i8", '\0', 0, "");
  int32_t* i32 = flags_i32(&flags, "i32", '\0', 0, "");
  int64_t* i64 = flags_i64(&flags, "i64", '\0', 0, "");

  flags_parse_with_output(&flags, NULL, argc, (char**)argv);

  assert(flags.error_code == 0);

  assert(*i8 == 5);
  assert(*i32 == 9);
  assert(*i64 == -128);

  flags_deinit(&flags);
}

void unsigned_integer_flags(void) {
  const size_t argc = 4;
  const char* argv[] = {"program", "--u8", "5", "--u32=9"};

  flags_context flags = {0};
  assert(flags_init(&flags) == 0);

  uint8_t* u8 = flags_u8(&flags, "u8", '\0', 0, "");
  uint32_t* u32 = flags_u32(&flags, "u32", '\0', 0, "");

  flags_parse_with_output(&flags, NULL, argc, (char**)argv);

  assert(flags.error_code == 0);

  assert(*u8 == 5);
  assert(*u32 == 9);

  flags_deinit(&flags);
}

void string_flags(void) {
  const size_t argc = 7;
  const char* argv[] = {"program", "--str", "hello_world", "--streq=hello", "-s", "world", "-d=hhhh"};

  flags_context flags = {0};
  assert(flags_init(&flags) == 0);

  char** str = flags_str(&flags, "str", '\0', "", "");
  char** streq = flags_str(&flags, "streq", '\0', "", "");
  char** strs = flags_str(&flags, "strs", 's', "", "");
  char** strd = flags_str(&flags, "strd", 'd', "", "");

  flags_parse_with_output(&flags, NULL, argc, (char**)argv);

  assert(flags.error_code == 0);

  assert(strcmp(*str, "hello_world") == 0);
  assert(strcmp(*streq, "hello") == 0);
  assert(strcmp(*strs, "world") == 0);
  assert(strcmp(*strd, "hhhh") == 0);

  flags_deinit(&flags);
}

void multi_string_flags(void) {
  const size_t argc = 7;
  const char* argv[] = {"program", "--strings", "hello", "--strings=world", "-s", "again", "-s=there"};

  flags_context flags = {0};
  assert(flags_init(&flags) == 0);

  flags_string_list* strings = flags_multi_str(&flags, "strings", 's', "");

  flags_parse_with_output(&flags, NULL, argc, (char**)argv);

  assert(flags.error_code == 0);

  assert(strcmp(strings->content[0], "hello") == 0);
  assert(strcmp(strings->content[1], "world") == 0);
  assert(strcmp(strings->content[2], "again") == 0);
  assert(strcmp(strings->content[3], "there") == 0);

  flags_deinit(&flags);
}

int main(int argc, char* argv[]) {
  flags_context flags = {0};
  assert(flags_init(&flags) == 0);

  flags_string_list* skip = flags_multi_str(&flags, "skip", 's', "List of test names to skip");
  bool* meta_tests = flags_bool(&flags, "meta-tests", 'm', false, "Enable meta tests for the testing framework");

  flags_parse(&flags, NULL, argc, argv);
  if (flags.error_code != 0) {
    printf("Flag parse error:\n");
    for (size_t i = 0; i < flags.error_list.count; i += 1) {
      printf("%s\n", flags.error_list.content[i]);
    }
    printf("\n%s", flags_usage(&flags));
  }

  test_context context = test_init();

  // meta tests for testing framework
  if (*meta_tests) {
    test_register(&context, "expect_failing", &failing, true);
    test_register(&context, "failing", &failing, false);
    test_register(&context, "passing", &passing, false);

    test_skip(&context, "failing");
  }

  // tests for flags library
  test_register(&context, "bool_flags", &bool_flags, false);
  test_register(&context, "integer_flags", &integer_flags, false);
  test_register(&context, "unsigned_integer_flags", &unsigned_integer_flags, false);
  test_register(&context, "string_flags", &string_flags, false);
  test_register(&context, "multi_string_flags", &multi_string_flags, false);


  for (size_t i = 0; i < skip->count; i += 1) {
    test_skip(&context, skip->content[i]);
  }

  test_run(&context);

  flags_deinit(&flags);
  test_deinit(&context);
}
