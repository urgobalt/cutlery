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

void test_register(test_context *context, void (*test)(void));
void fail_test_register(test_context *context, void (*test)(void));

// Utility functions
void test_assert(bool condition, char *message);

#ifdef TEST_IMPLEMENTATION

test_context test_init(void) {
  const size_t initial_capacity = 32;
  void (**tests)(void) = malloc(initial_capacity * sizeof(void*));
  assert(tests != NULL);
  test_context context = {
    .tests = tests,
    .count = 0,
    .capacity = initial_capacity,
  };
  return context;
}

void __test_print_output(int fd);

test_result_condition *test_run(test_context *context) {
  test_result_condition *results = malloc(context->count * sizeof(test_result_condition));
  int *stdouts = malloc(context->count * sizeof(int));
  int *stderrs = malloc(context->count * sizeof(int));

  // TODO: run tests and register the results into the results variable'
  for (size_t i = 0; i < context->count; i += 1) {
    int res = mkdir("/tmp/cutlerytest", 0777);
    assert(res == 0 || errno == EEXIST);

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
      if (status == 0) {
        results[i] = TEST_SUCCESS;
      } else {
        results[i] = TEST_FAILED;
      }
    }
  }

  for (size_t i = 0; i < context->count; i += 1) {
    printf("[stdout of test]\n");
    __test_print_output(stdouts[i]);
    printf("[stderr of test]\n");
    __test_print_output(stderrs[i]);
  }

  rmdir("/tmp/cutlerytest");

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

void test_register(test_context *context, void (*test)(void)) {
  if (context->count >= context->capacity) {
    size_t new_capacity = context->capacity*2;
    void (**tests)(void) = realloc(context->tests, new_capacity);
    assert(tests != NULL);
    context->tests = tests;
    context->capacity = new_capacity;
  }

  context->tests[context->count] = test;
  context->count += 1;
}

// TODO: implement tests that is expected to fail
void fail_test_register(test_context *context, void (*test)(void)) {
  assert(false && "NOT YET IMPLEMENTED");
}

#endif // TEST_IMPLEMENTATION
#endif // TEST_H
