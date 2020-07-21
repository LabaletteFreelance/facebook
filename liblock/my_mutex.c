#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdatomic.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <errno.h>
#include <pthread.h>
#include "liblock.h"

struct mutex {
  int flag;
};

static int futex(int *uaddr, int futex_op, int val,
		 const struct timespec *timeout, int *uaddr2, int val3) {
  return syscall(SYS_futex, uaddr, futex_op, val,
		 timeout, uaddr, val3);
}

int (*pthread_mutex_lock_original)(pthread_mutex_t *mutex);
int (*pthread_mutex_trylock_original)(pthread_mutex_t *mutex);
int (*pthread_mutex_unlock_original)(pthread_mutex_t *mutex);
int (*pthread_mutex_init_original)(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int (*pthread_mutex_destroy_original)(pthread_mutex_t *mutex);

int pthread_mutex_lock(pthread_mutex_t *mutex) {

  struct mutex* l = (struct mutex*) mutex;
  int ret = 0;
  DEBUG_PRINTF("entering %s (mutex=%p, l=%d)\n", __FUNCTION__, mutex, l->flag);

  while(1) {
    int expected = 1;
    if (atomic_compare_exchange_strong(&l->flag, &expected, 0)) {
      ret = 0;      /* Yes */
      goto out;
    }

    /* Futex is not available; wait */
    int s = futex(&l->flag, FUTEX_WAIT, 0, NULL, NULL, 0);
    if (s == -1 && errno != EAGAIN) {
      ret = 1;
      goto out;
    }
  }
 out:
  DEBUG_PRINTF("leaving %s (mutex=%p, l=%d) -> %d\n", __FUNCTION__, mutex, l->flag, ret);
  return ret;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) {
  int ret = 1;
  struct mutex* l = (struct mutex*) mutex;
  DEBUG_PRINTF("entering %s(mutex=%p, l=%d)\n", __FUNCTION__, mutex, l->flag);
  int expected = 1;
  if (atomic_compare_exchange_strong(&l->flag, &expected, 0)) {
    /* mutex was acquired */
    ret = 0;
    goto out;
  }

  /* mutex was not acquired */
  ret = 1;
 out:
  DEBUG_PRINTF("leaving %s(mutex=%p, l=%d) -> %d\n", __FUNCTION__, mutex, l->flag, ret);
  return ret;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex){
  int expected = 0;
  int ret = 0;
  struct mutex* l = (struct mutex*) mutex;
  DEBUG_PRINTF("entering %s (mutex=%p, l=%d)\n", __FUNCTION__, mutex, l->flag);
  atomic_compare_exchange_strong(&l->flag, &expected, 1);
  int s = futex(&l->flag, FUTEX_WAKE, 1, NULL, NULL, 0);
  if (s  == -1) {
    ret = 1;
    goto out;
  }
 out:
  DEBUG_PRINTF("leaving %s (mutex=%p, l= %d) -> %d\n", __FUNCTION__, mutex, l->flag, ret);
  return ret;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
  struct mutex* l = (struct mutex*) mutex;
  DEBUG_PRINTF("entering %s(mutex=%p, attr=%p)\n", __FUNCTION__, mutex, attr);
  int ret = 0;
  l->flag = 1;
  DEBUG_PRINTF("leaving %s(mutex=%p, l=%d, attr=%p) -> %d\n", __FUNCTION__, mutex, l->flag, attr, ret);
  return ret;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex){
  DEBUG_PRINTF("entering %s(mutex=%p)\n", __FUNCTION__, mutex);
  DEBUG_PRINTF("leaving %s(mutex=%p)\n", __FUNCTION__, mutex);
  return 0;
}

static void __mutex_init(void) __attribute__((constructor));
static void __mutex_init(void) {
  DEBUG_PRINTF("[LIMYBLOCK] initializing the mutex library\n");
}

static void __mutex_conclude(void) __attribute__((destructor));
static void __mutex_conclude(void) {
  DEBUG_PRINTF("[LIMYBLOCK] finalizing the mutex library\n");
}
