#ifndef FLAGS_H
#define FLAGS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

typedef struct flags_container {
  uint8_t* values;
  const char** names;
  char* short_names;
  const char** help;
  size_t count;
  size_t capacity;
} flags_container;

typedef struct argument_list {
  char** content;
  size_t count;
} argument_list;

flags_container flags_init(void);
void flags_deinit(flags_container* flags);

char* flags_fprint_err(int errcode);
char* flags_usage(flags_container* flags);

int flags_parse(flags_container* flags, argument_list* args);

int8_t * flags_i8 (flags_container* flags, const char* name, char short_name, int8_t  value, char* help);
int16_t* flags_i16(flags_container* flags, const char* name, char short_name, int16_t value, char* help);
int32_t* flags_i32(flags_container* flags, const char* name, char short_name, int32_t value, char* help);
int64_t* flags_i64(flags_container* flags, const char* name, char short_name, int64_t value, char* help);

uint8_t * flags_u8 (flags_container* flags, const char* name, char short_name, uint8_t  value, char* help);
uint16_t* flags_u16(flags_container* flags, const char* name, char short_name, uint16_t value, char* help);
uint32_t* flags_u32(flags_container* flags, const char* name, char short_name, uint32_t value, char* help);
uint64_t* flags_u64(flags_container* flags, const char* name, char short_name, uint64_t value, char* help);

char** flags_str(flags_container* flags, const char* name, char short_name, char* value, char* help);
bool* flags_bool(flags_container* flags, const char* name, char short_name, bool value, char* help);

// TODO: List of strings functionality
// TODO: Add required flags functionality

#endif // FLAGS_H

#ifdef FLAGS_IMPLEMENTATION
const size_t initial_flags_capacity = 128;

inline flags_container flags_init(void) {
  flags_container flags = {
    .values       = malloc(initial_flags_capacity * sizeof(void*)),
    .names        = malloc(initial_flags_capacity * sizeof(char*)),
    .short_names  = malloc(initial_flags_capacity * sizeof(char)),
    .help         = malloc(initial_flags_capacity * sizeof(char*)),
    .count        = 0,
    .capacity     = 0,
  };

  assert(flags.values      != NULL);
  assert(flags.names       != NULL);
  assert(flags.short_names != NULL);
  assert(flags.help        != NULL);

  return flags;
}

inline void flags_deinit(flags_container* flags) {
  free(flags->values);
  free(flags->names);
  free(flags->short_names);
  free(flags->help);
}

char* flags_fprint_err(int errcode) {
  (void)errcode;
  assert(false && "TODO: return a error msg");
}

char* flags_usage(flags_container* flags) {
  (void)flags;
  assert(false && "TODO: return the usage for the flags defined");
}

int flags_parse(flags_container* flags, argument_list* args) {
  (void)flags; (void)args;
  assert(false && "TODO: parse the flags");
}

void __flags_realloc(flags_container* flags, size_t capacity) {
  (void)flags; (void) capacity;
  assert(false && "Please change the initial_capacity, by default you should never be able to reach that amount of flags in a program");
}

void* __flags_append(flags_container* flags, const char* name, const char short_name, void* value, size_t bytes, char* help) {
  if (flags->count >= flags->capacity)
    __flags_realloc(flags, flags->capacity*2);

  flags->names[flags->count] = name;
  flags->short_names[flags->count] = short_name;
  flags->help[flags->count] = help;

  void* addr = &flags->values[flags->count];
  memcpy(addr, value, bytes);

  flags->count += bytes;
  return addr;
}

inline int8_t * flags_i8 (flags_container* flags, const char* name, char short_name, int8_t  value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(int8_t), help);
}

inline int16_t* flags_i16(flags_container* flags, const char* name, char short_name, int16_t value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(int16_t), help);
}

inline int32_t* flags_i32(flags_container* flags, const char* name, char short_name, int32_t value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(int32_t), help);
}

inline int64_t* flags_i64(flags_container* flags, const char* name, char short_name, int64_t value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(int64_t), help);
}

inline uint8_t * flags_u8 (flags_container* flags, const char* name, char short_name, uint8_t  value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(uint8_t), help);
}

inline uint16_t* flags_u16(flags_container* flags, const char* name, char short_name, uint16_t value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(uint16_t), help);
}

inline uint32_t* flags_u32(flags_container* flags, const char* name, char short_name, uint32_t value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(uint32_t), help);
}

inline uint64_t* flags_u64(flags_container* flags, const char* name, char short_name, uint64_t value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(uint64_t), help);
}

inline char** flags_str(flags_container* flags, const char* name, char short_name, char* value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(char*), help);
}

inline bool* flags_bool(flags_container* flags, const char* name, char short_name, bool value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(bool), help);
}

#endif // FLAGS_IMPLEMENTATION
