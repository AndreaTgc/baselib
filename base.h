/**
 * Base.h - core library for starting new C projects.
 * Author: AndreaTgc
 * Version: 0.1.0
 *
 * Description: This library contains a bunch of nice utilities that i use
 * whenever i write my personal C projects. I decided to write this as a header
 * only library because i wanted it to be as portable as possible.
 * If you're not familiar with header only libraries, they work as follows:
 * You can include the library in as many files as you want but, in order to
 * get the functions implementations you have to define BASE_IMPLEMENTATION in
 * one (and only one) C file, otherwise you're going to get a linker error for
 * missing symbols.
 *
 * The implemented features are the following:
 * - General purpose macros
 * - Arena allocator
 * - Length based strings (instead of null terminated)
 * - Macro based generic vector (stb_da style)
 */
#ifndef BASE_H__
#define BASE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifndef BASE_ASSERT
#include <assert.h>
#define BASE_ASSERT(cond) assert(cond)
#endif /* BASE_ASSERT */

#ifdef BASE_STATIC
#define BASE_API static
#else
#define BASE_API extern
#endif /* BASE_STATIC */

/**
 * The following macros are the ones that i end up relying on the most when I
 * work on my personal projects. They are nothing crazy but i just don't want to
 * waste my time rewriting them for every single project.
 */

#define KILOBYTES(n)          ((n) * 1024)
#define MEGABYTES(n)          (KILOBYTES(n) * 1024)
#define GIGABYTES(n)          (MEGABYTES(n) * 1024)
#define ARRLEN(arr)           (sizeof(arr) / sizeof((arr)[0]))
#define MIN(x, y)             ((x) < (y) ? (x) : (y))
#define MAX(x, y)             ((x) > (y) ? (x) : (y))
#define CLAMP(x, lo, hi)      (MAX(lo, MIN(x, hi)))
#define STATIC_ASSERT(c, msg) typedef int assert_##msg[(c) ? 1 : -1]

/**
 * This is a very basic implementation of a arena allocator (also called linear
 * allocator). I decided not to make this a growable arena because, most of the
 * time, we do not need this feature and we can just allocate a really big chunk
 * of memory at the program startup and use it until the very end.
 * The arena can be initialized using heap memory or by passing an buffer and a
 * length to have finer control over where the memory comes from.
 */

typedef struct { uint8_t *data; size_t size, capacity; } Arena;

BASE_API void  arenaInit           (Arena *ar, size_t bytes);
BASE_API void  arenaInitFromBuffer (Arena *ar, void *buf, size_t bytes);
BASE_API void* arenaAllocAligned   (Arena *ar, size_t bytes, size_t align);
BASE_API void* arenaAllocUnaligned (Arena *ar, size_t bytes);
BASE_API void  arenaDeinit         (Arena *ar);

/**
 * This Str implementation is made to be really lightweight and provide a better
 * development experience compared to C null-terminated strings.
 * It can work as a non owning string slice, in fact, most of the functions do
 * not allocate any dynamic memory, the only exception is strSlurpFile that has
 * to allocate dynamic memory depending on the file lenght.
 */

typedef struct { char *data; size_t size; } Str;

BASE_API Str  strFromCStr   (const char *cstr);
BASE_API Str  strSlurpFile  (const char *path);
BASE_API Str  strFromParts  (const char *startPtr, size_t len);
BASE_API Str  strTrimLeft   (Str s);
BASE_API Str  strTrimRight  (Str s);
BASE_API Str  strTrim       (Str s);
BASE_API Str  strGetSubstr  (Str s, size_t startIndex, size_t len);
BASE_API Str  strChopLeft   (Str *s, Str delim);
BASE_API Str  strChopLeftC  (Str *s, char delim);
BASE_API bool strEquals     (Str s1, Str s2);
BASE_API bool strContains   (Str haystack, Str needle);
BASE_API bool strStartsWith (Str haystack, Str needle);
BASE_API bool strEndsWith   (Str haystack, Str needle);

/**
 * This dynamic array implementation is inspired by the stb_da library and the
 * implementation used in Nob.h.
 * The idea is to have a really lightweight struct that offers some of the basic
 * features found in C++'s vector.
 * Unfortunately the lack of generics/templates in C requires us to use a little
 * bit of macro magic but overall it's still quite readable.
 */

#define DA(T) struct { T *data; size_t size, capacity; }

BASE_API void daGrow (void **data, size_t *capacity, size_t elementSize); 

#define daReset(da)          ((da)->size = 0)
#define daPopLast(da)        ((da)->data[--(da)->size])
#define daFirst(da)          ((da)->data[0])
#define daLast(da)           ((da)->data[(da)->size - 1])
#define daAt(da, index)      ((da)->data[(index)])
#define daForEach(da, T, it) for (T *(it) = (da)->data; (it) != ((da)->data) + ((da)->size); (it)++)

#define daAppend(da, i)                                                        \
  do {                                                                         \
    if ((da)->size == (da)->capacity)                                          \
      daGrow((void **)&(da)->data, &(da)->capacity, sizeof((da)->data[0]));    \
    (da)->data[(da)->size++] = (i);                                            \
  } while (0)                                                                  \

#define daFree(da)                                                             \
  do {                                                                         \
    if ((da)->data)                                                            \
      free((da)->data);                                                        \
    (da)->data = NULL;                                                         \
    (da)->size = 0;                                                            \
    (da)->capacity = 0;                                                        \
  } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASE_H__ */

#ifdef BASE_IMPLEMENTATION

#include <stdio.h>

BASE_API void arenaInit(Arena *ar, size_t bytes) {
  BASE_ASSERT(ar);
  memset(ar, 0, sizeof(*ar));
  ar->capacity = bytes;
  ar->data = (uint8_t*)calloc(sizeof(uint8_t), bytes);
}

BASE_API void arenaInitFromBuffer(Arena *ar, void *buf, size_t bytes) {
  BASE_ASSERT(ar);
  ar->size = 0;
  ar->capacity = bytes;
  ar->data = buf;
}

BASE_API void *arenaAllocUnaligned(Arena *ar, size_t bytes) {
  BASE_ASSERT(ar);
  if (ar->size + bytes > ar->capacity) {
    return NULL;
  }
  void *p = ar->data + ar->size;
  ar->size += bytes;
  return p;
}

BASE_API void arenaDeinit(Arena *ar) {
  BASE_ASSERT(ar);
  if (ar->data) free(ar->data);
  memset(ar, 0, sizeof(*ar));
}

BASE_API Str strFromCStr(const char *cstr) {
  return cstr == NULL
    ? (Str) { .data = NULL, .size = 0 }
    : (Str) { .data = (char *)cstr, .size = strlen(cstr) };
}

BASE_API Str strSlurpFile(const char *path) {
  BASE_ASSERT(path);
  FILE *f = fopen(path, "rb");
  if (!f) {
    return (Str) { .data = NULL, .size = 0 };
  }

  fseek(f, 0, SEEK_END);
  size_t bytes = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  char *data = (char *)malloc(bytes);
  if (!data) {
    return (Str) { .data = NULL, .size = 0 };
  }

  fread(data, 1, bytes, f);
  fclose(f);

  return (Str) { .data = data, .size = bytes };
}

BASE_API Str strFromParts(const char *startPtr, size_t len) {
  return (Str) { .data = (char *)startPtr, .size = len };
}

BASE_API Str strTrimLeft(Str s) {
  while (s.size) {
    if (isspace((unsigned char) s.data[0])) {
      s.size--;
      s.data++;
    } else {
      break;
    }
  }
  return s;
}

BASE_API Str strTrimRight(Str s) {
  while (s.size) {
    if (isspace((unsigned char) s.data[s.size-1])) {
      s.size--;
    } else {
      break;
    }
  }
  return s;
}

BASE_API Str strTrim(Str s) {
  return strTrimLeft(strTrimRight(s));
}

BASE_API bool strEquals(Str s1, Str s2) {
  return s1.size == s2.size &&
         !memcmp(s1.data, s2.data, s1.size);
}

BASE_API bool strContains(Str haystack, Str needle) {
  if (needle.size == 0) {
    return true;
  }
  for (size_t i = 0; i < haystack.size; i++) {
    if (haystack.data[i] == needle.data[0] &&
        haystack.size - i > needle.size &&
        !memcmp(&haystack.data[i], needle.data, needle.size)) {
      return true;
    }
    if (needle.size > haystack.size - i) {
      break;
    }
  }
  return false;
}

BASE_API Str strGetSubstr(Str s, size_t startIndex, size_t len) {
  if (startIndex > s.size) {
    return (Str) { .data = NULL, .size = 0 };
  }
  if (len >= s.size - startIndex) {
    len = s.size - startIndex;
  }
  return (Str) { .data = &s.data[startIndex], .size = len };
}

BASE_API Str strChopLeft(Str *s, Str delim) {
  if (delim.size != 0 && delim.size <= s->size) {
    for (size_t i = 0; i <= s->size - delim.size; i++) {
      if (memcmp(&s->data[i], delim.data, delim.size) == 0) {
        Str res = strFromParts(s->data, i);
        s->data += i + delim.size;
        s->size -= i + delim.size;
        return res;
      }
    } 
  }
  Str ret = *s;
  s->data += s->size;
  s->size = 0;
  return ret;
}

BASE_API Str strChopLeftC(Str *s, char delim) {
  Str c = strFromParts(&delim, 1);
  return strChopLeft(s, c);
}

BASE_API bool strStartsWith(Str haystack, Str needle) {
  return needle.size <= haystack.size &&
         !memcmp(haystack.data, needle.data, needle.size);
}

BASE_API bool strEndsWith(Str haystack, Str needle) {
  return needle.size <= haystack.size &&
         !memcmp(&haystack.data[haystack.size - needle.size], needle.data,
                 needle.size);
}

BASE_API void daGrow(void **data, size_t *capacity, size_t elementSize) {
  size_t newCapacity = *capacity == 0 ? 8 : *capacity * 2;
  void *newData = realloc(*data, newCapacity * elementSize);
  BASE_ASSERT(newData && "daGrow failed to realloc data");
  *data = newData;
  *capacity = newCapacity;
}

#endif /* BASE_IMPLEMENTATION */
