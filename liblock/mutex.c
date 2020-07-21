#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <pthread.h>

#include "liblock.h"

int (*pthread_mutex_lock_original)(pthread_mutex_t *mutex);
int (*pthread_mutex_trylock_original)(pthread_mutex_t *mutex);
int (*pthread_mutex_unlock_original)(pthread_mutex_t *mutex);
int (*pthread_mutex_init_original)(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int (*pthread_mutex_destroy_original)(pthread_mutex_t *mutex);

int pthread_mutex_lock(pthread_mutex_t *mutex) {
  DEBUG_PRINTF("entering %s(mutex=%p)\n", __FUNCTION__, mutex);
  int ret = pthread_mutex_lock_original(mutex);
  DEBUG_PRINTF("leaving %s(mutex=%p) -> %d\n", __FUNCTION__, mutex, ret);
  return ret;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
  DEBUG_PRINTF("entering %s(mutex=%p)\n", __FUNCTION__, mutex);
  int ret = pthread_mutex_trylock_original(mutex);
  DEBUG_PRINTF("leaving %s(mutex=%p) -> %d\n", __FUNCTION__, mutex, ret);
  return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
  DEBUG_PRINTF("entering %s(mutex=%p\n", __FUNCTION__, mutex);
  int ret = pthread_mutex_unlock_original(mutex);
  DEBUG_PRINTF("leaving %s(mutex=%p)-> %d\n", __FUNCTION__, mutex, ret);
  return ret;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
  DEBUG_PRINTF("entering %s(mutex=%p, attr=%p)\n", __FUNCTION__, mutex, attr);
  int ret = pthread_mutex_init_original(mutex, attr);
  DEBUG_PRINTF("leaving %s(mutex=%p, attr=%p) -> %d\n", __FUNCTION__, mutex, attr, ret);
  return ret;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex){
  DEBUG_PRINTF("entering %s(mutex=%p)\n", __FUNCTION__, mutex);
  int ret = pthread_mutex_destroy_original(mutex);
  DEBUG_PRINTF("leaving %s(mutex=%p) -> %d\n", __FUNCTION__, mutex, ret);
  return ret;
}


static void __mutex_init(void) __attribute__((constructor));
static void __mutex_init(void) {
  DEBUG_PRINTF("[LIBLOCK] initializing the mutex library\n");

  pthread_mutex_lock_original = get_callback("pthread_mutex_lock");
  pthread_mutex_trylock_original = get_callback("pthread_mutex_trylock");
  pthread_mutex_unlock_original = get_callback("pthread_mutex_unlock");
  pthread_mutex_init_original = get_callback("pthread_mutex_init");
  pthread_mutex_destroy_original = get_callback("pthread_mutex_destroy");
}

static void __mutex_conclude(void) __attribute__((destructor));
static void __mutex_conclude(void) {
  DEBUG_PRINTF("[LIBLOCK] finalizing the mutex library\n");
}
