#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "server.h"
#include "process_requests.h"

/* append a string to the reply */
#define APPEND_TEXT(...) do {						\
    len = snprintf(&req->reply[index], MAX_REPLY_LENGTH-index, __VA_ARGS__); \
    index += len;							\
  }while(0)

int compteur_reponse;

#define MAX_USERNAME_LENGTH 128

struct message;
struct user {  
  int id;
  char name[MAX_USERNAME_LENGTH];

  int nb_friends;
  int* friends;

  int nb_messages;
  struct message*messages;
};

struct message {
  int from;
  char msg[MAX_REPLY_LENGTH];
};

int nb_users = 0;
struct user* users = NULL;

/* Procedure chargee de toutes les initialisations specifiques */
void init(){
  compteur_reponse = 0; /* NB : On aurait aussi pu faire cette initialisation au moment */
                        /* de la declaration de compteur_reponse, mais l'avantage de le  */
                        /* faire ici est que c'est plus pedagogique pour comprendre le  */
                        /* role de la procedure init.                                   */
}

void init_user(struct user*u){
  static int next_id = 0;
  u->id = next_id++;

  strcpy(u->name, "");
  u->nb_friends = 0;
  u->friends = NULL;

  u->nb_messages=0;
  u->messages=0;
}

struct user* search_user(int id) {
  for(int i=0; i<nb_users; i++) {
    if(users[i].id == id) {
      return &users[i];
    }
  }

  return NULL;
}

struct user* search_user_name(const char* name) {
  for(int i=0; i<nb_users; i++) {
    if(strcmp(users[i].name, name) == 0) {
      return &users[i];
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
  snprintf(page_request->reply, MAX_REPLY_LENGTH, "%s", help_response);
}


void do_hello(struct page_request* req) {
  static char* hello_response = "Hello world !";

  struct key_value* user = search_key(req, "user");
  if(user) {
    struct user* u = search_user(atoi(user->value));
    if(u)
      snprintf(req->reply, MAX_REPLY_LENGTH, "Hello %s !", u->name);
    else
      snprintf(req->reply, MAX_REPLY_LENGTH, "Unknown user: '%s'", user->value);
  } else {
    snprintf(req->reply, MAX_REPLY_LENGTH, "%s", hello_response);
  }
}

/* print the list of users that are friend with user */
void do_add_friend(struct page_request* req) {
  struct key_value* user = search_key(req, "user");
  if(! user) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
    return;
  }
  struct user* u = search_user(atoi(user->value));
  if(!u) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", user->value);
    return;
  }

  struct key_value* friend = search_key(req, "friend");
  if(! friend) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'friend'");
    return;
  }
  struct user* f = search_user(atoi(friend->value));
  if(!f) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", friend->value);
    return;
  }

  if(is_friend(u, f)) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s is already a friend of %s!", f->name, u->name);
    return;
  }

  void*ptr = realloc(u->friends, sizeof(int)*(u->nb_friends+1));
  if(!ptr) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Cannot allocate memory !");
    return;    
  }
  u->friends = ptr;
  int friend_rank = u->nb_friends++;

  u->friends[friend_rank] = f->id;
  snprintf(req->reply, MAX_REPLY_LENGTH, "User %s is now a friend of %s", f->name, u->name);
}

/* create a new_user */
void do_list_users(struct page_request* req) {
  int index = 0;
  int len;

  APPEND_TEXT("<h1>Existing users:</h1>\n");
  APPEND_TEXT("<ul>\n");

  for(int i = 0; i<nb_users; i++) {
    APPEND_TEXT("<li><a href=\"view_user?user=%d\">%s</a></li>", users[i].id, users[i].name);
  }
  APPEND_TEXT("</ul>\n");
}


/* create a new_user */
void do_add_user(struct page_request* req) {
  struct key_value* user = search_key(req, "user");
  if(! user) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
    return;
  }

  struct user*u = search_user_name(user->value);
  if(u) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s already exists!", user->value);
    return;
  }

  void*ptr = realloc(users, sizeof(struct user)*(nb_users+1));
  if(!ptr) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Cannot allocate memory !");
    return;
  }
  users = ptr;
  int user_id = nb_users++;

  init_user(&users[user_id]);
  strncpy(users[user_id].name, user->value, MAX_USERNAME_LENGTH);

  snprintf(req->reply, MAX_REPLY_LENGTH, "User %s has been created!", users[user_id].name);
}

/* create a new_user */
void do_view_user(struct page_request* req) {
  struct key_value* user = search_key(req, "user");
  if(! user) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
    return;
  }

  struct user*u = search_user(atoi(user->value));
  if(!u) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", user->value);
    return;
  }

  int index = 0;
  int len = 0;
  APPEND_TEXT("<h1>%s</h1>\n", u->name);
  APPEND_TEXT("%s has %d friends:\n", u->name, u->nb_friends);
  APPEND_TEXT("<ul>\n");

  for(int i=0; i<u->nb_friends; i++) {
    APPEND_TEXT("<li><a href=\"view_user?user=%d\">%s</a></li>", u->friends[i], search_user(u->friends[i])->name);
  }

  APPEND_TEXT("</ul>\n");
  APPEND_TEXT("<h2>Wall</h2>\n");

  for(int i=0; i<u->nb_messages; i++) {
    struct message* msg = &u->messages[i];
    APPEND_TEXT("<p>%s: %s</p>\n", search_user(msg->from)->name, msg->msg);
  }


  APPEND_TEXT("<form action=\"/post_message\" method=\"get\">");
  APPEND_TEXT("<input name=\"user\" type=\"hidden\" value=\"%d\">", u->id);
  APPEND_TEXT("<select name=\"dest\">\n");
  for(int i=0; i<u->nb_friends; i++) {
    APPEND_TEXT("<option value=\"%d\">%s</option>\n", u->friends[i], search_user(u->friends[i])->name );
  }

  APPEND_TEXT(" </select>\n");
  APPEND_TEXT("<input type=\"text\" name=\"msg\">\n");
  APPEND_TEXT("<input type=\"submit\">\n");
  APPEND_TEXT("</form>\n");
}


void do_post_message(struct page_request* req) {
  struct key_value* user = search_key(req, "user");
  if(! user) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'user'");
    return;
  }

  struct user*u = search_user(atoi(user->value));
  if(!u) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", user->value);
    return;
  }

  struct key_value* dest = search_key(req, "dest");
  if(! dest) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'dest'");
    return;
  }
  struct user*d = search_user(atoi(dest->value));
  if(!d) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s does not exist!", dest->value);
    return;
  }

  struct key_value* message = search_key(req, "msg");
  if(! message) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Parameter missing: you need to set 'msg'");
    return;
  }

  if(!is_friend(u, d)) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "User %s is not a friend of %s!", u->name, d->name);
    return;
  }

  void *ptr = realloc(d->messages, sizeof(struct message)*((d->nb_messages)+1));
  if(!ptr) {
    snprintf(req->reply, MAX_REPLY_LENGTH, "Cannot allocate memory !");
    return;    
  }

  d->messages = ptr;
  int msg_rank = d->nb_messages++;

  d->messages[msg_rank].from = u->id;
  strncpy(d->messages[msg_rank].msg, message->value, MAX_REPLY_LENGTH);
  snprintf(req->reply, MAX_REPLY_LENGTH, "User %s sends a message to %s", u->name, d->name);  
}

void print_footer(struct page_request* req) {
  compteur_reponse++;
  
  char footer_str[MAX_REPLY_LENGTH];
  snprintf(footer_str, MAX_REPLY_LENGTH, "\n<p>Connection counter: %d</p>\n", compteur_reponse);
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
