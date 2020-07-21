#ifndef MEMCACHE_H
#define MEMCACHE_H

#include <pthread.h>
#include "arbreAVL.h"

struct memcache_t {
  struct Node *root;
  pthread_rwlock_t lock;
};

/* initialize a memcache */
void  memcache_init(struct memcache_t* cache);

/* get the value associated with key */
void* memcache_get(struct memcache_t* cache, const char* key);

/* associate key with value */
void  memcache_set(struct memcache_t* cache, const char* key, void* value);

#endif
