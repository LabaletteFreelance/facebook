#ifndef LIBLOCK_H
#define LIBLOCK_H

#define DEBUG 1

#if DEBUG
#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF(...) (void) 0
#endif


static void* get_callback(const char*fname) __attribute__((unused));
static void* get_callback(const char*fname){
  void* ret = dlsym(RTLD_NEXT, fname);
  if(!ret) {
    fprintf(stderr, "Warning: cannot find %s: %s\n", fname, dlerror());
  }
  return ret;
}

#endif
