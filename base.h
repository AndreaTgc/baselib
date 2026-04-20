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
 * - Helpers for vector math (up to 4D)
 *
 * LICENSING:
 * This library is distributed with the MIT license. See the bottom of this
 * file for the license.
 */
#ifndef BASE_H__
#define BASE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <math.h>
#include <ctype.h>
#include <stdio.h>
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
 * terminated but it can be converted to a cstr string using strBuilderToCStr
 * and you're going to be able to pass the "data" field to C functions that
 * expect a null terminated buffer.
 * If you push some content onto the string builder after converting it to a
 * cstr, the content is not going to be null terminated anymore, you should
 * treat strBuilderToCStr as a utility that you call only when you need to
 * interact with functions that require null terminators.
 */

typedef DA(char) StrBuilder;

BASE_API void strBuilderAppendCStr  (StrBuilder *sb, const char *cstr);
BASE_API void strBuilderAppendSlice (StrBuilder *sb, const char *ptr, size_t len);
BASE_API void strBuilderToCStr      (StrBuilder *sb);

#define strBuilderFree(sb) daFree(sb)

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
 * The following are some very simple utilities for vector calculations.
 * I decided to have them return the result by value instead of modifying the
 * structs simply because they are so small there is no real benefit to it.
 * The vectors are supported up to 4D which should cover 99% of all the use
 * cases i actually care about
 */

typedef struct { float x, y; } Vec2;

BASE_API Vec2  vec2Add       (Vec2 a, Vec2 b);
BASE_API Vec2  vec2Sub       (Vec2 a, Vec2 b);
BASE_API Vec2  vec2Mul       (Vec2 a, Vec2 b);
BASE_API Vec2  vec2Scale     (Vec2 a, float s);
BASE_API Vec2  vec2Rotate    (Vec2 a, float rads);
BASE_API Vec2  vec2Normalize (Vec2 a);
BASE_API float vec2Dot       (Vec2 a, Vec2 b);
BASE_API float vec2Len       (Vec2 a);
BASE_API float vec2LenSq     (Vec2 a);
BASE_API float vec2Distance  (Vec2 a, Vec2 b);

typedef struct { float x, y, z; } Vec3;

BASE_API Vec2  vec3ToVec2    (Vec3 a);
BASE_API Vec3  vec3FromVec2  (Vec2 a, float z);
BASE_API Vec3  vec3Add       (Vec3 a, Vec3 b);
BASE_API Vec3  vec3Sub       (Vec3 a, Vec3 b);
BASE_API Vec3  vec3Mul       (Vec3 a, Vec3 b);
BASE_API Vec3  vec3Cross     (Vec3 a, Vec3 b);
BASE_API Vec3  vec3Scale     (Vec3 a, float s);
BASE_API Vec3  vec3Rotate    (Vec3 a, Vec3 axis, float rads);
BASE_API Vec3  vec3Normalize (Vec3 a);
BASE_API float vec3Dot       (Vec3 a, Vec3 b);
BASE_API float vec3Len       (Vec3 a);
BASE_API float vec3LenSq     (Vec3 a);
BASE_API float vec3Distance  (Vec3 a, Vec3 b);

typedef struct { float x, y, z, w; } Vec4;


BASE_API Vec3  vec4ToVec3    (Vec4 a);
BASE_API Vec4  vec4FromVec3  (Vec3 a, float w);
BASE_API Vec4  vec4Add       (Vec4 a, Vec4 b);
BASE_API Vec4  vec4Sub       (Vec4 a, Vec4 b);
BASE_API Vec4  vec4Mul       (Vec4 a, Vec4 b);
BASE_API Vec4  vec4Scale     (Vec4 a, float s);
BASE_API Vec4  vec4Normalize (Vec4 a);
BASE_API float vec4Dot       (Vec4 a, Vec4 b);
BASE_API float vec4Len       (Vec4 a);
BASE_API float vec4LenSq     (Vec4 a);
BASE_API float vec4Distance  (Vec4 a, Vec4 b);

typedef struct { float x, y, z, w; } Quat;

BASE_API Quat  quatIdentity      (void);
BASE_API Quat  quatFromAxisAngle (Vec3 axis, float rads);
BASE_API Quat  quatFromEuler     (float pitch, float yaw, float roll);
BASE_API Quat  quatAdd           (Quat a, Quat b);
BASE_API Quat  quatSub           (Quat a, Quat b);
BASE_API Quat  quatMul           (Quat a, Quat b);
BASE_API Quat  quatScale         (Quat a, float s);
BASE_API Quat  quatConjugate    (Quat a);
BASE_API Quat  quatInverse       (Quat a);
BASE_API Quat  quatNormalize     (Quat a);
BASE_API Vec3  quatRotateVec3    (Quat a, Vec3 v);
BASE_API float quatDot           (Quat a, Quat b);
BASE_API float quatLen           (Quat a);
BASE_API float quatLenSq         (Quat a);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASE_H__ */

#ifdef BASE_IMPLEMENTATION

BASE_API void arenaInit(Arena *ar, size_t bytes) {
  BASE_ASSERT(ar);
  MEMZERO(ar, sizeof(*ar));
  ar->capacity = bytes;
  ar->data = (uint8_t*)calloc(sizeof(uint8_t), bytes);
}

BASE_API void arenaInitFromBuffer(Arena *ar, void *buf, size_t bytes) {
  BASE_ASSERT(ar);
  ar->size = 0;
  ar->capacity = bytes;
  ar->data = buf;
}

BASE_API void *arenaAllocAligned(Arena *ar, size_t bytes, size_t align) {
  BASE_ASSERT(ar);
  BASE_ASSERT(align > 0 && IS_POW2(align));
  size_t offset = ALIGN_UP(ar->size, align);
  if (offset + bytes > ar->capacity) return NULL;
  void *p  = ar->data + offset;
  ar->size = offset + bytes;
  return p;
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
  MEMZERO(ar, sizeof(*ar));
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
    fclose(f);
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

BASE_API void strBuilderAppendCStr(StrBuilder *sb, const char *cstr) {
  BASE_ASSERT(sb && cstr);
  size_t slen = strlen(cstr);
  for (size_t i = 0; i < slen; i++) {
    daAppend(sb, cstr[i]);
  }
}

BASE_API void strBuilderAppendSlice(StrBuilder *sb, const char *ptr, size_t len) {
  BASE_ASSERT(sb && ptr);
  for (size_t i = 0; i < len; i++) {
    daAppend(sb, ptr[i]);
  }
}

BASE_API void strBuilderToCStr(StrBuilder *sb) {
  BASE_ASSERT(sb);
  daAppend(sb, '\0');
  sb->size--;
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

BASE_API Vec2 vec2Add(Vec2 a, Vec2 b) {
  return (Vec2) { a.x + b.x, a.y + b.y };
}

BASE_API Vec2 vec2Sub(Vec2 a, Vec2 b) {
  return (Vec2) { a.x - b.x, a.y - b.y };
}

BASE_API Vec2 vec2Mul(Vec2 a, Vec2 b) {
  return (Vec2) { a.x * b.x, a.y * b.y };
}

BASE_API Vec2  vec2Scale(Vec2 a, float s) {
  return (Vec2) { a.x * s, a.y * s };  
}
  
BASE_API Vec2 vec2Rotate(Vec2 a, float rads) {
  float c = cosf(rads);
  float s = sinf(rads);
  return (Vec2) { a.x * c - a.y * s, a.x * s + a.y * c };
}

BASE_API Vec2 vec2Normalize (Vec2 a) {
  float lsq = vec2LenSq(a);
  if (lsq == 0.f) return (Vec2) { 0 };
  return vec2Scale(a, 1.f / sqrtf(lsq));
}

BASE_API float vec2Dot(Vec2 a, Vec2 b) {
  return a.x * b.x + a.y * b.y;
}

BASE_API float vec2Len(Vec2 a) {
  return sqrtf(vec2LenSq(a));
}

BASE_API float vec2LenSq(Vec2 a) {
  return a.x * a.x + a.y * a.y;
}

BASE_API float vec2Distance(Vec2 a, Vec2 b) {
  return vec2Len(vec2Sub(a, b));
}

BASE_API Vec2 vec3ToVec2(Vec3 a) {
  return (Vec2) { a.x, a.y };
}

BASE_API Vec3 vec3FromVec2(Vec2 a, float z) {
  return (Vec3) { a.x, a.y, z };
}

BASE_API Vec3 vec3Add(Vec3 a, Vec3 b) {
  return (Vec3) { a.x + b.x, a.y + b.y, a.z + b.z };
}

BASE_API Vec3 vec3Sub(Vec3 a, Vec3 b) {
  return (Vec3) { a.x - b.x, a.y - b.y, a.z - b.z };
}

BASE_API Vec3 vec3Mul(Vec3 a, Vec3 b) {
  return (Vec3) { a.x * b.x, a.y * b.y, a.z * b.z };
}

BASE_API Vec3 vec3Cross(Vec3 a, Vec3 b) {
  return (Vec3) {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x,
  };
}

BASE_API Vec3 vec3Scale(Vec3 a, float s) {
  return (Vec3) { a.x * s, a.y * s, a.z * s };
}

BASE_API Vec3 vec3Rotate(Vec3 a, Vec3 axis, float rads) {
  axis = vec3Normalize(axis);
  Vec3 aPar = vec3Scale(axis, vec3Dot(a, axis));
  Vec3 aPerp = vec3Sub(a, aPar);
  Vec3 w = vec3Cross(axis, aPerp);
  return (Vec3) {
    aPar.x + cosf(rads) * aPerp.x + sinf(rads) * w.x,
    aPar.y + cosf(rads) * aPerp.y + sinf(rads) * w.y,
    aPar.z + cosf(rads) * aPerp.z + sinf(rads) * w.z,
  };
}

BASE_API Vec3 vec3Normalize(Vec3 a) {
  float lsq = vec3LenSq(a);
  if (lsq == 0.f) return (Vec3) { 0 };
  return vec3Scale(a, 1.f / sqrtf(lsq));
}

BASE_API float vec3Dot(Vec3 a, Vec3 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

BASE_API float vec3Len(Vec3 a) {
  return sqrtf(vec3LenSq(a));
}

BASE_API float vec3LenSq(Vec3 a) {
  return a.x * a.x + a.y * a.y + a.z * a.z;
}

BASE_API float vec3Distance(Vec3 a, Vec3 b) {
  return vec3Len(vec3Sub(a, b));
}

BASE_API Vec3 vec4ToVec3(Vec4 a) {
  return (Vec3) { a.x, a.y, a.z };
}

BASE_API Vec4 vec4FromVec3(Vec3 a, float w) {
  return (Vec4) { a.x, a.y, a.z, w };
}

BASE_API Vec4 vec4Add(Vec4 a, Vec4 b) {
  return (Vec4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

BASE_API Vec4 vec4Sub(Vec4 a, Vec4 b) {
  return (Vec4) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

BASE_API Vec4 vec4Mul(Vec4 a, Vec4 b) {
  return (Vec4) { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

BASE_API Vec4 vec4Scale(Vec4 a, float s) {
  return (Vec4) { a.x * s, a.y * s, a.z * s, a.w * s };
}

BASE_API Vec4 vec4Normalize (Vec4 a) {
  float lsq = vec4LenSq(a);
  if (lsq == 0.f) return (Vec4) { 0 };
  return vec4Scale(a, 1.f / sqrtf(lsq));
}

BASE_API float vec4Dot(Vec4 a, Vec4 b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

BASE_API float vec4Len(Vec4 a) {
  return sqrtf(vec4LenSq(a));
}

BASE_API float vec4LenSq(Vec4 a) {
  return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
}

BASE_API float vec4Distance(Vec4 a, Vec4 b) {
  return vec4Len(vec4Sub(a, b));
}

BASE_API Quat quatIdentity(void) {
  return (Quat) { 0.f, 0.f, 0.f, 1.f };
}
BASE_API Quat quatFromAxisAngle(Vec3 axis, float rads) {
  axis = vec3Normalize(axis);
  float s = sinf(rads * 0.5f);
  float c = cosf(rads * 0.5f);
  return (Quat) { axis.x * s, axis.y * s, axis.z * s, c };
}

BASE_API Quat quatFromEuler(float pitch, float yaw, float roll) {
  float cosP = cosf(pitch * 0.5f);
  float sinP = sinf(pitch * 0.5f);
  float cosY = cosf(yaw * 0.5f);
  float sinY = sinf(yaw * 0.5f);
  float cosR = cosf(roll * 0.5f);
  float sinR = sinf(roll * 0.5f);

  return (Quat) {
    sinR * cosP * cosY - cosR * sinP * sinY,
    cosR * sinP * cosY - sinR * cosP * sinY,
    cosR * cosP * sinY - sinR * sinP * cosY,
    cosR * cosP * cosY - sinR * sinP * sinY
  };
}

BASE_API Quat quatAdd(Quat a, Quat b) {
  return (Quat) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }; 
}

BASE_API Quat quatSub(Quat a, Quat b) {
  return (Quat) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

BASE_API Quat quatMul(Quat a, Quat b) {
  return (Quat) {
    a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
    a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
    a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
    a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
  }; 
}

BASE_API Quat quatScale(Quat a, float s) {
  return (Quat) { a.x * s, a.y * s, a.z * s, a.w * s };
}

BASE_API Quat quatConjugate(Quat a) {
  return (Quat) { -a.x, -a.y, -a.z, a.w };
}

BASE_API Quat quatInverse(Quat a) {
  float lsq = quatLenSq(a);
  if (lsq == 0.f) return quatIdentity();
  return quatScale(quatConjugate(a), 1.f / lsq);
}

BASE_API Quat quatNormalize(Quat a) {
  float lsq = quatLenSq(a);
  if (lsq == 0.f) return quatIdentity();
  return quatScale(a, 1.f / sqrtf(lsq));
}

BASE_API Vec3 quatRotateVec3(Quat a, Vec3 v) {
  Vec3 vecQuat = (Vec3) { a.x, a.y, a.z };
  Vec3 t = vec3Scale(vec3Cross(vecQuat, v), 2.f);
  Vec3 u = vec3Cross(vecQuat, t);
  return (Vec3) {
    v.x + a.w * t.x + u.x,
    v.y + a.w * t.y + u.y,
    v.z + a.w * t.z + u.z,
  };
}

BASE_API float quatDot(Quat a, Quat b) {
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

BASE_API float quatLen(Quat a) {
  return sqrtf(quatLenSq(a));
}

BASE_API float quatLenSq(Quat a) {
  return a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w;
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
