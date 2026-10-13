#ifndef PRELUDE_H
#define PRELUDE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// --- sized type aliases ------------------------------------------------------

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

typedef size_t    usize;
typedef ptrdiff_t isize;

// --- integer limits ----------------------------------------------------------

#define U8_MAX UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX

#define I8_MAX INT8_MAX
#define I8_MIN INT8_MIN
#define I16_MAX INT16_MAX
#define I16_MIN INT16_MIN
#define I32_MAX INT32_MAX
#define I32_MIN INT32_MIN
#define I64_MAX INT64_MAX
#define I64_MIN INT64_MIN

#define USIZE_MAX SIZE_MAX
#define ISIZE_MAX PTRDIFF_MAX
#define ISIZE_MIN PTRDIFF_MIN

// --- string_view -------------------------------------------------------------

typedef struct string_view {
  const u8* data;
  usize len;
} string_view;

/// Compare two views for byte equality.
bool sv_eq(const string_view a, const string_view b);

/// True if `haystack` contains `needle` at its start.
bool sv_starts_with(string_view haystack, string_view needle);

/// True if `haystack` contains `needle` at its end.
bool sv_ends_with(string_view haystack, string_view needle);

/// True if `haystack` contains `needle` anywhere.
bool sv_contains(string_view haystack, string_view needle);

/// Return the first index of `needle` in `haystack`, or `USIZE_MAX` if not
/// found.
usize sv_find(string_view haystack, string_view needle);

/// Slice a sub-view. Clamp out-of-bounds start/len.
string_view sv_slice(string_view src, usize start, usize len);

/// Trim ASCII whitespace from the left.
string_view sv_trim_left(string_view src);

/// Trim ASCII whitespace from the right.
string_view sv_trim_right(string_view src);

/// Trim ASCII whitespace from both sides.
string_view sv_trim(string_view src);

#endif // PRELUDE_H

#ifdef PRELUDE_IMPLEMENTATION

bool sv_eq(string_view a, string_view b) {
  return a.len == b.len && memcmp(a.data, b.data, a.len) == 0;
}

bool sv_starts_with(string_view haystack, string_view needle) {
  return needle.len <= haystack.len && memcmp(haystack.data, needle.data, needle.len) == 0;
}

bool sv_ends_with(string_view haystack, string_view needle) {
  return needle.len < haystack.len && memcmp(haystack.data + haystack.len - needle.len, needle.data, needle.len) == 0;
}

bool sv_contains(string_view haystack, string_view needle) {
  return sv_find(haystack, needle) != USIZE_MAX;
}

usize sv_find(string_view haystack, string_view needle) {
  if (needle.len == 0)
    return 0;
  if (needle.len > haystack.len)
    return USIZE_MAX;

  usize last = haystack.len - needle.len;
  for (usize i = 0; i <= last; i++) {
    if (memcmp(haystack.data + i, needle.data, needle.len) == 0) {
      return i;
    }
  }
  return USIZE_MAX;
}

string_view sv_slice(string_view src, usize start, usize len) {
  if (start >= src.len)
    return (string_view){.data = NULL, .len = 0};
  if (start + len > src.len)
    len = src.len - start;
  return (string_view){.data = src.data + start, .len = len};
}

#define _sv_is_space(c)                                                        \
  ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r' || (c) == '\f' ||   \
   (c) == '\v')

string_view sv_trim_left(string_view src) {
  usize i = 0;
  while (i < src.len && _sv_is_space(src.data[i]))
    i++;
  return sv_slice(src, i, src.len - i);
}

string_view sv_trim_right(string_view src) {
  usize i = src.len;
  while (i > 0 && _sv_is_space(src.data[i - 1]))
    i--;
  return sv_slice(src, 0, i);
}

#undef _sv_is_space

string_view sv_trim(string_view src) {
  return sv_trim_right(sv_trim_left(src));
}

#endif // PRELUDE_IMPLEMENTATION
