#include "memcache.h"


/* initialize a memcache */
void  memcache_init(struct memcache_t* cache) {
  cache->root = NULL;
  pthread_rwlock_init(&cache->lock, NULL);
}

/* get the value associated with key */
void* memcache_get(struct memcache_t* cache, const char* key) {
  pthread_rwlock_rdlock(&cache->lock);
  void* retval = getValue(cache->root, key);
  pthread_rwlock_unlock(&cache->lock);
  return retval;
}

/* associate key with value */
void  memcache_set(struct memcache_t* cache, const char* key, void* value) {
  pthread_rwlock_wrlock(&cache->lock);
  cache->root = insert(cache->root, key, value);
  pthread_rwlock_unlock(&cache->lock);
}
