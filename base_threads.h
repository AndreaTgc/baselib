/**
 * Base_Threads.h - simple thread abstraction for POSIX and windows.
 * Author: AndreaTgc
 * Version: 0.1.0
 *
 * Description: This file contains a thin abstraction layer over POSIX and
 * windows threads; the goal is to ensure that this library can be used in a way
 * that leads to the vast majority of the code being platform agnostic.
 * The struct definitions are visible in the header section just to allow stack
 * allocation of these types, but accessing them and interacting with them using
 * platform specific functions leads to less portable code
 */
#ifndef BASE_THREADS_H_
#define BASE_THREADS_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef BASE_THREADS_STATIC
  #define BASE_THREADS_API static
#else
  #define BASE_THREADS_API extern
#endif /* BASE_THREADS_STATIC */

#if defined(_WIN32) || defined(_WIN64)
#else /* assumes POSIX */
  #include <pthread.h>
#endif

#if defined(_MSC_VER)
  #define THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__) || defined(__clang__)
  #define THREAD_LOCAL __thread
#else
  #define THREAD_LOCAL _Thread_local
#endif

typedef struct {
#if defined(_WIN32) || defined(_WIN64)
  CRITICAL_SECTION handle;
#else /* assumes POSIX */
  pthread_mutex_t handle;
#endif
} Mutex;

#define MUTEX_SCOPE(mtx) DEFER(mutexLock(mtx), mutexUnlock(mtx))

BASE_THREADS_API bool mutexInit   (Mutex *m);
BASE_THREADS_API bool mutexLocK   (Mutex *m);
BASE_THREADS_API bool mutexUnlock (Mutex *m);
BASE_THREADS_API void mutexDeinit (Mutex *m);

typedef struct {
#if defined(_WIN32) || defined(_WIN64)
  THREAD handle;
  void *trampolineData;
#else /* assumes POSIX */
  pthread_t handle;
#endif
} Thread;


typedef struct {
#if defined(_WIN32) || defined(_WIN64)
  CONDITIONAL_VARIABLE handle;
#else /* assumes POSIX */
  pthread_cond_t handle;
#endif
} CondVar;

BASE_THREADS_API bool condVarInit      (CondVar *cv);
BASE_THREADS_API bool condVarWait      (CondVar *cv, Mutex *m);
BASE_THREADS_API bool condVarTimedWait (CondVar *cv, Mutex *m, uint32_t ms);
BASE_THREADS_API bool condVarSignal    (CondVar *cv);
BASE_THREADS_API bool condVarBroadcast (CondVar *cv);
BASE_THREADS_API void condVarDeinit    (CondVar *cv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BASE_THREADS_H_ */

#ifdef BASE_THREADS_IMPLEMENTATION

BASE_THREADS_API bool mutexInit(Mutex *m) {
  if (!m) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_mutex_init(&m->handle, NULL) == 0;
#endif
}

BASE_THREADS_API bool mutexLock(Mutex *m) {
  if (!m) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_mutex_lock(&m->handle) == 0;
#endif
}

BASE_THREADS_API bool mutexUnlock(Mutex *m) {
  if (!m) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_mutex_unlock(&m->handle) == 0;
#endif
}

BASE_THREADS_API void mutexDeinit(Mutex *m) {
  if (!m) { return; }
#if defined(_WIN32) || defined(_WIN64)
#else
  pthread_mutex_destroy(&m->handle);
#endif
}

BASE_THREADS_API bool condVarInit(CondVar *cv) {
  if (!cv) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_cond_init(&cv->handle, NULL) == 0;
#endif
}

BASE_THREADS_API bool condVarWait(CondVar *cv, Mutex *m) {
  if (!cv || !m) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_cond_wait(&cv->handle, &m->handle) == 0;
#endif
}

BASE_THREADS_API bool condVarTimedWait(CondVar *cv, Mutex *m, uint32_t ms) {
  if (!cv) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += ms / 1000;
  ts.tv_nsec += (long)(ms % 1000) * 1000000L;
  if (ts.tv_nsec >= 1000000000L) { ts.tv_sec++; ts.tv_nsec -= 1000000000L; }
  return pthread_cond_timedwait(&cv->handle, &m->handle, &ts) == 0;
#endif
}

BASE_THREADS_API bool condVarSignal(CondVar *cv) {
  if (!cv) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_cond_signal(&cv->handle) == 0;
#endif
}

BASE_THREADS_API bool condVarBroadcast(CondVar *cv) {
  if (!cv) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_cond_broadcast(&cv->handle) == 0;
#endif
}

BASE_THREADS_API void condVarDeinit(CondVar *cv) {
  if (!cv) { return; }
#if defined(_WIN32) || defined(_WIN64)
#else
  pthread_cond_destroy(&cv->handle);
#endif
}


#endif /* BASE_THREADS_IMPLEMENTATION */
