#ifndef FLAGS_H
#define FLAGS_H

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

enum flags_error {
  FLAGS_SUCCESS = 0,
  FLAGS_ERROR_NOT_FOUND,
  FLAGS_ERROR_VALUE_NOT_PROVIDED,
  FLAGS_ERROR_NAN,
  FLAGS_ERROR_NUMBER_OUT_OF_RANGE,
  FLAGS_ERROR_NOT_A_BOOL,
};

enum flags_type {
  FLAGS_EMPTY = 0,
  FLAGS_STR,
  FLAGS_MULTI_STR,
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

  // Parsing
  char* error_msg;
  char** argv;
  int argc;
} flags_context;

typedef struct flags_string_list {
  char** content;
  size_t count;
  size_t capacity;
} flags_string_list;

void flags_init(flags_context* flags);
// Free all of the memory allocated by flags
void flags_deinit(flags_context* flags);

char* flags_fprint_err(flags_context* flags, int errcode);
char* flags_usage(flags_context* flags);

enum flags_error flags_parse(flags_context* flags, flags_string_list* args, const int argc, char* const* argv);

// Define a numeric flag of with the size of 1 byte / int8_t with default value
int8_t * flags_i8 (flags_context* flags, const char* name, unsigned char short_name, int8_t  value, const char* help);
// Define a numeric flag of with the size of 2 bytes / int16_t with default value
int16_t* flags_i16(flags_context* flags, const char* name, unsigned char short_name, int16_t value, const char* help);
// Define a numeric flag of with the size of 4 bytes / int32_t with default value
int32_t* flags_i32(flags_context* flags, const char* name, unsigned char short_name, int32_t value, const char* help);
// Define a numeric flag of with the size of 8 bytes / int64_t with default value
int64_t* flags_i64(flags_context* flags, const char* name, unsigned char short_name, int64_t value, const char* help);

// Define a numeric flag of with the size of 1 byte / uint8_t with default value
uint8_t * flags_u8 (flags_context* flags, const char* name, unsigned char short_name, uint8_t  value, const char* help);
// Define a numeric flag of with the size of 2 bytes / uint16_t with default value
uint16_t* flags_u16(flags_context* flags, const char* name, unsigned char short_name, uint16_t value, const char* help);
// Define a numeric flag of with the size of 4 bytes / uint32_t with default value
uint32_t* flags_u32(flags_context* flags, const char* name, unsigned char short_name, uint32_t value, const char* help);
// Define a numeric flag of with the size of 8 bytes / uint64_t with default value
uint64_t* flags_u64(flags_context* flags, const char* name, unsigned char short_name, uint64_t value, const char* help);

// Define a string flag with default value
char* flags_str(flags_context* flags, const char* name, unsigned char short_name, char* value, const char* help);

// Define a numeric flag of with the size of 1 byte / int8_t
int8_t * flags_required_i8 (flags_context* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 2 bytes / int16_t
int16_t* flags_required_i16(flags_context* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 4 bytes / int32_t
int32_t* flags_required_i32(flags_context* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 8 bytes / int64_t
int64_t* flags_required_i64(flags_context* flags, const char* name, unsigned char short_name, const char* help);

// Define a numeric flag of with the size of 1 byte / uint8_t
uint8_t * flags_required_u8 (flags_context* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 2 bytes / uint16_t
uint16_t* flags_required_u16(flags_context* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 4 bytes / uint32_t
uint32_t* flags_required_u32(flags_context* flags, const char* name, unsigned char short_name, const char* help);
// Define a numeric flag of with the size of 8 bytes / uint64_t
uint64_t* flags_required_u64(flags_context* flags, const char* name, unsigned char short_name, const char* help);

// Define a string flag
char* flags_required_str(flags_context* flags, const char* name, unsigned char short_name, const char* help);

// Define a boolean flag with default value
bool* flags_bool(flags_context* flags, const char* name, unsigned char short_name, bool value, const char* help);

// Define a flag that instead of redifining the flags value, appends it into a list
// WARN: If not flags are provided, then the content will be NULL
flags_string_list* flags_multi_str(flags_context* flags, const char* name, unsigned char short_name, const char* help);

#endif // FLAGS_H

#ifdef FLAGS_IMPLEMENTATION

// START OF VARIABLES

// WARN: This capacity need to be a base of 2 in order to perform optimized
// modulus operation
const size_t __flags_initial_flags_capacity = 128;
const size_t __flags_initial_string_list_capacity = 32;
const size_t __flags_error_msg_max_len = 160; // Double the size of a normal terminal

// START OF PRIVATE FUNCTIONS

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

void __flags_realloc(flags_context* flags, size_t capacity) {
  (void)flags; (void) capacity;
  assert(false && "Please change the initial_capacity, by default you should never be able to reach that amount of flags in a program");
}

void* __flags_insert(flags_context* flags, const char* name, const unsigned char short_name, void* value, enum flags_type type, const char* help) {
  if (flags->count >= flags->capacity)
    __flags_realloc(flags, flags->capacity*2);

  flags_item* addr = NULL;
  size_t index = __flags_hash(name) & (flags->capacity - 1);
  for (size_t i = 0; i < flags->capacity; i += 1) {
    // TODO: Create a proper crashing function to signal to the user that it
    // will crash with a certain message
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
  flags->count += 1;

  return &addr->value;
}

void __flags_string_list_append(flags_string_list* list, char* value) {
  // TODO: Handle comma-delimited string lists

  assert(list != NULL);

  if (list->count >= list->capacity) {
    list->capacity = list->capacity*2;
    if (list->capacity == 0)
      list->capacity = __flags_initial_string_list_capacity;
    list->content  = realloc(list->content, list->capacity);
    assert(list->content != NULL);
  }

  list->content[list->count] = value;
  list->count += 1;
}

enum flags_error __flags_parse_and_assign_number(flags_context* flags, flags_item* item, char* value, intmax_t min, uintmax_t max) {
  assert(item        != NULL);
  assert(item->value != NULL);

  if (value == NULL) {
    snprintf(flags->error_msg, __flags_error_msg_max_len, "Flag %s expects a number", item->name);
    return FLAGS_ERROR_VALUE_NOT_PROVIDED;
  }

  for (size_t i = 0; i < strlen(value); i += 1){
    if (!isdigit(value[i])) {
      snprintf(flags->error_msg, __flags_error_msg_max_len, "Flag %s expects a number, found: %s", item->name, value);
      return FLAGS_ERROR_NAN;
    }
  }
  long long number = atoll(value);
  if ((intmax_t)number < min || (uintmax_t)number > max) {
    return FLAGS_ERROR_NUMBER_OUT_OF_RANGE;
  }
  item->value = (void*)(uintptr_t)number;
  return FLAGS_SUCCESS;
}

// TODO: Fix how bool is interacted with, I need the information about bool much
// mroe easily accessible to simplify the code structure
enum flags_error __flags_parse_and_assign_bool(flags_item* item, char* value) {
  if ( value == NULL
    || strcmp(value, "true") == 0
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
  return FLAGS_ERROR_NOT_A_BOOL;
}

enum flags_error __flags_update(flags_context* flags, char* name, char* value) {
  size_t index = __flags_hash(name) & (flags->capacity - 1);
  for (size_t i = 0; i < flags->capacity; i += 1) {
    flags_item* item = &flags->items[index];
    if (item->type == FLAGS_EMPTY)
      return FLAGS_ERROR_NOT_FOUND;
    assert(item->name != NULL);
    if (strcmp(item->name, name) == 0) {
      switch (item->type) {
      case FLAGS_STR:
        if (value == NULL) {
          snprintf(flags->error_msg, __flags_error_msg_max_len, "Flag %s expects a string", item->name);
          return FLAGS_ERROR_VALUE_NOT_PROVIDED;
        }
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
      case FLAGS_MULTI_STR:
        if (value == NULL) {
          snprintf(flags->error_msg, __flags_error_msg_max_len, "Flag %s expects a string", item->name);
          return FLAGS_ERROR_VALUE_NOT_PROVIDED;
        }
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

// END OF PRIVATE FUNCTIONS

inline void flags_init(flags_context* flags) {
  *flags = (flags_context){
    .items                = calloc(__flags_initial_flags_capacity, sizeof(flags_item)),
    // NOTE: Might consider doing this with a dynamic array instead of
    // allocating the maximum possible amount of items instantly.
    .short_to_long_map    = calloc(CHAR_MAX+1, sizeof(flags_item*)),
    .count                = 0,
    .capacity             = __flags_initial_flags_capacity,

    // Error handling
    .error_msg            = malloc(__flags_error_msg_max_len * sizeof(char)),
  };

  assert(flags->items             != NULL);
  assert(flags->short_to_long_map != NULL);
  assert(flags->error_msg         != NULL);
}

inline void flags_deinit(flags_context* flags) {
  // Free all the allocated string lists
  for (size_t i = 0; i < flags->capacity; i += 1) {
    if (flags->items[i].type == FLAGS_MULTI_STR) {
      flags_string_list* value = flags->items[i].value;
      if (value->content != NULL)
        free(value->content);
      free(value);
    }
  }

  for (int i = 0; i < flags->argc; i += 1) {
    free(flags->argv[i]);
  }
  free(flags->argv);

  free(flags->items);
  free(flags->short_to_long_map);
  free(flags->error_msg);
}

char* flags_fprint_err(flags_context* flags, int errcode) {
  (void)flags; (void)errcode;
  assert(false && "TODO: return an error msg");
}

char* flags_usage(flags_context* flags) {
  (void)flags;
  assert(false && "TODO: return the usage for the flags defined");
}

enum flags_error flags_parse(flags_context* flags, flags_string_list* args, const int argc, char* const* argv) {

  flags->argc = argc;
  flags->argv = malloc(argc * sizeof(char**));
  assert(flags->argv != NULL);

  for (int i = 0; i < argc; i += 1) {
    // NOTE: We could possible get the length of the string here and store it
    // somewhere for later reuse (optimization)
    flags->argv[i] = strdup(argv[i]);
    assert(flags->argv[i] != NULL);
  }

  const unsigned char flag_marker = '-';

  for (int argument_index = 0; argument_index < argc; argument_index += 1) {

    char* name = flags->argv[argument_index];

    if (name[0] == flag_marker) {
      if (name[1] == flag_marker) {

        size_t character_index = 2;
        char* value = NULL;
        bool explicit_assignment = false;

        while (name[character_index] != '\0') {
          if (name[character_index] == '=') {
            name[character_index] = '\0';
            value = &name[character_index+1];
            explicit_assignment = true;
            break;
          }
          character_index += 1;
        }

        if (value == NULL && argument_index < argc) {
          argument_index += 1;
          value = flags->argv[argument_index];
        }

        enum flags_error error = __flags_update(flags, name+2, value);
        if (error != 0) {
          if (error == FLAGS_ERROR_NOT_A_BOOL && !explicit_assignment) {
            argument_index -= 1;
          } else {
            return error;
          }
        }
      } else {
        assert(false && "TODO: handle short flags");
      }
    } else {
      if (args == NULL) continue;
      __flags_string_list_append(args, flags->argv[argument_index]);
    }
  }

  return 0;
}

inline int8_t * flags_i8 (flags_context* flags, const char* name, unsigned char short_name, int8_t  value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i8, help);
}

inline int16_t* flags_i16(flags_context* flags, const char* name, unsigned char short_name, int16_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i16, help);
}

inline int32_t* flags_i32(flags_context* flags, const char* name, unsigned char short_name, int32_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i32, help);
}

inline int64_t* flags_i64(flags_context* flags, const char* name, unsigned char short_name, int64_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_i64, help);
}

inline uint8_t * flags_u8 (flags_context* flags, const char* name, unsigned char short_name, uint8_t  value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u8, help);
}

inline uint16_t* flags_u16(flags_context* flags, const char* name, unsigned char short_name, uint16_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u16, help);
}

inline uint32_t* flags_u32(flags_context* flags, const char* name, unsigned char short_name, uint32_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u32, help);
}

inline uint64_t* flags_u64(flags_context* flags, const char* name, unsigned char short_name, uint64_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_u64, help);
}

inline char* flags_str(flags_context* flags, const char* name, unsigned char short_name, char* value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_STR, help);
}

// TODO: Implmentation of required functionality

inline bool* flags_bool(flags_context* flags, const char* name, unsigned char short_name, bool value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_BOOL, help);
}

flags_string_list* flags_multi_str(flags_context* flags, const char* name, unsigned char short_name, const char* help) {
  flags_string_list* list = malloc(sizeof(flags_string_list));
  assert(list != NULL);
  *list = (flags_string_list){0};
  return *((flags_string_list**)__flags_insert(flags, name, short_name, list, FLAGS_MULTI_STR, help));
}

#endif // FLAGS_IMPLEMENTATION
