#ifndef FLAGS_H
#define FLAGS_H

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

enum flags_error {
  FLAGS_SUCCESS = 0,
  FLAGS_ERROR_NOT_FOUND,
  FLAGS_NAN,
  FLAGS_NUMBER_OUT_OF_RANGE,
  FLAGS_NOT_A_BOOL,
};

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

  // Error handling
  char* error_msg;
} flags_container;

typedef struct argument_list {
  char** content;
  size_t count;
  size_t capacity;
} argument_list;

flags_container flags_init(void);
// Free all of the memory allocated by flags
void flags_deinit(flags_container* flags);

char* flags_fprint_err(flags_container* flags, int errcode);
char* flags_usage(flags_container* flags);

// WARN: This function will modify the content of the argv parameter
enum flags_error flags_parse(flags_container* flags, argument_list* args, int argc, char* argv[]);

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
const size_t __flags_initial_flags_capacity = 128;
const size_t __flags_error_msg_max_len = 160; // Double the size of a normal terminal


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
    .items                = calloc(__flags_initial_flags_capacity, sizeof(flags_item)),
    // NOTE: Might consider doing this with a dynamic array instead of
    // allocating the maximum possible amount of items instantly.
    .short_to_long_map    = calloc(CHAR_MAX+1, sizeof(flags_item*)),
    .count                = 0,
    .capacity             = __flags_initial_flags_capacity,

    .string_list_items    = calloc(__flags_initial_flags_capacity / 4, sizeof(flags_string_list*)),
    .string_list_count    = 0,
    .string_list_capacity = __flags_initial_flags_capacity / 4,

    .error_msg            = malloc(__flags_error_msg_max_len * sizeof(char)),
  };

  assert(flags.items             != NULL);
  assert(flags.short_to_long_map != NULL);
  assert(flags.string_list_items != NULL);
  assert(flags.error_msg         != NULL);

  return flags;
}

inline void flags_deinit(flags_container* flags) {
  free(flags->items);
  free(flags->short_to_long_map);
  free(flags->error_msg);

  // Free all the allocated string lists
  for (size_t i = 0; i < flags->string_list_count; i += 1) {
    flags_string_list* current_item = flags->string_list_items[i];
    while (current_item != NULL) {
      flags_string_list* next_item = current_item->next;
      free(current_item);
      current_item = next_item;
    }
  }
  free(flags->string_list_items);
}

char* flags_fprint_err(flags_container* flags, int errcode) {
  (void)flags; (void)errcode;
  assert(false && "TODO: return an error msg");
}

char* flags_usage(flags_container* flags) {
  (void)flags;
  assert(false && "TODO: return the usage for the flags defined");
}

void __flags_string_list_append(flags_string_list* list, char* value) {
  // TODO: Handle comma-delimited string lists

  flags_string_list* current = list;
  while (list->next != NULL)
    current = current->next;

  current->next = malloc(sizeof(flags_string_list));
  assert(current->next == NULL);
  *current->next = (flags_string_list){
    .next = NULL,
    .content = value,
  };
}

enum flags_error __flags_parse_and_assign_number(flags_container* flags, flags_item* item, char* value, intmax_t min, uintmax_t max) {
  for (size_t i = 0; i < strnlen(value, 0xFFFFFFFFFFFFFFFF); i += 1){
    if (!isdigit(value[i])) {
      snprintf(flags->error_msg, __flags_error_msg_max_len, "Flag %s expects a number, found: %s", item->name, value);
      return FLAGS_NAN;
    }
  }
  long long number = atoll(value);
  if ((intmax_t)number < min || (uintmax_t)number > max) {
    return FLAGS_NUMBER_OUT_OF_RANGE;
  }
  item->value = (void*)(uintptr_t)number;
  return FLAGS_SUCCESS;
}

enum flags_error __flags_parse_and_assign_bool(flags_item* item, char* value) {
  if ( strcmp(value, "true") == 0
    || strcmp(value, "TRUE") == 0
    || strcmp(value, "1") == 0) {
    item->value = (void*)true;
    return FLAGS_SUCCESS;
  }

  if ( strcmp(value, "false") == 0
    || strcmp(value, "FALSE") == 0
    || strcmp(value, "0") == 0) {
    item->value = (void*)false;
    return FLAGS_SUCCESS;
  }

  item->value = (void*)true;
  return FLAGS_NOT_A_BOOL;
}

enum flags_error __flags_update(flags_container* flags, char* name, char* value) {
  size_t index = __flags_hash(name) & (flags->capacity - 1);
  for (size_t i = 0; i < flags->capacity; i += 1) {
    flags_item* item = &flags->items[index];
    printf("name: %s\n", name);
    printf("type: %i\n", item->type);
    if (item->type == FLAGS_EMPTY)
      return FLAGS_ERROR_NOT_FOUND;
    assert(item->name != NULL);
    if (strcmp(item->name, name) == 0) {
      switch (item->type) {
      case FLAGS_STR:
        item->value = value;
        return FLAGS_SUCCESS;
      case FLAGS_i8:
        return __flags_parse_and_assign_number(flags, item, value, INT8_MIN, INT8_MAX);
      case FLAGS_i16:
        return __flags_parse_and_assign_number(flags, item, value, INT16_MIN, INT16_MAX);
      case FLAGS_i32:
        return __flags_parse_and_assign_number(flags, item, value, INT32_MIN, INT32_MAX);
      case FLAGS_i64:
        return __flags_parse_and_assign_number(flags, item, value, INT64_MIN, INT64_MAX);
      case FLAGS_u8:
        return __flags_parse_and_assign_number(flags, item, value, 0, UINT8_MAX);
      case FLAGS_u16:
        return __flags_parse_and_assign_number(flags, item, value, 0, UINT16_MAX);
      case FLAGS_u32:
        return __flags_parse_and_assign_number(flags, item, value, 0, UINT32_MAX);
      case FLAGS_u64:
        return __flags_parse_and_assign_number(flags, item, value, 0, UINT64_MAX);
      case FLAGS_STRLIST:
        __flags_string_list_append(item->value, value);
        return FLAGS_SUCCESS;
      case FLAGS_BOOL:
        return __flags_parse_and_assign_bool(item, value);
      case FLAGS_EMPTY:
        assert(false && "Unreachable");
      }
    }
    index = (index+1) & (flags->capacity - 1);
  }
  return FLAGS_ERROR_NOT_FOUND;
}

enum flags_error flags_parse(flags_container* flags, argument_list* args, int argc, char* argv[]) {

  const unsigned char flag_marker = '-';

  for (int argument_index = 0; argument_index < argc; argument_index += 1) {

    char* name = argv[argument_index];

    if (name[0] == flag_marker) {
      if (name[1] == flag_marker) {

        size_t character_index = 2;
        char* value = NULL;

        while (name[character_index] != '\0') {
          if (name[character_index] == '=') {
            name[character_index] = '\0';
            value = &name[character_index+1];
            break;
          }
          character_index += 1;
        }

        if (value == NULL) {
          argument_index += 1;
          value = argv[argument_index];
        }

        enum flags_error error;
        // TODO: Handle anonymous value for boolean flags
        if ((error = __flags_update(flags, name+2, value)) != 0) {
          return error;
        }
      } else {
        assert(false && "TODO: handle short flags");
      }
    } else {
      if (args == NULL) continue;
      if (args->count >= args->capacity) {
        args->capacity = args->capacity*2;
        args->content  = realloc(args->content, args->capacity);
        assert(args->content != NULL);
      }

      args->content[args->count] = argv[argument_index];
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
    assert(flags->items[index].name == NULL || (strcmp(flags->items[index].name, name) == 0 && "Adding the same flag multiple times is not allowed."));
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
