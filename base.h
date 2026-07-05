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
 * The code style you're going to see below can be summed up as follows:
 * - General purpose macros are going to be in ALL_CAPS
 * - Types are going to be in PascalCase
 * - functions (or function like macros) are going to be in camelCase
 *
 * The implemented features are the following:
 * - General purpose macros
 * - Arena allocator
 * - Length based strings (instead of null terminated)
 * - Macro based generic vector (stb_da style)
 * - String builder, wrapper around DA(char)
 * - Intrusive linked list
 *
 * LICENSING:
 * This library is distributed with the MIT license. See the bottom of this
 * file for the license.
 */
#ifndef BASE_H_
#define BASE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
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
 * waste my time rewriting them for ever single project.
 */

#define KILOBYTES(n)          ((n) * 1024)
#define MEGABYTES(n)          (KILOBYTES(n) * 1024)
#define GIGABYTES(n)          (MEGABYTES(n) * 1024)
#define ARRLEN(arr)           (sizeof(arr) / sizeof((arr)[0]))
#define MIN(x, y)             ((x) < (y) ? (x) : (y))
#define MAX(x, y)             ((x) > (y) ? (x) : (y))
#define ABS(x)                ((x) >= 0 ? (x) : -(x))
#define SIGN(x)               ((x) >= 0 ? 1 : -1)
#define CLAMP(x, lo, hi)      (MAX(lo, MIN(x, hi)))
#define MEMZERO(p, l)         memset(p, 0, l)
#define ALIGN_UP(x, a)        (((x) + ((a) - 1)) & ~((a) - 1))
#define IS_POW2(x)            (((x) & ((x) - 1)) == 0)
#define UNUSED(x)             ((void)(x))
#define CAST(T, x)            ((T)(x))
#define DEFER(s, e)           for (int _i = ((s), 0); !_i; ((e), ++_i))
#define CONTAINER_OF(p, T, m) ((T *)((char *)(p) - offsetof(T, m)))
#define PANIC()               do { fprintf(stderr, "PANIC: %s:%d\n", __FILE__, __LINE__); abort(); } while (0)
#define TODO(msg)             do { fprintf(stderr, "TODO: %s:%d - "msg"\n", __FILE__, __LINE__); abort(); } while (0)
#define STATIC_ASSERT(c, msg) typedef int assert_##msg[(c) ? 1 : -1]
#define CH_ISSPACE(c)         ((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == '\r')
#define CH_ISDIGIT(c)         ((c) >= '0' && (c) <= '9')
#define CH_ISALPHA(c)         ((c) >= 'a' && (c) <= 'z' || (c) >= 'A' && (c) <= 'Z')
#define CH_ISALNUM(c)         (CH_ISDIGIT(c) || CH_ISALPHA(c))

#if defined(__GNUC__) || defined(__clang__)
#define BASE_PRINTF_LIKE(fmt, args) __attribute__((format(printf, fmt, args)))
#define BASE_NODISCARD              __attribute__((warn_unused_result))
#define BASE_LIKELY(x)              __builtin_expect(!!(x), 1)
#define BASE_UNLIKELY(x)            __builtin_expect(!!(x), 0)
#else
#define BASE_PRINTF_LIKE(fmt, args)
#define BASE_NODISCARD
#define BASE_LIKELY(x)              (x)
#define BASE_UNLIKELY(x)            (x)
#endif /* __GNUC__ || __clang__ */

/**
 * This is a very basic implementation of a arena allocator (also called linear
 * allocator). I decided not to make this a growable arena because, most of the
 * time, we do not need this feature and we can just allocate a really big chunk
 * of memory at the program startup and use it until the very end.
 * The arena can be initialized using heap memory or by passing an buffer and a
 * length to have finer control over where the memory comes from.
 */

typedef struct { uint8_t *data; size_t size, capacity; } Arena;

BASE_API bool  arenaInit           (Arena *ar, size_t bytes);
BASE_API bool  arenaInitFromBuffer (Arena *ar, void *buf, size_t bytes);
BASE_API void* arenaAllocAligned   (Arena *ar, size_t bytes, size_t align);
BASE_API void* arenaAllocUnaligned (Arena *ar, size_t bytes);
BASE_API char* arenaAllocFmt       (Arena *ar, const char *fmt, ...) BASE_PRINTF_LIKE(2, 3);
BASE_API void  arenaReset          (Arena *ar);
BASE_API void  arenaRestoreAt      (Arena *ar, size_t size);
BASE_API void  arenaDeinit         (Arena *ar);

#define arenaAllocT(ar, T) (T *)arenaAllocAligned(ar, sizeof(T), _Alignof(T))
#define arenaAllocN(ar, T, n) (T *)arenaAllocAligned(ar, sizeof(T) * (n), _Alignof(T))

/**
 * This Sv (string view) implementation is made to be really lightweight and
 * provide a better development experience compared to C null-terminated strings.
 * It can work as a non owning string slice, in fact, most of the functions do
 * not allocate any dynamic memory, the only exception is svSlurpFile that has
 * to allocate dynamic memory depending on the file length.
 */

typedef struct { char *data; size_t size; } Sv;

#define SV_LIT(l) (Sv) { (l), sizeof(l) - 1 }
#define SV_NIL (Sv) { NULL, 0 }
#define SV_FMT "%.*s"
#define SV_FMT_ARGS(s) (int)(s).size, (s).data

BASE_API Sv     svFromCStr   (const char *cstr);
BASE_API Sv     svSlurpFile  (const char *path);
BASE_API Sv     svFromParts  (const char *startPtr, size_t len);
BASE_API Sv     svTrimLeft   (Sv s);
BASE_API Sv     svTrimRight  (Sv s);
BASE_API Sv     svTrim       (Sv s);
BASE_API Sv     svGetSubstr  (Sv s, size_t startIndex, size_t len);
BASE_API Sv     svChopLeft   (Sv *s, Sv delim);
BASE_API Sv     svChopLeftC  (Sv *s, char delim);
BASE_API bool   svEquals     (Sv s1, Sv s2);
BASE_API bool   svContains   (Sv haystack, Sv needle);
BASE_API bool   svStartsWith (Sv haystack, Sv needle);
BASE_API bool   svEndsWith   (Sv haystack, Sv needle);
BASE_API size_t svHash       (Sv s);

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
#define daSwapRemove(da, i)  ((da)->data[i] = (da)->data[--(da)->size])
#define daForEach(da, T, it) for (T *(it) = (da)->data; (it) != ((da)->data) + ((da)->size); (it)++)

#define daRemove(da, i)                                                        \
  do {                                                                         \
    BASE_ASSERT((i) >= 0 && (i) < (da)->size);                                 \
    memmove(&daAt(da, i), &daAt(da, i + 1),                                    \
            sizeof(daAt(da, 0)) * ((da)->size - i - 1));                       \
    (da)->size--;                                                              \
  } while (0)                                                                  \

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

/**
 * This is a very basic implementation of a string builder. it's a wrapper over
 * the DA implementation (see above). The string builder by itself is not null
 * terminated but it can be converted to a cstr string using sbToCStr and
 * you're going to be able to pass the "data" field to C functions that expect
 * a null terminated buffer.
 * If you push some content onto the string builder after converting it to a
 * cstr, the content is not going to be null terminated anymore, you should
 * treat sbToCStr as a utility that you call only when you need to interact
 * with functions that require null terminators.
 */

typedef DA(char) StrBuilder;

BASE_API void sbAppendCStr  (StrBuilder *sb, const char *cstr);
BASE_API void sbAppendSlice (StrBuilder *sb, const char *ptr, size_t len);
BASE_API void sbAppendSv    (StrBuilder *sb, Sv sv);
BASE_API void sbToCStr      (StrBuilder *sb);
BASE_API void sbAppendFmt   (StrBuilder *sb, const char *fmt, ...) BASE_PRINTF_LIKE(2, 3);

#define sbFree(sb) daFree(sb)

/**
 * Minimal intrusive doubly linked list implementation. the type is meant to be
 * embedded into other structs in order to access the implementation features
 */

typedef struct LLNode { struct LLNode *prev, *next; } LLNode;

BASE_API void llInsertBetween (LLNode *prev, LLNode *next, LLNode *node);
BASE_API void llSplice        (LLNode *pos, LLNode *other);
BASE_API void llRemoveNode    (LLNode *node);

#define llIsEmpty(h)         ((h)->next == (h))
#define llPushBack(h, n)     llInsertBetween((h)->prev, (h), (n))
#define llPushFront(h, n)    llInsertBetween((h), (h)->next, (n))
#define llInsertAfter(a, n)  llInsertBetween((a), (a)->next, (n))
#define llInsertBefore(b, n) llInsertBetween((b)->prev, (b), (n))
#define llInitSentinel(n)    do { (n)->prev = (n)->next = (n); } while (0)
#define llForEach(h, it)     for (LLNode *(it) = (h)->next; (it) != (h); (it) = (it)->next)
#define llForEachRev(h, it)  for (LLNode *(it) = (h)->prev; (it) != (h); (it) = (it)->prev)

/**
 * The following is a very simple non-owning string map implementation.
 * It is meant to be extremely simple, i decided to use linear probing because
 * even if primary clustering is definitely a thing, the cache locality this
 * probing technique provides often make it so that it doesn't really matter.
 * To make things a little faster, i store 6 bits of the hashes to avoid some
 * useless comparisons (string equality is still pretty expensive). These bits
 * are packed with 2 bits to identify the slot state.
 */

#ifndef SVMAP_MAX_LOAD_FACTOR
  #define SVMAP_MAX_LOAD_FACTOR 0.7
#endif /* SVMAP_MAX_LOAD_FACTOR */

#ifndef SVMAP_INVALID_VALUE
  #define SVMAP_INVALID_VALUE ((intptr_t)-1)
#endif /* SVMAP_INVALID_VALUE */

#define SVMAP_META_EMPTY (0)
#define SVMAP_META_TOMBS (1)
#define SVMAP_META_OCCUP (2)
#define SVMAP_META_STATE_MASK (0x03)
#define SVMAP_META_HASH_MASK (0xFC)

typedef struct {
  Sv *keys;
  intptr_t *values;
  uint8_t *meta;
  size_t size;
  size_t capacity;
  size_t used;
  bool canRehash;
} SvMap;

BASE_API bool      svMapInit     (SvMap *map, size_t capacity, bool canRehash);
BASE_API bool      svMapDeinit   (SvMap *map);
BASE_API bool      svMapRehash   (SvMap *map, size_t newCapacity);
BASE_API bool      svMapInsert   (SvMap *map, Sv key, intptr_t value);
BASE_API bool      svMapContains (SvMap *map, Sv key);
BASE_API bool      svMapRemove   (SvMap *map, Sv key);
BASE_API intptr_t *svMapFindPtr  (SvMap *map, Sv key);
BASE_API intptr_t  svMapFind     (SvMap *map, Sv key);

/**
 * The next section contains a series of file system utilities that can be used
 * on POSIX systems and windows.
 * They try to stick to standard C as much as possible, using platform dependent
 * api only when needed.
 * They are built on top the types seen before
 */

typedef bool (*FsWalkFn) (const char *path, bool isDir, void *userData);

BASE_API bool fsCreateDir    (const char *path);
BASE_API bool fsExists       (const char *path);
BASE_API bool fsIsFile       (const char *path);
BASE_API bool fsIsDir        (const char *path);
BASE_API bool fsDeleteFile   (const char *path);
BASE_API bool fsCopyFile     (const char *src, const char *dst);
BASE_API bool fsMoveFile     (const char *src, const char *dst);
BASE_API bool fsWalkDir      (const char *path, FsWalkFn fn, void *userData, Arena *ar);
BASE_API Sv   fsGetBaseName  (Sv path);
BASE_API Sv   fsGetExtension (Sv path);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASE_H_ */

#ifdef BASE_IMPLEMENTATION

#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
  #if !defined(WIN32_LEAN_AND_MEAN)
    #define WIN32_LEAN_AND_MEAN
  #endif /* WIN32_LEAN_AND_MEAN */
  #if !defined(NOMINMAX)
    #define NOMINMAX
  #endif /* NOMINMAX */
  #include <windows.h>
  #include <direct.h>
#else /* we assume posix if not windows */
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
  #include <dirent.h>
  #include <errno.h>
#endif /* _WIN32 || _WIN64 */

BASE_API bool arenaInit(Arena *ar, size_t bytes) {
  if (!ar) { return false; }
  MEMZERO(ar, sizeof(*ar));
  ar->data = (uint8_t*)calloc(sizeof(uint8_t), bytes);
  if (!ar->data) { return false; }
  ar->capacity = bytes;
  return true;
}

BASE_API bool arenaInitFromBuffer(Arena *ar, void *buf, size_t bytes) {
  if (!ar || !buf) { return false; }
  ar->size = 0;
  ar->capacity = bytes;
  ar->data = buf;
  return true;
}

BASE_API void *arenaAllocAligned(Arena *ar, size_t bytes, size_t align) {
  if (!ar) { return NULL; }
  if (align == 0 || !IS_POW2(align)) { return NULL; }
  size_t offset = ALIGN_UP(ar->size, align);
  if (offset + bytes > ar->capacity) { return NULL; }
  void *p  = ar->data + offset;
  ar->size = offset + bytes;
  return p;
}

BASE_API void *arenaAllocUnaligned(Arena *ar, size_t bytes) {
  if (!ar) { return NULL; }
  if (ar->size + bytes > ar->capacity) { return NULL; }
  void *p = ar->data + ar->size;
  ar->size += bytes;
  return p;
}

BASE_API char* arenaAllocFmt(Arena *ar, const char *fmt, ...) {
  if (!ar) { return NULL; }
  va_list args;
  va_start(args, fmt);
  int needed = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  if (needed <= 0) { return NULL; }
  char *buf = (char *)arenaAllocAligned(ar, (size_t)needed + 1, _Alignof(char));
  if (!buf) { return NULL; }
  va_start(args, fmt);
  vsnprintf(buf, (size_t)needed + 1, fmt, args);
  va_end(args);
  return buf;
}

BASE_API void arenaReset(Arena *ar) {
  if (!ar) { return; }
  ar->size = 0;
}

BASE_API void arenaRestoreAt(Arena *ar, size_t size) {
  if (!ar || size > ar->capacity) { return; };
  ar->size = size;
}

BASE_API void arenaDeinit(Arena *ar) {
  if (!ar) { return; }
  if (ar->data) free(ar->data);
  MEMZERO(ar, sizeof(*ar));
}

BASE_API Sv svFromCStr(const char *cstr) {
  return cstr == NULL
    ? (Sv) { .data = NULL, .size = 0 }
    : (Sv) { .data = (char *)cstr, .size = strlen(cstr) };
}

BASE_API Sv svSlurpFile(const char *path) {
  BASE_ASSERT(path);
  FILE *f = fopen(path, "rb");
  if (!f) {
    return (Sv) { .data = NULL, .size = 0 };
  }

  fseek(f, 0, SEEK_END);
  size_t bytes = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  char *data = (char *)malloc(bytes);
  if (!data) {
    fclose(f);
    return (Sv) { .data = NULL, .size = 0 };
  }

  fread(data, 1, bytes, f);
  fclose(f);

  return (Sv) { .data = data, .size = bytes };
}

BASE_API Sv svFromParts(const char *startPtr, size_t len) {
  return (Sv) { .data = (char *)startPtr, .size = len };
}

BASE_API Sv svTrimLeft(Sv s) {
  while (s.size) {
    if (CH_ISSPACE(s.data[0])) {
      s.data++;
      s.size--;
    } else {
      break;
    }
  }
  return s;
}

BASE_API Sv svTrimRight(Sv s) {
  while (s.size) {
    if (CH_ISSPACE(s.data[s.size-1])) {
      s.size--;
    } else {
      break;
    }
  }
  return s;
}

BASE_API Sv svTrim(Sv s) {
  return svTrimLeft(svTrimRight(s));
}

BASE_API bool svEquals(Sv s1, Sv s2) {
  return s1.size == s2.size &&
         !memcmp(s1.data, s2.data, s1.size);
}

BASE_API bool svContains(Sv haystack, Sv needle) {
  if (needle.size == 0) {
    return true;
  }
  for (size_t i = 0; i < haystack.size; i++) {
    if (haystack.data[i] == needle.data[0] &&
        haystack.size - i >= needle.size &&
        !memcmp(&haystack.data[i], needle.data, needle.size)) {
      return true;
    }
    if (needle.size > haystack.size - i) {
      break;
    }
  }
  return false;
}

BASE_API Sv svGetSubstr(Sv s, size_t startIndex, size_t len) {
  if (startIndex > s.size) {
    return (Sv) { .data = NULL, .size = 0 };
  }
  if (len >= s.size - startIndex) {
    len = s.size - startIndex;
  }
  return (Sv) { .data = &s.data[startIndex], .size = len };
}

BASE_API Sv svChopLeft(Sv *s, Sv delim) {
  if (delim.size != 0 && delim.size <= s->size) {
    for (size_t i = 0; i <= s->size - delim.size; i++) {
      if (memcmp(&s->data[i], delim.data, delim.size) == 0) {
        Sv res = svFromParts(s->data, i);
        s->data += i + delim.size;
        s->size -= i + delim.size;
        return res;
      }
    }
  }
  Sv ret = *s;
  s->data += s->size;
  s->size = 0;
  return ret;
}

BASE_API Sv svChopLeftC(Sv *s, char delim) {
  for (size_t i = 0; i < s->size; i++) {
    if (s->data[i] == delim) {
      Sv res = svFromParts(s->data, i);
      s->data += i + 1;
      s->size -= i + 1;
      return res;
    }
  }

  Sv ret = *s;
  s->data += s->size;
  s->size = 0;
  return ret;
}

BASE_API bool svStartsWith(Sv haystack, Sv needle) {
  return needle.size <= haystack.size &&
         !memcmp(haystack.data, needle.data, needle.size);
}

BASE_API bool svEndsWith(Sv haystack, Sv needle) {
  return needle.size <= haystack.size &&
         !memcmp(&haystack.data[haystack.size - needle.size], needle.data,
                 needle.size);
}

BASE_API size_t svHash(Sv s) {
  uint64_t h = 0x9e3779b97f4a7c15ULL;
  h ^= (uint64_t)s.size * 0x9e3779b97f4a7c15ULL;
  for (size_t i = 0; i < s.size; i++) {
    h ^= (uint8_t)s.data[i];
    h *= 0x9e3779b97f4a7c15ULL;
  }
  h ^= h >> 30;
  h *= 0xbf58476d1ce4e5b9ULL;
  h ^= h >> 27;
  h *= 0x94d049bb133111ebULL;
  h ^= h >> 31;
  return (size_t)(h ^ (h >> (sizeof(size_t) * 4)));
}

BASE_API void daGrow(void **data, size_t *capacity, size_t elementSize) {
  size_t newCapacity = *capacity == 0 ? 8 : *capacity * 2;
  void *newData = realloc(*data, newCapacity * elementSize);
  BASE_ASSERT(newData && "daGrow failed to realloc data");
  *data = newData;
  *capacity = newCapacity;
}

BASE_API void sbAppendCStr(StrBuilder *sb, const char *cstr) {
  BASE_ASSERT(sb && cstr);
  size_t slen = strlen(cstr);
  for (size_t i = 0; i < slen; i++) {
    daAppend(sb, cstr[i]);
  }
}

BASE_API void sbAppendSlice(StrBuilder *sb, const char *ptr, size_t len) {
  BASE_ASSERT(sb && ptr);
  for (size_t i = 0; i < len; i++) {
    daAppend(sb, ptr[i]);
  }
}

BASE_API void sbAppendSv(StrBuilder *sb, Sv sv) {
  sbAppendSlice(sb, sv.data, sv.size);
}

BASE_API void sbToCStr(StrBuilder *sb) {
  BASE_ASSERT(sb);
  daAppend(sb, '\0');
  sb->size--;
}

BASE_API void sbAppendFmt(StrBuilder *sb, const char *fmt, ...) {
  BASE_ASSERT(sb && fmt);
  va_list args;
  va_start(args, fmt);
  int needed = vsnprintf(NULL, 0, fmt, args);
  va_end(args);

  if (needed <= 0) { return; }
  size_t oldSize = sb->size;
  size_t newSize = oldSize + (size_t)needed;

  while (sb->capacity < newSize + 1) {
    daGrow((void **)&sb->data, &sb->capacity, sizeof(char));
  }

  va_start(args, fmt);
  vsnprintf(sb->data + oldSize, (size_t)needed + 1, fmt, args);
  va_end(args);
  sb->size = newSize;
}

BASE_API void llInsertBetween(LLNode *prev, LLNode *next, LLNode *node) {
  BASE_ASSERT(prev && next && node);
  node->prev = prev;
  node->next = next;
  prev->next = node;
  next->prev = node;
}

BASE_API void llSplice(LLNode *pos, LLNode *other) {
  BASE_ASSERT(pos && other);
  if (llIsEmpty(other)) return;
  LLNode *otherFirst = other->next;
  LLNode *otherLast = other->prev;
  LLNode *posPrev = pos->prev;

  posPrev->next = otherFirst;
  otherFirst->prev = posPrev;
  otherLast->next = pos;
  pos->prev = otherLast;
  llInitSentinel(other);
}

BASE_API void llRemoveNode(LLNode *node) {
  BASE_ASSERT(node && node->next && node->prev);
  node->prev->next = node->next;
  node->next->prev = node->prev;
  node->next = NULL;
  node->prev = NULL;
}

BASE_API bool svMapInit(SvMap *map, size_t capacity, bool canRehash) {
  if (!map) { return false; }
  if (capacity == 0 || !IS_POW2(capacity)) { return false; }
  MEMZERO(map, sizeof(*map));
  map->keys = calloc(sizeof(Sv), capacity);
  if (!map->keys) { return false; }
  map->values = calloc(sizeof(intptr_t), capacity);
  if (!map->values) { free(map->keys); return false; }
  map->meta = calloc(sizeof(uint8_t), capacity);
  if (!map->meta) { free(map->keys); free(map->values); return false; }
  map->capacity = capacity;
  map->canRehash = canRehash;
  return true;
}

BASE_API bool svMapDeinit(SvMap *map) {
  if (!map) { return false; }
  if (map->keys) { free(map->keys); }
  if (map->values) { free(map->values); }
  if (map->meta) { free(map->meta); }
  MEMZERO(map, sizeof(*map));
  return true;
}

BASE_API bool svMapRehash(SvMap *map, size_t newCapacity) {
  if (!map) { return false; }
  if (newCapacity < map->capacity || !IS_POW2(newCapacity)) { return false; }
  SvMap oldMap = *map;
  if (!svMapInit(map, newCapacity, map->canRehash)) {
    /* if the init failed, we just reset the map to the old state
     * and return to the caller, if this was performed during an insert
     * it means that the insert itself will fail but the map will keep all
     * the data. Giving the user more options on what to do in case of failure */
    *map = oldMap;
    return false;
  }

  for (size_t i = 0; i < oldMap.capacity; i++) {
    if ((oldMap.meta[i] & SVMAP_META_STATE_MASK) == SVMAP_META_OCCUP) {
      svMapInsert(map, oldMap.keys[i], oldMap.values[i]);
    }
  }

  free(oldMap.keys);
  free(oldMap.values);
  free(oldMap.meta);
  return true;
}

BASE_API bool svMapInsert(SvMap *map, Sv key, intptr_t value) {
  if (!map) { return false; }

  if (map->canRehash) {
    if ((double)map->used / (double)map->capacity > SVMAP_MAX_LOAD_FACTOR) {
      if (!svMapRehash(map, map->capacity * 2)) { return false; }
    }
  }

  size_t hash = svHash(key);
  size_t index = hash & (map->capacity - 1);
  uint8_t hMeta = ((uint8_t)(hash & SVMAP_META_HASH_MASK));
  size_t tomb = (size_t)-1;

  for (size_t i = 0; i < map->capacity; i++) {
    uint8_t m = map->meta[index];
    uint8_t state = m & SVMAP_META_STATE_MASK;
    if (state == SVMAP_META_EMPTY) { break; }
    if (state == SVMAP_META_TOMBS) {
      if (tomb == (size_t)-1) { tomb = index; }
    } else {
      if ((m & SVMAP_META_HASH_MASK) == hMeta && svEquals(map->keys[index], key)) {
        map->values[index] = value;
        return true;
      }
    }
    index = (index + 1) & (map->capacity - 1);
  }

  size_t target = (tomb != (size_t)-1) ? tomb : index;
  if ((map->meta[target] & SVMAP_META_STATE_MASK) == SVMAP_META_EMPTY) { map->used++; }
  map->keys[target] = key;
  map->values[target] = value;
  map->meta[target] = (uint8_t)(SVMAP_META_OCCUP | hMeta);
  map->size++;
  return true;
}

BASE_API bool svMapContains(SvMap *map, Sv key) {
  return svMapFindPtr(map, key) != NULL;
}

BASE_API bool svMapRemove(SvMap *map, Sv key) {
  if (!map) { return false; }
  size_t hash = svHash(key);
  size_t index = hash & (map->capacity - 1);
  uint8_t hMeta = ((uint8_t)(hash & SVMAP_META_HASH_MASK));

  for (size_t i = 0; i < map->capacity; i++) {
    uint8_t m = map->meta[index];
    if ((m & SVMAP_META_STATE_MASK) == SVMAP_META_EMPTY) { return false; }
    if ((m & SVMAP_META_STATE_MASK) == SVMAP_META_OCCUP &&
        (m & SVMAP_META_HASH_MASK) == hMeta) {
      if (svEquals(key, map->keys[index])) {
        map->meta[index] = SVMAP_META_TOMBS;
        map->size--;
        return true;
      }
    }
    index = (index + 1) & (map->capacity - 1);
  }

  return false;
}

BASE_API intptr_t *svMapFindPtr(SvMap *map, Sv key) {
  if (!map) { return NULL; }
  size_t hash = svHash(key);
  size_t index = hash & (map->capacity - 1);
  uint8_t hMeta = ((uint8_t)(hash & SVMAP_META_HASH_MASK));

  for (size_t i = 0; i < map->capacity; i++) {
    uint8_t m = map->meta[index];
    if ((m & SVMAP_META_STATE_MASK) == SVMAP_META_EMPTY) { return NULL; }
    if ((m & SVMAP_META_STATE_MASK) == SVMAP_META_OCCUP &&
        (m & SVMAP_META_HASH_MASK) == hMeta) {
      if (svEquals(key, map->keys[index])) { return &map->values[index]; }
    }
    index = (index + 1) & (map->capacity - 1);
  }

  return NULL;
}

BASE_API intptr_t svMapFind(SvMap *map, Sv key) {
  intptr_t *p = svMapFindPtr(map, key);
  return p != NULL ? *p : SVMAP_INVALID_VALUE;
}

BASE_API bool fsCreateDir(const char *path) {
  if (!path) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  return _mkdir(path) == 0;
#else
  return mkdir(path, 0755) == 0;
#endif
}

BASE_API bool fsExists(const char *path) {
  if (!path) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  DWORD attr = GetFileAttributesA(path);
  return attr != INVALID_FILE_ATTRIBUTES;
#else
  struct stat st;
  return stat(path, &st) == 0;
#endif
}

BASE_API bool fsIsFile(const char *path) {
  if (!path) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  DWORD attr = GetFileAttributesA(path);
  if (attr == INVALID_FILE_ATTRIBUTES) { return false; }
  return (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
#else
  struct stat st;
  if (stat(path, &st) != 0) { return false; }
  return S_ISREG(st.st_mode);
#endif
}

BASE_API bool fsIsDir(const char *path) {
  if (!path) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  DWORD attr = GetFileAttributesA(path);
  if (attr == INVALID_FILE_ATTRIBUTES) { return false; }
  return (attr & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
  struct stat st;
  if (stat(path, &st) != 0) { return false; }
  return S_ISDIR(st.st_mode);
#endif
}

BASE_API bool fsDeleteFile(const char *path) {
  if (!path) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  return DeleteFileA(path) != 0;
#else
  return remove(path) == 0;
#endif
}

BASE_API bool fsCopyFile(const char *src, const char *dst) {
  if (!src || !dst) { return false; }
  FILE *in = fopen(src, "rb");
  if (!in) { return false; }
  FILE *out = fopen(dst, "wb");
  if (!out) { fclose(in); return false; }

  char buf[8192];
  size_t n;
  bool ok = true;
  while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
    if (fwrite(buf, 1, n, out) != n) { ok = false; break; }
  }
  if (ferror(in)) { ok = false; }

  fclose(in);
  fclose(out);
  return ok;
}

BASE_API bool fsMoveFile(const char *src, const char *dst) {
  if (!src || !dst) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  return MoveFileExA(src, dst, MOVEFILE_REPLACE_EXISTING) != 0;
#else
  if (rename(src, dst) == 0) { return true; }
  if (errno == EXDEV) {
    /* crossing filesystems - fall back to copy + delete */
    if (fsCopyFile(src, dst)) { return fsDeleteFile(src); }
  }
  return false;
#endif
}

BASE_API bool fsWalkDir(const char *path, FsWalkFn fn, void *userData, Arena *ar) {
  if (!path || !fn || !ar) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  size_t mark = ar->size;
  char *pattern = arenaAllocFmt(ar, "%s\\*", path);
  if (!pattern) { return false; }
  WIN32_FIND_DATAA fd;
  HANDLE h = FindFirstFileA(pattern, &fd);
  arenaRestoreAt(ar, mark);
  if (h == INVALID_HANDLE_VALUE) { return false; }
  bool cont = true;
  do {
    if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) { continue; }
    size_t entryMark = ar->size;
    char *full = arenaAllocFmt(ar, "%s\\%s", path, fd.cFileName);
    if (!full) { cont = false; break; }
    bool isDir = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    cont = fn(full, isDir, userData);
    if (cont && isDir) { cont = fsWalkDir(full, fn, userData, ar); }
    arenaRestoreAt(ar, entryMark);
  } while (cont && FindNextFileA(h, &fd));

  FindClose(h);
  return true;
#else
  DIR *d = opendir(path);
  if (!d) { return false; }
  bool cont = true;
  struct dirent *entry;
  while (cont && (entry = readdir(d)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) { continue; }
    size_t mark = ar->size;
    char *full = arenaAllocFmt(ar, "%s/%s", path, entry->d_name);
    if (!full) { cont = false; break; }
    struct stat st;
    if (stat(full, &st) != 0) {
      arenaRestoreAt(ar, mark);
      continue;
    }
    bool isDir = S_ISDIR(st.st_mode);
    cont = fn(full, isDir, userData);
    if (cont && isDir) { cont = fsWalkDir(full, fn, userData, ar); }
    arenaRestoreAt(ar, mark);
  }
  closedir(d);
  return true;
#endif
}

BASE_API Sv SvfsGetBaseName(Sv path) {
  for (size_t i = path.size; i > 0; i--) {
    if (path.data[i - 1] == '/') {
      return svFromParts(&path.data[i], path.size - i);
    }
  }
  return path;
}

BASE_API Sv fsGetExtension(Sv path) {
  for (size_t i = path.size; i > 0; i--) {
    char c = path.data[i - 1];
    if (c == '.') {
      /* a leading dot with nothing before it (or right after a '/') is a
       * dotfile like ".gitignore", not an extension */
      if (i - 1 == 0 || path.data[i - 2] == '/') { return SV_NIL; }
      return svFromParts(&path.data[i - 1], path.size - (i - 1));
    }
    if (c == '/') { break; }
  }
  return SV_NIL;
}

#endif /* BASE_IMPLEMENTATION */

/**
 * MIT License
 *
 * Copyright (c) 2026 AndreaTgc
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

