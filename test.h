#ifndef TEST_H
#define TEST_H

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <execinfo.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef struct test_context {
  void (**tests)(void);
  char **test_names;
  bool *should_fail;
  size_t count;
  size_t capacity;
} test_context;

typedef enum test_result_condition {
  TEST_SUCCESS = 0,
  TEST_FAILED = 1,
  TEST_SKIPPED = 2,
  TEST_SUPERVISOR_ERROR = 99,
} test_result_condition;

// Supervisor
test_context test_init(void);
test_result_condition *test_run(test_context *context);

void test_register(test_context *context, char* name, void (*test)(void), bool should_fail);
void fail_test_register(test_context *context, void (*test)(void));

// Utility functions
void test_assert(bool condition, char *message);

#endif // TEST_H

#ifdef TEST_IMPLEMENTATION

test_context test_init(void) {
  const size_t initial_capacity = 32;

  test_context context = {
    .tests       = malloc(initial_capacity * sizeof(void*)),
    .test_names  = malloc(initial_capacity * sizeof(char*)),
    .should_fail = malloc(initial_capacity * sizeof(bool)),
    .count       = 0,
    .capacity    = initial_capacity,
  };
  assert(context.tests       != NULL);
  assert(context.test_names  != NULL);
  assert(context.should_fail != NULL);
  return context;
}

void __test_print_output(int fd);

test_result_condition *test_run(test_context *context) {
  test_result_condition *results = malloc(context->count * sizeof(test_result_condition));
  int *stdouts = malloc(context->count * sizeof(int));
  int *stderrs = malloc(context->count * sizeof(int));

  int res = mkdir("/tmp/cutlerytest", 0777);
  assert(res == 0 || errno == EEXIST);

  size_t success_count = 0;

  for (size_t i = 0; i < context->count; i += 1) {
    printf("%s => ", context->test_names[i]);
    fflush(stdout);

    // TODO: Implement skip functionality

    char templ[] = "/tmp/cutlerytest/testoutputXXXXXX";
    int stdout_fd = mkstemp(templ);

    char templ_err[] = "/tmp/cutlerytest/testerrXXXXXX";
    int stderr_fd = mkstemp(templ_err);

    assert(stdout_fd >= 0);
    assert(stderr_fd >= 0);

    stdouts[i] = stdout_fd;
    stderrs[i] = stderr_fd;

    pid_t test_pid = fork();
    assert(test_pid >= 0);
    if (test_pid == 0) {
      dup2(stdout_fd, STDOUT_FILENO);
      dup2(stderr_fd, STDERR_FILENO);

      context->tests[i]();
      exit(0);
    } else {
      int status;
      (void)waitpid(test_pid, &status, WUNTRACED);
      bool has_failed = status != 0;
      if (has_failed == context->should_fail[i]) {
        results[i] = TEST_SUCCESS;
        success_count += 1;
        printf("\x1b[1;32mPASS\x1b[0m\n");
      } else {
        results[i] = TEST_FAILED;
        printf("\x1b[1;31mFAIL\x1b[0m\n");
      }
      fflush(stdout);
    }
  }

  // TODO: List skipped tests
  printf("\n"
          "=============\n"
          "Completed  = %zu\n"
          "Passed     = %zu\n"
          "Failed     = %zu\n"
          "=============\n\n",
        context->count,
        success_count,
        context->count - success_count);

  for (size_t i = 0; i < context->count; i += 1) {
    if (results[i] != TEST_FAILED) continue;
    printf("Output from test '%s':\n", context->test_names[i]);
    printf("[stdout]\n");
    __test_print_output(stdouts[i]);
    printf("[stderr]\n");
    __test_print_output(stderrs[i]);
  }

  return results;
}

void __test_print_output(int fd) {
  lseek(fd, 0, SEEK_SET);

  const size_t buffer_size = 128;
  char buffer[buffer_size];

  while(read(fd, buffer, buffer_size) != 0) {
    printf("%s", buffer);
  }

  printf("\n");
}

void test_register(test_context *context, char* name, void (*test)(void), bool should_fail) {
  if (context->count >= context->capacity) {
    context->capacity = context->capacity*2;
    context->tests = realloc(context->tests, context->capacity * sizeof(test));
    context->test_names = realloc(context->test_names, context->capacity * sizeof(name));
    context->should_fail = realloc(context->should_fail, context->capacity * sizeof(should_fail));

    assert(context->tests != NULL);
    assert(context->test_names != NULL);
  }

  context->tests[context->count] = test;
  context->test_names[context->count] = name;
  context->should_fail[context->count] = should_fail;
  context->count += 1;
}

#endif // TEST_IMPLEMENTATION
