#ifndef TEST_H
#define TEST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct test_context {
  void (**tests)(struct test_context *context);
  uintmax_t count;
  uintmax_t capacity;
} test_context;

// Supervisor
test_context test_init(void);
int32_t test_run(test_context *context);

void test_register(test_context *context, void (*test)(test_context *context));
void fail_test_register(test_context *context, void (*test)(test_context *context));

// Utility functions
void test_assert(bool condition, char *message);

#ifdef TEST_IMPLEMENTATION

test_context test_init(void) {
  const uintmax_t initial_capacity = 32;
  void (**tests)(struct test_context *context) = malloc(initial_capacity * sizeof(void*));
  test_context context = {
    .tests = tests,
    .count = 0,
    .capacity = initial_capacity,
  };
  return context;
}
int32_t test_run(test_context *context) { }

void test_register(test_context *context, void (*test)(test_context *context)) {}
void fail_test_register(test_context *context, void (*test)(test_context *context)) {}

#endif // TEST_IMPLEMENTATION
#endif // TEST_H
