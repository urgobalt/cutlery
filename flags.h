#ifndef FLAGS_H
#define FLAGS_H

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

enum flags_type {
  FLAGS_EMPTY = 0,
  FLAGS_STR,
  FLAGS_STRLIST,
  FLAGS_BOOL,
  FLAGS_i8,
  FLAGS_i16,
  FLAGS_i32,
  FLAGS_i64,
  FLAGS_u8,
  FLAGS_u16,
  FLAGS_u32,
  FLAGS_u64,
};

typedef struct flags_string_list {
  struct flags_string_list* next;
  char* content;
} flags_string_list;

typedef struct flags_item {
  void* value;
  const char* name;
  const char* help;
  enum flags_type type;
  unsigned char short_name;
} flags_item;

typedef struct flags_container {
  // Content
  flags_item* items;
  flags_item** short_to_long_map;
  size_t count;
  size_t capacity;

  // Memory log
  flags_string_list** string_list_items;
  size_t string_list_count;
  size_t string_list_capacity;
} flags_container;

typedef struct argument_list {
  char** content;
  size_t count;
  size_t capacity;
} argument_list;

flags_container flags_init(void);
// Free all of the memory allocated by flags
void flags_deinit(flags_container* flags);

char* flags_fprint_err(int errcode);
char* flags_usage(flags_container* flags);

int flags_parse(flags_container* flags, argument_list* args, int argc, char* argv[]);

// Define a numeric flag of with the size of 1 byte / int8_t with default value
int8_t * flags_i8 (flags_container* flags, const char* name, unsigned char short_name, int8_t  value, const char* help);
// Define a numeric flag of with the size of 2 bytes / int16_t with default value
int16_t* flags_i16(flags_container* flags, const char* name, unsigned char short_name, int16_t value, const char* help);
// Define a numeric flag of with the size of 4 bytes / int32_t with default value
int32_t* flags_i32(flags_container* flags, const char* name, unsigned char short_name, int32_t value, const char* help);
// Define a numeric flag of with the size of 8 bytes / int64_t with default value
int64_t* flags_i64(flags_container* flags, const char* name, unsigned char short_name, int64_t value, const char* help);

// Define a numeric flag of with the size of 1 byte / uint8_t with default value
uint8_t * flags_u8 (flags_container* flags, const char* name, unsigned char short_name, uint8_t  value, const char* help);
// Define a numeric flag of with the size of 2 bytes / uint16_t with default value
uint16_t* flags_u16(flags_container* flags, const char* name, unsigned char short_name, uint16_t value, const char* help);
// Define a numeric flag of with the size of 4 bytes / uint32_t with default value
uint32_t* flags_u32(flags_container* flags, const char* name, unsigned char short_name, uint32_t value, const char* help);
// Define a numeric flag of with the size of 8 bytes / uint64_t with default value
uint64_t* flags_u64(flags_container* flags, const char* name, unsigned char short_name, uint64_t value, const char* help);

// Define a string flag with default value
char* flags_str(flags_container* flags, const char* name, unsigned char short_name, char* value, const char* help);

// Define a numeric flag of with the size of 1 byte / int8_t
int8_t * flags_required_i8 (flags_container* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 2 bytes / int16_t
int16_t* flags_required_i16(flags_container* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 4 bytes / int32_t
int32_t* flags_required_i32(flags_container* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 8 bytes / int64_t
int64_t* flags_required_i64(flags_container* flags, const char* name, unsigned char short_name, const char* help);

// Define a numeric flag of with the size of 1 byte / uint8_t
uint8_t * flags_required_u8 (flags_container* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 2 bytes / uint16_t
uint16_t* flags_required_u16(flags_container* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 4 bytes / uint32_t
uint32_t* flags_required_u32(flags_container* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 8 bytes / uint64_t
uint64_t* flags_required_u64(flags_container* flags, const char* name, unsigned char short_name, const char* help);

// Define a string flag
char* flags_required_str(flags_container* flags, const char* name, unsigned char short_name, const char* help);

// Define a boolean flag with default value
bool* flags_bool(flags_container* flags, const char* name, unsigned char short_name, bool value, const char* help);

// Defing a string list flag, meaning that if a user specify the flag more than
// once, the value is appended to the list.
flags_string_list* flags_strlist(flags_container* flags, const char* name, unsigned char short_name, const char* help);
// Iterates the flags string list
char* flags_next_string(flags_string_list* string_list);

#endif // FLAGS_H

#ifdef FLAGS_IMPLEMENTATION

// WARN: This capacity need to be a base of 2 in order to perform optimized
// modulus operation
const size_t initial_flags_capacity = 128;

// Hash a key using the very fast fnv-1a (Fowler-Noll-Vo 1a) non-cryptographic algorithm
inline size_t __flags_hash(const char* key) {
  const size_t offset_basis = 0xcbf29ce484222325;
  const size_t prime = 0x100000001b3;

  size_t accumulator = offset_basis;

  for (size_t i = 0; key[i] != '\0'; i += 1) {
    accumulator *= prime;
    accumulator ^= key[i];
  }

  return accumulator;
}

inline flags_container flags_init(void) {
  flags_container flags = {
    .items                = calloc(initial_flags_capacity, sizeof(flags_item)),
    // NOTE: Might consider doing this with a dynamic array instead of
    // allocating the maximum possible amount of items instantly.
    .short_to_long_map    = calloc(CHAR_MAX+1, sizeof(flags_item*)),
    .count                = 0,
    .capacity             = initial_flags_capacity,

    .string_list_items    = malloc(initial_flags_capacity / 4),
    .string_list_count    = 0,
    .string_list_capacity = initial_flags_capacity / 4,
  };

  assert(flags.items             != NULL);
  assert(flags.string_list_items != NULL);

  return flags;
}

inline void flags_deinit(flags_container* flags) {
  free(flags->items);

  // Free all the allocated string lists
  for (size_t i = 0; i < flags->string_list_count; i += 1) {
    flags_string_list* current_item = flags->string_list_items[i];
    while (current_item != NULL) {
      flags_string_list* next_item = current_item->next;
      free(current_item);
      current_item = next_item;
    }
  }
}

char* flags_fprint_err(int errcode) {
  (void)errcode;
  assert(false && "TODO: return an error msg");
}

char* flags_usage(flags_container* flags) {
  (void)flags;
  assert(false && "TODO: return the usage for the flags defined");
}

int flags_parse(flags_container* flags, argument_list* args, int argc, char* argv[]) {

  const unsigned char flag_marker = '-';
  for (int i = 0; i < argc; i += 1) {
    if (argv[i][0] == flag_marker) {
      if (argv[i][1] == flag_marker) {
        //
        size_t len  = 0;
        size_t j    = 2;
        char* val = NULL;
        while (argv[i][j] != '\0') {
          if (argv[i][j] == '=') {
            argv[i][j] = '\0';
            val = &argv[i][j+1];
            break;
          }
          len += 1;
          j += 1;
        }

        const char* name = malloc(len * sizeof(char));
      }
    } else {
      if (args == NULL) continue;
      if (args->count >= args->capacity) {
        args->capacity = args->capacity*2;
        args->content  = realloc(args->content, args->capacity);
        assert(args->content != NULL);
      }

      args->content[args->count] = argv[i];
      args->count += 1;
    }
  }

  return 0;
}

void __flags_realloc(flags_container* flags, size_t capacity) {
  (void)flags; (void) capacity;
  assert(false && "Please change the initial_capacity, by default you should never be able to reach that amount of flags in a program");
}

void* __flags_insert(flags_container* flags, const char* name, const unsigned char short_name, void* value, enum flags_type type, const char* help) {
  if (flags->count >= flags->capacity)
    __flags_realloc(flags, flags->capacity*2);

  flags_item* addr = NULL;
  size_t index = __flags_hash(name) & (flags->capacity - 1);
  for (size_t i = 0; i < flags->capacity; i += 1) {
    assert(flags->items[index].name == 0 || (strcmp(flags->items[index].name, name) == false && "Adding the same flag multiple times is not allowed."));
    if (flags->items[index].type == FLAGS_EMPTY) {
      flags->items[index] = (flags_item) {
        .value = value,
        .name = name,
        .short_name = short_name,
        .help = help,
        .type = type,
      };
      addr = &flags->items[index];
      break;
    }
    index = (index+1) & (flags->capacity - 1);
  }

  assert(addr != NULL);

  flags->short_to_long_map[short_name] = addr;
  return addr;
}

inline int8_t * flags_i8 (flags_container* flags, const char* name, unsigned char short_name, int8_t  value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i8, help);
}

inline int16_t* flags_i16(flags_container* flags, const char* name, unsigned char short_name, int16_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i16, help);
}

inline int32_t* flags_i32(flags_container* flags, const char* name, unsigned char short_name, int32_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i32, help);
}

inline int64_t* flags_i64(flags_container* flags, const char* name, unsigned char short_name, int64_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_i64, help);
}

inline uint8_t * flags_u8 (flags_container* flags, const char* name, unsigned char short_name, uint8_t  value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u8, help);
}

inline uint16_t* flags_u16(flags_container* flags, const char* name, unsigned char short_name, uint16_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u16, help);
}

inline uint32_t* flags_u32(flags_container* flags, const char* name, unsigned char short_name, uint32_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u32, help);
}

inline uint64_t* flags_u64(flags_container* flags, const char* name, unsigned char short_name, uint64_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_u64, help);
}

inline char* flags_str(flags_container* flags, const char* name, unsigned char short_name, char* value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_STR, help);
}

// TODO: Implmentation of required functionality

inline bool* flags_bool(flags_container* flags, const char* name, unsigned char short_name, bool value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_BOOL, help);
}

flags_string_list* flags_strlist(flags_container* flags, const char* name, unsigned char short_name, const char* help) {
  if (flags->string_list_count >= flags->string_list_capacity) {
    flags->string_list_capacity = flags->string_list_capacity*2;
    flags->string_list_items = realloc(flags->string_list_items, flags->string_list_capacity);
    assert(flags->string_list_items != NULL);
  }

  void* value = &flags->string_list_items[flags->string_list_count];
  flags->string_list_count += 1;
  return __flags_insert(flags, name, short_name, value, FLAGS_STRLIST, help);
}

#endif // FLAGS_IMPLEMENTATION
