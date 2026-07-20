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
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif /* WIN32_LEAN_AND_MEAN */
  #include <windows.h>
#else /* assumes POSIX */
  #include <pthread.h>
  #include <sched.h>
  #include <time.h>
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

#define MUTEX_SCOPE(mtx) for (int _i = (mutexLock(mtx), 0); !_i; (mutexUnlock(mtx), _i++))

BASE_THREADS_API bool mutexInit   (Mutex *m);
BASE_THREADS_API bool mutexLock   (Mutex *m);
BASE_THREADS_API bool mutexUnlock (Mutex *m);
BASE_THREADS_API void mutexDeinit (Mutex *m);


typedef void *(*ThreadFunc)(void *);

typedef struct {
#if defined(_WIN32) || defined(_WIN64)
  HANDLE handle;
  void *trampolineData;
#else /* assumes POSIX */
  pthread_t handle;
#endif
} Thread;

BASE_THREADS_API bool     threadCreate    (Thread *t, ThreadFunc fn, void *arg);
BASE_THREADS_API bool     threadJoin      (Thread *t, void **retVal);
BASE_THREADS_API bool     threadDetach    (Thread *t);
BASE_THREADS_API bool     threadSleepMs   (uint32_t ms);
BASE_THREADS_API bool     threadYield     (void);
BASE_THREADS_API uint64_t threadCurrentId (void);

typedef struct {
#if defined(_WIN32) || defined(_WIN64)
  CONDITION_VARIABLE handle;
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
  InitializeCriticalSection(&m->handle);
  return true;
#else
  return pthread_mutex_init(&m->handle, NULL) == 0;
#endif
}

BASE_THREADS_API bool mutexLock(Mutex *m) {
  if (!m) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  EnterCriticalSection(&m->handle);
  return true;
#else
  return pthread_mutex_lock(&m->handle) == 0;
#endif
}

BASE_THREADS_API bool mutexUnlock(Mutex *m) {
  if (!m) { return false; }
#if defined(_WIN32) || defined(_WIN64)
  LeaveCriticalSection(&m->handle);
  return true;
#else
  return pthread_mutex_unlock(&m->handle) == 0;
#endif
}

BASE_THREADS_API void mutexDeinit(Mutex *m) {
  if (!m) { return; }
#if defined(_WIN32) || defined(_WIN64)
  DeleteCriticalSection(&m->handle);
#else
  pthread_mutex_destroy(&m->handle);
#endif
}

BASE_THREADS_API bool threadCreate(Thread *t, ThreadFunc fn, void *arg) {
  if (!t) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_create(&t->handle, NULL, fn, arg) == 0;
#endif
}

BASE_THREADS_API bool threadJoin(Thread *t, void **retVal) {
  if (!t) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_join(t->handle, retVal) == 0;
#endif
}

BASE_THREADS_API bool threadDetach(Thread *t) {
  if (!t) { return false; }
#if defined(_WIN32) || defined(_WIN64)
#else
  return pthread_detach(t->handle) == 0;
#endif
}

BASE_THREADS_API bool threadSleepMs(uint32_t ms) {
#if defined(_WIN32) || defined(_WIN64)
#else
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (long)(ms % 1000) * 1000000L;
  return nanosleep(&ts, NULL) == 0;
#endif
}

BASE_THREADS_API bool threadYield(void) {
#if defined(_WIN32) || defined(_WIN64)
#else
  return sched_yield() == 0;
#endif
}

BASE_THREADS_API uint64_t threadCurrentId(void) {
#if defined(_WIN32) || defined(_WIN64)
#else
  return (uint64_t)(uintptr_t)pthread_self();
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
  if (!cv || !m) { return false; }
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
