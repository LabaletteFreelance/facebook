#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdatomic.h>
#include <pthread.h>
#include <limits.h>
#include "liblock.h"


struct cond {
  int cpt;
};

static int futex(int *uaddr, int futex_op, int val) {
  return syscall(SYS_futex, uaddr, futex_op, val, NULL, uaddr, 0);
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) {
  DEBUG_PRINTF("entering %s (cond=%p, mutex=%p)\n", __FUNCTION__, cond, mutex);
  int ret = 0;
  struct cond*c = (struct cond*)cond;
  int cpt = atomic_load(&c->cpt);
  pthread_mutex_unlock(mutex);
  futex(&c->cpt, FUTEX_WAIT, cpt);
  pthread_mutex_lock(mutex);

  DEBUG_PRINTF("leaving %s (cond=%p, mutex=%p). -> %d\n", __FUNCTION__, cond, mutex, ret);
  return ret;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime) {
  DEBUG_PRINTF("entering %s (cond=%p, mutex=%p, abstime=%p)\n", __FUNCTION__, cond, mutex, abstime);
  int ret = 0;
  fprintf(stderr, "NOT IMPLEMENTED\n");
  abort();
  DEBUG_PRINTF("leaving %s (cond=%p, mutex=%p, abstime=%p). -> %d\n", __FUNCTION__, cond, mutex, abstime, ret);
  return ret;
}

int pthread_cond_signal(pthread_cond_t *cond) {
  DEBUG_PRINTF("entering %s (cond=%p)\n", __FUNCTION__, cond);
  struct cond*c = (struct cond*)cond;
  int ret = 0;
  atomic_fetch_add(&c->cpt, 1);
  futex(&c->cpt, FUTEX_WAKE, 1);
  DEBUG_PRINTF("leaving %s (cond=%p). -> %d\n", __FUNCTION__, cond, ret);
  return ret;
}

int pthread_cond_broadcast(pthread_cond_t *cond) {
  DEBUG_PRINTF("entering %s (cond=%p)\n", __FUNCTION__, cond);
  struct cond*c = (struct cond*)cond;
  int ret = 0;
  atomic_fetch_add(&c->cpt, 1);
  futex(&c->cpt, FUTEX_WAKE, INT_MAX);
 
  DEBUG_PRINTF("leaving %s (cond=%p). -> %d\n", __FUNCTION__, cond, ret);
  return ret;
}

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) {
  DEBUG_PRINTF("entering %s (cond=%p, attr=%p)\n", __FUNCTION__, cond, attr);
  int ret = 0;
  struct cond*c = (struct cond*)cond;
  c->cpt = 0;
  DEBUG_PRINTF("leaving %s (cond=%p, attr=%p). -> %d\n", __FUNCTION__, cond, attr, ret);
  return ret;
}

int pthread_cond_destroy(pthread_cond_t *cond) {
  DEBUG_PRINTF("entering %s (cond=%p)\n", __FUNCTION__, cond);
  int ret = 0;
  DEBUG_PRINTF("leaving %s (cond=%p). -> %d\n", __FUNCTION__, cond, ret);
  return ret;
}

static void __cond_init(void) __attribute__((constructor));
static void __cond_init(void) {
  DEBUG_PRINTF("[LIMYBLOCK] initializing the cond library\n");
}

static void __cond_conclude(void) __attribute__((destructor));
static void __cond_conclude(void) {
  DEBUG_PRINTF("[LIMYBLOCK] finalizing the cond library\n");
}
