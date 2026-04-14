#ifndef TEST_H
#define TEST_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

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

test_result_condition *test_run(test_context *context) {
  test_result_condition *results = malloc(context->count * sizeof(test_result_condition));

  // TODO: run tests and register the results into the results variable'
  for (size_t i = 0; i < context->count; i += 1) {
    pid_t p = fork();
    if (p < 0) {
      results[i] = TEST_SUPERVISOR_ERROR;
      continue;
    } else if (p == 0) {
      context->tests[i]();
    } else {
      int status;
      pid_t rp = waitpid(p, &status, WUNTRACED);
      printf("%i\n", status);
    }
  }

  return results;
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
