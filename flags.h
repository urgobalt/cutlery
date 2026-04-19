#ifndef FLAGS_H
#define FLAGS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

typedef struct flags_container {
  uint8_t* values;
  char** names;
  char* short_names;
  size_t count;
  size_t capacity;
} flags_container;

typedef struct argument_list {
  char** content;
  size_t count;
} argument_list;

flags_container flags_init(void);
void flags_deinit(flags_container* flags);
int flags_parse(flags_container* flags, argument_list* args);
char* flags_fprint_err(int errcode);

int8_t * flags_i8 (flags_container* flags, char* name, char short_name, int8_t  value, char* help);
int16_t* flags_i16(flags_container* flags, char* name, char short_name, int16_t value, char* help);
int32_t* flags_i32(flags_container* flags, char* name, char short_name, int32_t value, char* help);
int64_t* flags_i64(flags_container* flags, char* name, char short_name, int64_t value, char* help);

uint8_t * flags_u8 (flags_container* flags, char* name, char short_name, uint8_t  value, char* help);
uint16_t* flags_u16(flags_container* flags, char* name, char short_name, uint16_t value, char* help);
uint32_t* flags_u32(flags_container* flags, char* name, char short_name, uint32_t value, char* help);
uint64_t* flags_u64(flags_container* flags, char* name, char short_name, uint64_t value, char* help);

void flags_var_i8 (flags_container* flags, int8_t * var, char* name, char short_name, int8_t  value, char* help);
void flags_var_i16(flags_container* flags, int16_t* var, char* name, char short_name, int16_t value, char* help);
void flags_var_i32(flags_container* flags, int32_t* var, char* name, char short_name, int32_t value, char* help);
void flags_var_i64(flags_container* flags, int64_t* var, char* name, char short_name, int64_t value, char* help);

void flags_var_u8 (flags_container* flags, uint8_t * var, char* name, char short_name, uint8_t  value, char* help);
void flags_var_u16(flags_container* flags, uint16_t* var, char* name, char short_name, uint16_t value, char* help);
void flags_var_u32(flags_container* flags, uint32_t* var, char* name, char short_name, uint32_t value, char* help);
void flags_var_u64(flags_container* flags, uint64_t* var, char* name, char short_name, uint64_t value, char* help);

char** flags_str(flags_container* flags, char* name, char short_name, char* value, char* help);
bool* flags_bool(flags_container* flags, char* name, char short_name, bool value, char* help);

#endif // FLAGS_H

#ifdef FLAGS_IMPLEMENTATION
const size_t initial_flags_capacity = 256;

inline flags_container flags_init(void) {
  flags_container flags = {
    .values = malloc(initial_flags_capacity * sizeof(void*)),
    .names  = malloc(initial_flags_capacity * sizeof(char*)),
    .short_names  = malloc(initial_flags_capacity * sizeof(char)),
    .count = 0,
    .capacity = 0,
  };

  assert(flags.values      != NULL);
  assert(flags.names       != NULL);
  assert(flags.short_names != NULL);

  return flags;
}

inline void flags_deinit(flags_container* flags) {
  free(flags->values);
  free(flags->names);
  free(flags->short_names);
}

int flags_parse(flags_container* flags, argument_list* args) {
  (void)flags; (void)args;
  assert(false && "TODO: parse the flags");
}

#endif // FLAGS_IMPLEMENTATION
