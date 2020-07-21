#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <pthread.h>
#include "liblock.h"

int (*pthread_cond_wait_original)(pthread_cond_t *cond, pthread_mutex_t *mutex);
int (*pthread_cond_timedwait_original)(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
int (*pthread_cond_signal_original)(pthread_cond_t *cond);
int (*pthread_cond_broadcast_original)(pthread_cond_t *cond);
int (*pthread_cond_init_original)(pthread_cond_t *cond, const pthread_condattr_t *attr);
int (*pthread_cond_destroy_original)(pthread_cond_t *cond);

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  DEBUG_PRINTF("entering %s (cond=%p, mutex=%p)\n", __FUNCTION__, cond, mutex);
  int ret = pthread_cond_wait_original(cond, mutex);
  DEBUG_PRINTF("leaving %s (cond=%p, mutex=%p). -> %d\n", __FUNCTION__, cond, mutex, ret);
  return ret;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
  DEBUG_PRINTF("entering %s (cond=%p, mutex=%p, abstime=%p)\n", __FUNCTION__, cond, mutex, abstime);
  int ret = pthread_cond_timedwait_original(cond, mutex, abstime);
  DEBUG_PRINTF("leaving %s (cond=%p, mutex=%p, abstime=%p). -> %d\n", __FUNCTION__, cond, mutex, abstime, ret);
  return ret;
}

int pthread_cond_signal(pthread_cond_t *cond) {
  DEBUG_PRINTF("entering %s (cond=%p)\n", __FUNCTION__, cond);
  int ret = pthread_cond_signal_original(cond);
  DEBUG_PRINTF("leaving %s (cond=%p). -> %d\n", __FUNCTION__, cond, ret);
  return ret;
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
  DEBUG_PRINTF("entering %s (cond=%p)\n", __FUNCTION__, cond);
  int ret = pthread_cond_broadcast_original(cond);
  DEBUG_PRINTF("leaving %s (cond=%p). -> %d\n", __FUNCTION__, cond, ret);
  return ret;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  DEBUG_PRINTF("entering %s (cond=%p, attr=%p)\n", __FUNCTION__, cond, attr);
  int ret = pthread_cond_init_original(cond, attr);
  DEBUG_PRINTF("leaving %s (cond=%p, attr=%p). -> %d\n", __FUNCTION__, cond, attr, ret);
  return ret;
}

int pthread_cond_destroy(pthread_cond_t *cond) {
  DEBUG_PRINTF("entering %s (cond=%p)\n", __FUNCTION__, cond);
  int ret = pthread_cond_destroy_original(cond);
  DEBUG_PRINTF("leaving %s (cond=%p). -> %d\n", __FUNCTION__, cond, ret);
  return ret;
}

static void __cond_init(void) __attribute__((constructor));
static void __cond_init(void) {
  DEBUG_PRINTF("[LIBLOCK] initializing the cond library\n");

  pthread_cond_wait_original = get_callback("pthread_cond_wait");
  pthread_cond_timedwait_original = get_callback("pthread_cond_timedwait");
  pthread_cond_signal_original = get_callback("pthread_cond_signal");
  pthread_cond_broadcast_original = get_callback("pthread_cond_broadcast");
  pthread_cond_init_original = get_callback("pthread_cond_init");
  pthread_cond_destroy_original = get_callback("pthread_cond_destroy");
}

static void __cond_conclude(void) __attribute__((destructor));
static void __cond_conclude(void) {
  DEBUG_PRINTF("[LIBLOCK] finalizing the cond library\n");
}
