#ifndef FLAGS_H
#define FLAGS_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

typedef struct flags_string_list {
  struct flags_string_list* next;
  struct flags_string_list* prev;
  char* content;
} flags_string_list;

typedef struct flags_container {
  // Size
  size_t count;
  size_t capacity;

  // Flags content
  int8_t* values;
  const char** names;
  char* short_names;
  const char** help;

  // Memory log
  flags_string_list** string_list_list;
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

// Define a numeric flag of with the size of 1 byte / int8_t with default value
int8_t * flags_i8 (flags_container* flags, const char* name, char short_name, int8_t  value, char* help);
// Define a numeric flag of with the size of 2 bytes / int16_t with default value
int16_t* flags_i16(flags_container* flags, const char* name, char short_name, int16_t value, char* help);
// Define a numeric flag of with the size of 4 bytes / int32_t with default value
int32_t* flags_i32(flags_container* flags, const char* name, char short_name, int32_t value, char* help);
// Define a numeric flag of with the size of 8 bytes / int64_t with default value
int64_t* flags_i64(flags_container* flags, const char* name, char short_name, int64_t value, char* help);

// Define a numeric flag of with the size of 1 byte / uint8_t with default value
uint8_t * flags_u8 (flags_container* flags, const char* name, char short_name, uint8_t  value, char* help);
// Define a numeric flag of with the size of 2 bytes / uint16_t with default value
uint16_t* flags_u16(flags_container* flags, const char* name, char short_name, uint16_t value, char* help);
// Define a numeric flag of with the size of 4 bytes / uint32_t with default value
uint32_t* flags_u32(flags_container* flags, const char* name, char short_name, uint32_t value, char* help);
// Define a numeric flag of with the size of 8 bytes / uint64_t with default value
uint64_t* flags_u64(flags_container* flags, const char* name, char short_name, uint64_t value, char* help);

// Define a string flag with default value
char* flags_str(flags_container* flags, const char* name, char short_name, char* value, char* help);

// Define a numeric flag of with the size of 1 byte / int8_t
int8_t * flags_required_i8 (flags_container* flags, const char* name, char short_name, char* help);
// Define a numeric flag of with the size of 2 bytes / int16_t
int16_t* flags_required_i16(flags_container* flags, const char* name, char short_name, char* help);
// Define a numeric flag of with the size of 4 bytes / int32_t
int32_t* flags_required_i32(flags_container* flags, const char* name, char short_name, char* help);
// Define a numeric flag of with the size of 8 bytes / int64_t
int64_t* flags_required_i64(flags_container* flags, const char* name, char short_name, char* help);

// Define a numeric flag of with the size of 1 byte / uint8_t
uint8_t * flags_required_u8 (flags_container* flags, const char* name, char short_name, char* help);
// Define a numeric flag of with the size of 2 bytes / uint16_t
uint16_t* flags_required_u16(flags_container* flags, const char* name, char short_name, char* help);
// Define a numeric flag of with the size of 4 bytes / uint32_t
uint32_t* flags_required_u32(flags_container* flags, const char* name, char short_name, char* help);
// Define a numeric flag of with the size of 8 bytes / uint64_t
uint64_t* flags_required_u64(flags_container* flags, const char* name, char short_name, char* help);

// Define a string flag
char* flags_required_str(flags_container* flags, const char* name, char short_name, char* help);

// Define a boolean flag with default value
bool* flags_bool(flags_container* flags, const char* name, char short_name, bool value, char* help);

// Defing a string list flag, meaning that if a user specify the flag more than
// once, the value is appended to the list.
flags_string_list* flags_strlist(flags_container* flags, const char* name, char short_name, char* help);
// Iterates the flags string list
char* flags_next_string(flags_string_list* string_list);

#endif // FLAGS_H

#ifdef FLAGS_IMPLEMENTATION
const size_t initial_flags_capacity = 512;

inline flags_container flags_init(void) {
  flags_container flags = {
    .values       = malloc(initial_flags_capacity),
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

void* __flags_append(flags_container* flags, const char* name, const char short_name, void* value, int8_t bytes, char* help) {
  if (flags->count >= flags->capacity)
    __flags_realloc(flags, flags->capacity*2);

  flags->names[flags->count] = name;
  flags->short_names[flags->count] = short_name;
  flags->help[flags->count] = help;

  int8_t* addr = &flags->values[flags->count];
  addr[0] = bytes;
  memcpy(addr+1, value, bytes);

  flags->count += bytes+1;
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

inline char* flags_str(flags_container* flags, const char* name, char short_name, char* value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(char*), help);
}

inline bool* flags_bool(flags_container* flags, const char* name, char short_name, bool value, char* help) {
  return __flags_append(flags, name, short_name, &value, sizeof(bool), help);
}

flags_string_list* flags_strlist(flags_container* flags, const char* name, char short_name, char* help) {
  (void)flags; (void)name; (void)short_name; (void)help;
  assert(false && "TODO: implement strings lists");
}

#endif // FLAGS_IMPLEMENTATION
