#ifndef FLAGS_H_
#define FLAGS_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

enum flags_error {
  FLAGS_SUCCESS = 0,
  FLAGS_ERROR_UNKNOWN_FLAG = 0x1,
  FLAGS_ERROR_VALUE_NOT_PROVIDED = 0x2,
  FLAGS_ERROR_NAN = 0x4,
  FLAGS_ERROR_NUMBER_OUT_OF_RANGE = 0x8,
  FLAGS_ERROR_NOT_A_BOOL = 0x16,
  FLAGS_ERROR_LIBRARY_IMPLEMENTATION = 0x64,
  FLAGS_ERROR_ALLOCATION = 0x128,
  FLAGS_ERROR_INVALID_NAME = 0x256,
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

typedef struct flags_string_list {
  char** content;
  size_t count;
  size_t capacity;
} flags_string_list;

typedef struct flags_item {
  void* value;
  const char* name;
  const char* help;
  const char* type_hint;
  enum flags_type type;
  unsigned char short_name;
  bool is_assigned;
} flags_item;

typedef struct flags_context {
  // Content
  flags_item* items;
  flags_item** short_to_long_map;
  size_t count;
  size_t capacity;

  // Parsing
  flags_string_list error_list;
  char** argv;
  uint8_t error_code;
  int argc;
} flags_context;

// Initilize the flags context.
// Returns 0 on success and -1 if on an allocation failure.
inline int flags_init(flags_context* flags);
// Free all of the memory allocated by flags
void flags_deinit(flags_context* flags);

char* flags_print_usage(flags_context* flags);

// The function assumes that `flags_init` has been executed before parsing.
// `argv` is copied into an internal structure to avoid writeability problems
// and undefined mutation of the external arguments.
//
// Error reporting is done by setting betwise flags using `flags_context` member
// `error_code`. Error messages is also stored within `flags_context` member
// `error_list` which is a list of strings.
//
// Please refer to the tests for example usage.
void flags_parse(flags_context* flags, flags_string_list* args, const int argc, char* const* argv);

// Check whether a flag was used by the user. This is the preferred way of
// implementing a required flag since it also enables more complex logic.
bool flags_used(flags_context* flags, const char* name);

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
char** flags_str(flags_context* flags, const char* name, unsigned char short_name, char* value, const char* help);

// Define a boolean flag with default value
bool* flags_bool(flags_context* flags, const char* name, unsigned char short_name, bool value, const char* help);

// Define a flag that instead of redifining the flags value, appends it into a list
// WARN: If not flags are provided, then the content will be NULL
flags_string_list* flags_multi_str(flags_context* flags, const char* name, unsigned char short_name, const char* help);

#endif // FLAGS_H_

#ifdef FLAGS_IMPLEMENTATION

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>

// START OF VARIABLES

// WARN: This capacity need to be a base of 2 in order to perform optimized
// modulus operation
const size_t __flags_initial_flags_capacity = 128;
const size_t __flags_initial_string_list_capacity = 32;
const size_t __flags_error_msg_max_len = 160; // Double the size of a normal terminal

// START OF PRIVATE FUNCTIONS

// Hash a key using the very fast fnv-1a (Fowler-Noll-Vo 1a) non-cryptographic algorithm
static inline size_t __flags_hash(const char* __restrict key) {
  const size_t offset_basis = 0xcbf29ce484222325;
  const size_t prime = 0x100000001b3;

  size_t accumulator = offset_basis;

  for (size_t i = 0; key[i] != '\0'; i += 1) {
    accumulator *= prime;
    accumulator ^= key[i];
  }

  return accumulator;
}

static void __flags_realloc(flags_context* __restrict flags, size_t capacity) {
  (void)flags; (void) capacity;
  // TODO: Implement realloc of the flags context
  abort(); // Please change the initial_capacity, by default you should never be able to reach that amount of flags in a program
}

bool __flags_validate_character(const char character) {
  char* special_characters = "-_+?";

  while (*special_characters != '\0') {
    if (*special_characters == character)
      return true;

    special_characters += 1;
  }

  if ((character >= 'A' && character <= 'Z') || (character >= 'a' && character <= 'z'))
    return true;

  return false;
}

static void __flags_string_list_append(flags_string_list* __restrict list, char* __restrict value) {
  if (list->count >= list->capacity) {
    list->capacity = list->capacity*2;
    if (list->capacity == 0)
      list->capacity = __flags_initial_string_list_capacity;
    list->content  = realloc(list->content, list->capacity * sizeof(char**));
  }

  list->content[list->count] = value;
  list->count += 1;
}

// Append an error onto the flag context's error list
static int __flags_append_err(flags_context* flags, const char* __restrict format, ...) {
    va_list args, args_copy;
    va_start(args, format);
    va_copy(args_copy, args);

    int needed = vsnprintf(NULL, 0, format, args);
    char* buffer = malloc(needed + 1);

    if (buffer == NULL) {
        va_end(args);
        va_end(args_copy);
        return -1;
    }

    vsprintf(buffer, format, args_copy);

    va_end(args_copy);
    va_end(args);

    __flags_string_list_append(&flags->error_list, buffer);

    return needed;
}

void* __flags_insert(flags_context* flags, const char* name, const unsigned char short_name, void* __restrict value, enum flags_type type, const char* type_hint, const char* help) {
  if (flags->count >= flags->capacity)
    __flags_realloc(flags, flags->capacity*2);

  for (size_t i = 0; name[i] != '\0'; i++) {
    if (__flags_validate_character(*name) == false) {
      __flags_append_err(flags, "Unexpected character '%c' in flag name '%s'", name[i], name);
      flags->error_code |= FLAGS_ERROR_INVALID_NAME;
      return NULL;
    }
  }

  // TODO: Enable the following sanitization when null handling of short_name is complete

  // if (__flags_validate_character(short_name) == false) {
  //   __flags_append_err(flags, "Unexpected character '%c' used as short_name for a flag", short_name);
  //   flags->error_code |= FLAGS_ERROR_INVALID_NAME;
  //   return NULL;
  // }

  flags_item* addr = NULL;
  size_t index = __flags_hash(name) & (flags->capacity - 1);
  for (size_t i = 0; i < flags->capacity; i += 1) {
    if (flags->items[index].type == FLAGS_EMPTY) {
      addr = &flags->items[index];
      break;
    }

    index = (index+1) & (flags->capacity - 1);
  }

  if (addr == NULL)
    return NULL;

  *addr = (flags_item) {
    .value = value,
    .name = name,
    .short_name = short_name,
    .help = help,
    .type = type,
    .type_hint = type_hint,
  };

  flags->short_to_long_map[short_name] = addr;
  flags->count += 1;

  return &addr->value;
}

static void __flags_parse_and_assign_number(flags_context* flags, flags_item* item, const char* value, intmax_t min, intmax_t max) {
  errno = 0;
  intmax_t number = strtoimax(value, NULL, 10);

  if (number < min || number > max || errno == ERANGE) {
    __flags_append_err(flags, "--%s|-%c expects a number between %" PRIiMAX " and " PRIiMAX ", found " PRIiMAX, item->name, item->short_name, min, max, number);
    flags->error_code |= FLAGS_ERROR_NUMBER_OUT_OF_RANGE;
    return;
  } else if (errno == EINVAL) {
    __flags_append_err(flags, "Not a number. Expected: --%s|-%c <number>", item->name, value);
    flags->error_code |= FLAGS_ERROR_NAN;
    return;
  }

  item->value = (void*)(uintptr_t)number;
}

static void __flags_parse_and_assign_unsigned_number(flags_context* flags, flags_item* item, const char* value, uintmax_t max) {
  errno = 0;
  uintmax_t number = strtoumax(value, NULL, 10);

  if (number > max || errno == ERANGE) {
    __flags_append_err(flags, "--%s|-%c expects a number below %" PRIuMAX ", found " PRIuMAX, item->name, item->short_name, max, number);
    flags->error_code |= FLAGS_ERROR_NUMBER_OUT_OF_RANGE;
    return;
  } else if (errno == EINVAL) {
    __flags_append_err(flags, "Not a number. Expected: --%s|-%c <number>", item->name, value);
    flags->error_code |= FLAGS_ERROR_NAN;
    return;
  }

  item->value = (void*)(uintptr_t)number;
}

// Returns 1 if true, 0 if false, and -1 if unparsable
static int __flags_parse_bool(const char* value) {
  if ( strcmp(value, "true") == 0
    || strcmp(value, "TRUE") == 0
    || strcmp(value, "1") == 0) {
    return 1;
  }

  if ( strcmp(value, "false") == 0
    || strcmp(value, "FALSE") == 0
    || strcmp(value, "0") == 0) {
    return 0;
  }

  return -1;
}

// TODO: Consolidate this with the __flags_get_addr_from_item
static flags_item* __flags_get_item(flags_context* flags, const char* name) {
  size_t index = __flags_hash(name) & (flags->capacity - 1);
  for (size_t i = 0; i < flags->capacity; i += 1) {
    flags_item* item = &flags->items[index];

    if (item->type == FLAGS_EMPTY) {
      return NULL;
    }

    if (strcmp(item->name, name) == 0) {
      return item;
    }

    index = (index+1) & (flags->capacity - 1);
  }

  return NULL;
}

static flags_item* __flags_get_item_from_short(flags_context* flags, const char short_name) {
  flags_item* item = flags->short_to_long_map[(int)short_name];

  if (item->type == FLAGS_EMPTY) {
    return NULL;
  }

  return item;
}

// WARN: This function does not handle boolean flags
static inline void __flags_update(flags_context* flags, flags_item* flag, char* value) {
  switch (flag->type) {
  case FLAGS_STR:
    flag->value = value;
    return;
  case FLAGS_i8:
    __flags_parse_and_assign_number(flags, flag, value, INT8_MIN, INT8_MAX);
    return;
  case FLAGS_i16:
    __flags_parse_and_assign_number(flags, flag, value, INT16_MIN, INT16_MAX);
    return;
  case FLAGS_i32:
    __flags_parse_and_assign_number(flags, flag, value, INT32_MIN, INT32_MAX);
    return;
  case FLAGS_i64:
    __flags_parse_and_assign_number(flags, flag, value, INT64_MIN, INT64_MAX);
    return;
  case FLAGS_u8:
    __flags_parse_and_assign_unsigned_number(flags, flag, value, UINT8_MAX);
    return;
  case FLAGS_u16:
    __flags_parse_and_assign_unsigned_number(flags, flag, value, UINT16_MAX);
    return;
  case FLAGS_u32:
    __flags_parse_and_assign_unsigned_number(flags, flag, value, UINT32_MAX);
    return;
  case FLAGS_u64:
    __flags_parse_and_assign_unsigned_number(flags, flag, value, UINT64_MAX);
    return;
  case FLAGS_MULTI_STR:
    // TODO: Handle comma-delimited string lists
    __flags_string_list_append(flag->value, value);
    return;
  default:
    __flags_append_err(flags, "%s:%zu: Internal library error, unhandled type '%s'", __FILE__, __LINE__, flag->type_hint);
    flags->error_code |= FLAGS_ERROR_LIBRARY_IMPLEMENTATION;
  }
}

// END OF PRIVATE FUNCTIONS

int flags_init(flags_context* flags) {
  *flags = (flags_context){
    .items                = calloc(__flags_initial_flags_capacity, sizeof(flags_item)),
    // TODO: We can make this alot smaller now that we have limited the allowed
    // characters used within a name
    // NOTE: Might consider doing this with a dynamic array instead of
    // allocating the maximum possible amount of items instantly.
    .short_to_long_map    = calloc(CHAR_MAX+1, sizeof(flags_item*)),
    .count                = 0,
    .capacity             = __flags_initial_flags_capacity,

    // Error handling
    .error_list           = {0},
  };

  if (flags->items             == NULL) return -1;
  if (flags->short_to_long_map == NULL) return -1;

  return 0;
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

  for (size_t i = 0; i < flags->error_list.count; i++) {
    free(flags->error_list.content[i]);
  }

  free(flags->error_list.content);
}

char* flags_print_usage(flags_context* flags) {
  (void)flags;
  // TODO: Implement usage printing for the flags
  abort();
}

void flags_parse(flags_context* flags, flags_string_list* args, const int argc, char* const* argv) {
  flags->argc = argc;
  flags->argv = malloc(argc * sizeof(char**));

  if (flags->argv == NULL) {
    flags->error_code |= FLAGS_ERROR_ALLOCATION;
    return;
  }

  for (int i = 0; i < argc; i += 1) {
    // OPTIMIZATION: We could possible get the length of the string here and store it
    // somewhere for later reuse
    size_t len = strlen(argv[i]) + 1;

    flags->argv[i] = malloc(len);

    if (flags->argv[i] == NULL) {
      flags->error_code |= FLAGS_ERROR_ALLOCATION;
      return;
    }

    memcpy(flags->argv[i], argv[i], len);
  }

  const unsigned char flag_marker = '-';

  for (int argument_index = 0; argument_index < argc; argument_index += 1) {

    char* raw_flag_string = flags->argv[argument_index];

    if (raw_flag_string[0] == flag_marker) {
      char* value = NULL;
      flags_item* flag = NULL;

      if (raw_flag_string[1] == flag_marker) {
        const char* name = raw_flag_string+2;

        value = strchr(name, '=');
        if (value != NULL) {
          *value = '\0';
          value += 1;
        }

        flag = __flags_get_item(flags, name);

        if (flag == NULL) {
          __flags_append_err(flags, "Unknown flag. Found: --%s", name);
          flags->error_code |= FLAGS_ERROR_UNKNOWN_FLAG;
          continue;
        }

        flag->is_assigned = true;

        if (flag->type == FLAGS_BOOL) {
          if (value == NULL) {
            flag->value = (void*)true;
          } else {
            int boolean_value = __flags_parse_bool(value);

            if (boolean_value == -1) {
              __flags_append_err(flags, "Expected boolean value for flag --%s, found: %s", raw_flag_string, value);
              flags->error_code |= FLAGS_ERROR_NOT_A_BOOL;
              continue;
            }

            flag->value = (void*)(intptr_t)boolean_value;
          }
          continue;
        }

      } else {
        size_t character_index = 1;
        while (raw_flag_string[character_index] != '\0') {
          if (raw_flag_string[character_index] == '=') {
            raw_flag_string[character_index] = '\0';
            value = &raw_flag_string[character_index+1];
            break;
          }

          flag = __flags_get_item_from_short(flags, raw_flag_string[character_index]);

          if (flag == NULL) {
            __flags_append_err(flags, "Unknown flag. Found: -%c", raw_flag_string[character_index]);
            flags->error_code |= FLAGS_ERROR_UNKNOWN_FLAG;
            continue;
          }

          flag->is_assigned = true;

          if (flag->type == FLAGS_BOOL)
            flag->value = (void*)(uintptr_t)true;

          character_index += 1;
        }

        if (flag == NULL)
          continue;

        if (flag->type == FLAGS_BOOL)
          continue;
      }

      if (value == NULL && argument_index < (argc - 1)) {
        argument_index += 1;
        value = flags->argv[argument_index];
        if (*value == '-') {
          argument_index -= 1;
          goto flags_value_not_provided;
        }
      }

      if (value == NULL) {
      flags_value_not_provided:
        __flags_append_err(flags, "Value not provided. Expected: --%s|-%c <%s>", flag->name, flag->short_name, flag->type_hint);
        flags->error_code |= FLAGS_ERROR_VALUE_NOT_PROVIDED;
        continue;
      }

      __flags_update(flags, flag, value);
    } else {
      if (args == NULL) continue;
      __flags_string_list_append(args, flags->argv[argument_index]);
    }
  }
}

bool flags_used(flags_context *flags, const char *name) {
  const flags_item* addr = __flags_get_item(flags, name);
  if (addr == NULL) {
    __flags_append_err(flags, "Flag with name '%s' is not defined", name);
    flags->error_code |= FLAGS_ERROR_UNKNOWN_FLAG;
    return false;
  }
  return addr->is_assigned;
}

inline int8_t * flags_i8 (flags_context* flags, const char* name, unsigned char short_name, int8_t  value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i8, "number", help);
}

inline int16_t* flags_i16(flags_context* flags, const char* name, unsigned char short_name, int16_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i16, "number", help);
}

inline int32_t* flags_i32(flags_context* flags, const char* name, unsigned char short_name, int32_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_i32, "number", help);
}

inline int64_t* flags_i64(flags_context* flags, const char* name, unsigned char short_name, int64_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_i64, "number", help);
}

inline uint8_t * flags_u8 (flags_context* flags, const char* name, unsigned char short_name, uint8_t  value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u8, "positive number", help);
}

inline uint16_t* flags_u16(flags_context* flags, const char* name, unsigned char short_name, uint16_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u16, "positive number", help);
}

inline uint32_t* flags_u32(flags_context* flags, const char* name, unsigned char short_name, uint32_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_u32, "positive number", help);
}

inline uint64_t* flags_u64(flags_context* flags, const char* name, unsigned char short_name, uint64_t value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_u64, "positive number", help);
}

inline char** flags_str(flags_context* flags, const char* name, unsigned char short_name, char* value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)value, FLAGS_STR, "string", help);
}

inline bool* flags_bool(flags_context* flags, const char* name, unsigned char short_name, bool value, const char* help) {
  return __flags_insert(flags, name, short_name, (void*)(uintptr_t)value, FLAGS_BOOL, "boolean", help);
}

flags_string_list* flags_multi_str(flags_context* flags, const char* name, unsigned char short_name, const char* help) {
  flags_string_list* list = malloc(sizeof(flags_string_list));
  if (list == NULL) return NULL;
  *list = (flags_string_list){0};
  return *((flags_string_list**)__flags_insert(flags, name, short_name, list, FLAGS_MULTI_STR, "list of strings", help));
}

#endif // FLAGS_IMPLEMENTATION
