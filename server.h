/***********************************************************************
* Code listing from "Advanced Linux Programming," by CodeSourcery LLC  *
* Copyright (C) 2001 by New Riders Publishing                          *
* See COPYRIGHT for license information.                               *
***********************************************************************/

#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <sys/types.h>

/*** Symbols defined in common.c.  ************************************/

/* The name of this program.  */
extern const char* program_name;

/* If non-zero, print verbose messages.  */
extern int verbose;

/* Like malloc, except aborts the program if allocation fails.  */
extern void* xmalloc (size_t size);

/* Like realloc, except aborts the program if allocation fails.  */
extern void* xrealloc (void* ptr, size_t size);

/* Like strdup, except aborts the program if allocation fails.  */
extern char* xstrdup (const char* s);

/* Print an error message for a failed call OPERATION, using the value
   of errno, and end the program.  */
extern void system_error (const char* operation);

/* Print an error message for failure involving CAUSE, including a
   descriptive MESSAGE, and end the program.  */
extern void error (const char* cause, const char* message);

/* Return the directory containing the running program's executable.
   The return value is a memory buffer which the caller must deallocate
   using free.  This function calls abort on failure.  */
extern char* get_self_executable_directory ();


/*** Symbols defined in module.c  **************************************/

/* An instance of a loaded server module.  */
struct server_module {
  /* The shared library handle corresponding to the loaded module.  */
  void* handle;
  /* A name describing the module.  */
  const char* name;
  /* The function which generates the HTML results for this module.  */
  void (* generate_function) (int);
};

/* The directory from which modules are loaded.  */
extern char* module_dir;

/* Attempt to load a server module with the name MODULE_PATH.  If a
   server module exists with this path, loads the module and returns a
   server_module structure representing it.  Otherwise, returns NULL.  */
extern struct server_module* module_open (const char* module_path);

/* Close a server module and deallocate the MODULE object.  */
extern void module_close (struct server_module* module);


/*** Symbols defined in server.c.  ************************************/

/* Run the server on LOCAL_ADDRESS and PORT.  */
extern void server_run (uint16_t port, int ipVersion);


#define MAX_URL_LENGTH 128
#define MAX_VALUE_LENGTH 4096
#define MAX_REPLY_LENGTH (4096*10)
#define MAX_NB_PARAMETERS 50

struct key_value {
  char key[MAX_URL_LENGTH];
  char value[MAX_VALUE_LENGTH];
};

/* This structure contains the informations on a page request.
 * For instance, if the user requests the url "/display?user=plip&foo=plop&bar=plup",
 * the structure will contain the following:
 */
struct page_request {
  char url[MAX_URL_LENGTH]; //"display"
  struct key_value parameters[MAX_NB_PARAMETERS]; // [ {user, plip}, {foo, plop}, {bar, plup} ]
  int nb_param; // 3

  int connection_fd; // the file descriptor
  char reply[MAX_REPLY_LENGTH]; // "HTTP/1.0 200 OK\nContent-type: text/html\n...."
};

/* search for a key in the page parameters */
struct key_value* search_key(struct page_request* req, char* key);

#define NOT_IMPLEMENTED() do {						\
    fprintf(stderr, "error: %s is not implemented !\n", __FUNCTION__);	\
    abort();								\
  } while(0)

#endif  /* SERVER_H */
