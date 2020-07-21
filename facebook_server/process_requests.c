#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <sys/time.h>
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>

#include "server.h"
#include "process_requests.h"
#include "memcache.h"

/* append a string to the reply */
#define APPEND_TEXT(REPLY, ...) do {					\
    len = snprintf(&REPLY[index], MAX_REPLY_LENGTH-index, __VA_ARGS__); \
    index += len;							\
  }while(0)

#define TIME_DIFF(t1, t2) (((t2).tv_sec-(t1).tv_sec)*1e6+((t2).tv_usec-(t1).tv_usec))

#define MAX_USERNAME_LENGTH 128
#define MAX_NB_FRIENDS 100
#define MAX_NB_MESSAGES 100
#define MAX_NB_USERS 100

struct message {
  int from;
  char msg[MAX_REPLY_LENGTH];
};

struct user {  
  int id;
  char name[MAX_USERNAME_LENGTH];

  int nb_friends;
  int friends[MAX_NB_FRIENDS];

  int nb_messages;
  struct message messages[MAX_NB_MESSAGES];

  struct timeval last_modification;
};


struct shm {
  int nb_users;
  struct user users[MAX_NB_USERS];
  _Atomic int compteur_reponse;
  pthread_rwlock_t big_lock;
};

#define SHM_SIZE sizeof(struct shm)
struct shm*shm = NULL;

int fd;
struct memcache_t memcache;

void init_shm() {
  if(shm)
    /* already initialized */
    return;

  /* open the shared memory segment */
  fd = shm_open("/FaceBook", O_CREAT| O_EXCL | O_RDWR , 0666);
  if(fd<0 && errno == EEXIST) {
    fd = shm_open("/FaceBook", O_CREAT | O_RDWR , 0666);
    if(fd<0) {
      perror("shm_open");
      exit(EXIT_FAILURE);
    }
    
    /* the shared memory region already exists, don't initialize everything */
    if((shm = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) <0) {
      perror("mmap");
      exit(EXIT_FAILURE);
    }

    if(verbose)
      printf("Restoring the server data with %d users\n", shm->nb_users);
    return;
  } else if(fd<0) {
    perror("shm_open");
    exit(EXIT_FAILURE);
  }

  /* set the size of the shared memory segment */
  if(ftruncate(fd, SHM_SIZE) < 0) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  /* map the shared memory segment in the current address space */
  if((shm = mmap(NULL, SHM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) <0) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }

  shm->nb_users = 0;
  shm->compteur_reponse = 0;
}

void finalize() {
  munmap(shm, SHM_SIZE);
  close(fd);
}

/* Procedure chargee de toutes les initialisations specifiques */
void init(){

  init_shm();
  atexit(finalize);
  pthread_rwlock_init(&shm->big_lock, NULL);
  memcache_init(&memcache);
}

void init_user(struct user*u){
  static int next_id = 0;
  u->id = next_id++;

  strcpy(u->name, "");
  u->nb_friends = 0;
  u->nb_messages=0;
}

struct user* search_user(int id) {
  for(int i=0; i<shm->nb_users; i++) {
    if(shm->users[i].id == id) {
      return &shm->users[i];
    }
  }

  return NULL;
}

struct user* search_user_name(const char* name) {
  for(int i=0; i<shm->nb_users; i++) {
    if(strcmp(shm->users[i].name, name) == 0) {
      return &shm->users[i];
    }
  }

  return NULL;
}

/* return 1 if f is a friend of u */
int is_friend(struct user* u, struct user* f) {
  for(int i=0; i<u->nb_friends; i++) {
    if(u->friends[i] == f->id)
      return 1;
  }
  return 0;
}

void do_help(struct page_request* page_request) {
  struct page_reply* reply = memcache_get(&memcache, page_request->url);
  if(! reply) {
    printf("%s: cannot find page '%s' in the cache\n", __func__, page_request->url);
    reply = malloc(sizeof(struct page_request));
    gettimeofday(&reply->timestamp, NULL);  

    static char* help_response =
      "<h1>Help</h1>\n"
      "Here are the available commands:"
      "<ul>\n"
      "<li><code><a href='help'>help</a></code>: print this message</li>\n"
      "<li><code><a href='hello?user=0'>hello[?user=&lt;userid&gt;]</a></code>: say hello to <code>userid</code></li>\n"
      "<li><code><a href='list_users'>list_users</a></code>: print the list of created users</li>\n"
      "<li><code><a href='add_user?user=someone'>add_user?user=&lt;someone&gt;</a></code>: create a new user</li>\n"
      "<li><code><a href='view_user?user=0'>view_user?user=&lt;userid&gt;</a></code>: view a user</li>\n"
      "<li><code><a href='add_friend?user=0&friend=1'>add_friend?user=&lt;userid&gt;&amp;friend=&lt;friendid&gt;</a></code>: add a friend to user</li>\n"
      "<li><code><a href='post_message?user=0&dest=1&msg=Hello%20you'>post_message?user=&lt;userid&gt;&amp;dest=&lt;destod&gt;&amp;msg=&lt;message&gt;</a></code>: add a friend to user</li>\n"
      "</ul>\n";
    snprintf(reply->reply, MAX_REPLY_LENGTH, "%s", help_response);

    sleep(1); // simulate a complex computation

    memcache_set(&memcache, page_request->url, reply);
  } else {
    printf("%s: found the page in the cache (timestamp = %ld.%ld)\n",  __func__,
	   reply->timestamp.tv_sec, reply->timestamp.tv_usec);
  }
  memcpy(page_request->reply, reply->reply, strlen(reply->reply)+1);
}


void do_hello(struct page_request* req) {
  static char* hello_response = "Hello world !\n";  

  struct page_reply* reply = memcache_get(&memcache, req->url);
  if(! reply) {
    printf("%s: cannot find page in the cache\n", __func__);
    reply = malloc(sizeof(struct page_reply));
    gettimeofday(&reply->timestamp, NULL);  

    pthread_rwlock_rdlock(&shm->big_lock);
    struct key_value* user = search_key(req, "user");
    if(user) {    
      snprintf(reply->reply, MAX_REPLY_LENGTH, "Hello %s !", user->value);
    } else {
      snprintf(reply->reply, MAX_REPLY_LENGTH, "%s", hello_response);
    }
    pthread_rwlock_unlock(&shm->big_lock);
    sleep(1); // simulate a complex computation

    memcache_set(&memcache, req->url, reply);
  } else {
    printf("%s: found the page in the cache (timestamp = %ld.%ld)\n", __func__,
	   reply->timestamp.tv_sec, reply->timestamp.tv_usec);
  }

  memcpy(req->reply, reply->reply, strlen(reply->reply)+1);
}

/* print the list of users that are friend with user */
void do_add_friend(struct page_request* req) {
  pthread_rwlock_wrlock(&shm->big_lock);

  struct key_value* user = search_key(req, "user");
  if(! user) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
    goto out;
  }
  struct user* u = search_user(atoi(user->value));
  if(!u) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", user->value);
    goto out;
  }

  struct key_value* friend = search_key(req, "friend");
  if(! friend) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'friend'");
    goto out;
  }
  struct user* f = search_user(atoi(friend->value));
  if(!f) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", friend->value);
    goto out;
  }

  if(is_friend(u, f)) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s is already a friend of %s!", f->name, u->name);
    goto out;
  }

  int friend_rank = u->nb_friends++;
  u->friends[friend_rank] = f->id;
  gettimeofday(&u->last_modification, NULL);  
  snprintf(req->reply, MAX_REPLY_LENGTH, "User %s is now a friend of %s", f->name, u->name);

  printf("%s: update user %d (timestamp = %ld.%ld)\n", __func__, u->id,
	 u->last_modification.tv_sec, u->last_modification.tv_usec);

 out:
  pthread_rwlock_unlock(&shm->big_lock);
}

/* create a new_user */
void do_list_users(struct page_request* req) {
  int index = 0;
  int len;

  APPEND_TEXT(req->reply, "<h1>Existing users:</h1>\n");
  APPEND_TEXT(req->reply, "<ul>\n");

  for(int i = 0; i<shm->nb_users; i++) {
    APPEND_TEXT(req->reply, "<li><a href=\"view_user?user=%d\">%s</a></li>",
		shm->users[i].id, shm->users[i].name);
  }
  APPEND_TEXT(req->reply, "</ul>\n");
}


/* create a new_user */
void do_add_user(struct page_request* req) {
  pthread_rwlock_wrlock(&shm->big_lock);
  struct key_value* user = search_key(req, "user");
  if(! user) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
    goto out;
  }

  struct user*u = search_user_name(user->value);
  if(u) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s already exists!", user->value);
    goto out;
  }

  int user_id = shm->nb_users++;
  init_user(&shm->users[user_id]);
  strncpy(shm->users[user_id].name, user->value, MAX_USERNAME_LENGTH);
  gettimeofday(&shm->users[user_id].last_modification, NULL);  

  snprintf(req->reply, MAX_REPLY_LENGTH, "User %s has been created!", shm->users[user_id].name);
 out:
  pthread_rwlock_unlock(&shm->big_lock);
}

/* create a new_user */
void do_view_user(struct page_request* req) {

  struct page_reply* reply = memcache_get(&memcache, req->url);
  if(reply) {
    pthread_rwlock_rdlock(&shm->big_lock);
    struct key_value* user = search_key(req, "user");
    if(! user) {
      snprintf(reply->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
      goto out;
    }

    struct user*u = search_user(atoi(user->value));
    if(!u) {
      snprintf(reply->reply, MAX_REPLY_LENGTH, "User %s does not exist!", user->value);
      goto out;
    }

    if(TIME_DIFF(reply->timestamp, u->last_modification) < 0) {
      /* we need to regenerate this page */
      printf("%s: found the page in cache, but it's too old !\n", __func__);
      free(reply);
      reply = NULL;
    }
    pthread_rwlock_unlock(&shm->big_lock);
  }

  if(! reply) {
    printf("%s: cannot find page in the cache\n", __func__);
    reply = malloc(sizeof(struct page_reply));
    gettimeofday(&reply->timestamp, NULL);  

    pthread_rwlock_rdlock(&shm->big_lock);

    struct key_value* user = search_key(req, "user");
    if(! user) {
      snprintf(reply->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
      goto out;
    }


    struct user*u = search_user(atoi(user->value));
    if(!u) {
      snprintf(reply->reply, MAX_REPLY_LENGTH, "User %s does not exist!", user->value);
      goto out;
    }

    int index = 0;
    int len = 0;
    APPEND_TEXT(reply->reply, "<h1>%s</h1>\n", u->name);
    APPEND_TEXT(reply->reply, "%s has %d friends:\n", u->name, u->nb_friends);
    APPEND_TEXT(reply->reply, "<ul>\n");

    for(int i=0; i<u->nb_friends; i++) {
      APPEND_TEXT(reply->reply, "<li><a href=\"view_user?user=%d\">%s</a></li>", u->friends[i], search_user(u->friends[i])->name);
    }

    APPEND_TEXT(reply->reply, "</ul>\n");
    APPEND_TEXT(reply->reply, "<h2>Wall</h2>\n");

    for(int i=0; i<u->nb_messages; i++) {
      struct message* msg = &u->messages[i];
      APPEND_TEXT(reply->reply, "<p>%s: %s</p>\n", search_user(msg->from)->name, msg->msg);
    }


    APPEND_TEXT(reply->reply, "<form action=\"/post_message\" method=\"get\">");
    APPEND_TEXT(reply->reply, "<input name=\"user\" type=\"hidden\" value=\"%d\">", u->id);
    APPEND_TEXT(reply->reply, "<select name=\"dest\">\n");
    for(int i=0; i<u->nb_friends; i++) {
      APPEND_TEXT(reply->reply, "<option value=\"%d\">%s</option>\n", u->friends[i], search_user(u->friends[i])->name );
    }

    APPEND_TEXT(reply->reply, " </select>\n");
    APPEND_TEXT(reply->reply, "<input type=\"text\" name=\"msg\">\n");
    APPEND_TEXT(reply->reply, "<input type=\"submit\">\n");
    APPEND_TEXT(reply->reply, "</form>\n");

  out:
    pthread_rwlock_unlock(&shm->big_lock);

    memcache_set(&memcache, req->url, reply);
  } else {
    printf("%s: found the page in the cache (timestamp = %ld.%ld)\n", __func__,
	   reply->timestamp.tv_sec, reply->timestamp.tv_usec);

  }
  
  memcpy(req->reply, reply->reply, strlen(reply->reply)+1);
}


void do_post_message(struct page_request* req) {

  pthread_rwlock_wrlock(&shm->big_lock);
  struct key_value* user = search_key(req, "user");
  if(! user) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
    goto out;
  }

  struct user*u = search_user(atoi(user->value));
  if(!u) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", user->value);
    goto out;
  }

  struct key_value* dest = search_key(req, "dest");
  if(! dest) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'dest'");
    goto out;
  }
  struct user*d = search_user(atoi(dest->value));
  if(!d) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", dest->value);
    goto out;
  }

  struct key_value* message = search_key(req, "msg");
  if(! message) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'msg'");
    goto out;
  }

  if(!is_friend(u, d)) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s is not a friend of %s!", u->name, d->name);
    goto out;
  }

  int msg_rank = d->nb_messages++;
  d->messages[msg_rank].from = u->id;
  strncpy(d->messages[msg_rank].msg, message->value, MAX_REPLY_LENGTH);
  gettimeofday(&u->last_modification, NULL);  

  snprintf(req->reply, MAX_REPLY_LENGTH, "User %s sends a message to %s", u->name, d->name);  

 out:
  pthread_rwlock_unlock(&shm->big_lock);
}

void print_footer(struct page_request* req) {
  shm->compteur_reponse++;
  
  char footer_str[MAX_REPLY_LENGTH];
  snprintf(footer_str, MAX_REPLY_LENGTH, "<p>Connection counter: %d</p>\n", shm->compteur_reponse);
  strncat(req->reply, footer_str, MAX_REPLY_LENGTH-(strlen(req->reply)+1));
}


void process_page(struct page_request* req) {

  if(strcmp(req->url, "hello") == 0 ){
    do_hello(req);
  } else if(strcmp(req->url, "add_user") == 0 ){
    do_add_user(req);
  } else if(strcmp(req->url, "list_users") == 0 ){
    do_list_users(req);
  } else if(strcmp(req->url, "view_user") == 0 ){
    do_view_user(req);
  } else if(strcmp(req->url, "add_friend") == 0 ){
    do_add_friend(req);
  } else if(strcmp(req->url, "post_message") == 0 ){
    do_post_message(req);
  } else if(strcmp(req->url, "help") == 0 ){
    do_help(req);
  } else {
    do_help(req);
  }
  print_footer(req);
}
