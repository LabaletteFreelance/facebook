/***********************************************************************
* Code listing from "Advanced Linux Programming," by CodeSourcery LLC  *
* Copyright (C) 2001 by New Riders Publishing                          *
* See COPYRIGHT for license information.                               *
*                                                                      *
* Code adapted by Michel SIMATIC (TELECOM & Management SudParis)       *
* for ASR3 2008 graded lab                                             *
* Code adapted by François TRAHAY (Télécom SudParis)                   *
* for ASR3 2019 labs                                             *
***********************************************************************/

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <poll.h>
#include "server.h"
#include "process_requests.h"

/* HTTP response and header for a successful request.  */

static char* ok_response =
  "HTTP/1.0 200 OK\n"
  "Content-type: text/html\n"
  "\n";

/* HTTP response, header, and body indicating that the we didn't
   understand the request.  */

static char* bad_request_response = 
  "HTTP/1.0 400 Bad Request\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Bad Request</h1>\n"
  "  <p>This server did not understand your request.</p>\n"
  " </body>\n"
  "</html>\n";

/* HTTP response, header, and body template indicating that the
   method was not understood.  */

static char* bad_method_response_template = 
  "HTTP/1.0 501 Method Not Implemented\n"
  "Content-type: text/html\n"
  "\n"
  "<html>\n"
  " <body>\n"
  "  <h1>Method Not Implemented</h1>\n"
  "  <p>The method %s is not implemented by this server.</p>\n"
  " </body>\n"
  "</html>\n";

/* HTML source for the start of the page we generate.  */

static char* page_start =
  "<html>\n"
  " <body>\n"
  "  <pre>\n";

/* HTML source for the end of the page we generate.  */

static char* page_end =
  "  </pre>\n"
  "<a href=\"/\">Back to home page</a>\n"
  " </body>\n"
  "</html>\n";

/* Handler for SIGCHLD, to clean up child processes that have
   terminated.  */
static void clean_up_child_process (int signal_number) {
  int status;
  wait (&status);
}

/* search for a key in the page parameters */
struct key_value* search_key(struct page_request* req, char* key) {
  for(int i=0; i<req->nb_param; i++) {
    if(strcmp(key, req->parameters[i].key) == 0)
      return &req->parameters[i];
  }
  return NULL;
}

#define ishex( x) (((x) >= '0' && (x) <= '9') ||	\
		   ((x) >= 'a' && (x) <= 'f') ||	\
		   ((x) >= 'A' && (x) <= 'F'))

int decode(char *dest, const char *src) {
  int i_dest =0;
  int i_src = 0;

  for(i_src = 0; src[i_src] != '\0'; i_src++) {
    char c = src[i_src];
    if(c == '+')
      c = ' ';
    else if( c=='%' ) {
      if((! ishex(src[i_src+1]) )||
	 (! ishex(src[i_src+2]) )) {
	/* bad format */
	return -1;
      }
      int ascii_v;
      int n = sscanf(&src[i_src+1], "%2x", &ascii_v);
      if(n!= 1) {
	/* bad format */
	return -1;
      }

      c = (char) ascii_v;
      i_src+=2;
    }
    dest[i_dest++] = c;
  }
  dest[i_dest] = '\0';
  return i_dest;
}

/* parse an url parameter (that looks like "foo=plop") and fill a key_value structure */
void parse_key_value(char* str, struct key_value* key_value) {
  char* key, *value;
  key = strtok(str, "=");
  value = strtok(NULL, "=");
  strncpy(key_value->key, key, MAX_URL_LENGTH);
  strncpy(key_value->value, value, MAX_VALUE_LENGTH);
}

/* Parse a get url that looks like "/display?user=plip&foo=plop&bar=plup" and fill a
 *  page_request structure
 */
void parse_url(char* url, struct page_request *page_request) {
  NOT_IMPLEMENTED();
}

/* Process an HTTP "GET" request for PAGE, and send the results to the
   file descriptor CONNECTION_FD.  */
static void handle_get (struct page_request *req) {
  /* Send the HTTP response indicating success, and the HTTP header
     for an HTML page.  */
  write (req->connection_fd, ok_response, strlen (ok_response));
  /* Invoke the module, which will generate HTML output and send it
     to the client file descriptor.  */
  /* Write the start of the page.  */
  write (req->connection_fd, page_start, strlen (page_start));


  /* Invoke student's code */
  process_page(req);

  write(req->connection_fd,req->reply,strlen(req->reply));
  
  /* Write the end of the page.  */
  write (req->connection_fd, page_end, strlen (page_end));
  /* All done; close the connection socket.  */
  close (req->connection_fd);
  free(req);
}

/* Handle a client connection on the file descriptor CONNECTION_FD.  */
void handle_connection (int connection_fd) {
  char buffer[256];
  ssize_t bytes_read;

  /* Read some data from the client.  */
  bytes_read = read (connection_fd, buffer, sizeof (buffer) - 1);
  if (bytes_read > 0) {
    char method[sizeof (buffer)];
    char url[sizeof (buffer)];
    char protocol[sizeof (buffer)];

    /* Some data was read successfully.  NUL-terminate the buffer so
       we can use string operations on it.  */
    buffer[bytes_read] = '\0';
    /* The first line the client sends is the HTTP request, which is
       composed of a method, the requested page, and the protocol
       version.  */
    sscanf (buffer, "%s %s %s", method, url, protocol);
    /* The client may send various header information following the
       request.  For this HTTP implementation, we don't care about it.
       However, we need to read any data the client tries to send.  Keep
       on reading data until we get to the end of the header, which is
       delimited by a blank line.  HTTP specifies CR/LF as the line
       delimiter.  */

    if (strstr (buffer, "\r\n\r\n") == NULL) {
      do {
	bytes_read = read (connection_fd, buffer, sizeof (buffer));
	/* Make sure the last read didn't fail.  If it did, there's a
	   problem with the connection, so give up.  */
	if (bytes_read == -1) {
	  close (connection_fd);
	  return;
	}
      } while ((strstr (buffer, "\r\n\r\n") == NULL) &&
	       (buffer[0] != '\r' || buffer[1] != '\n'));
    }
    /* Check the protocol field.  We understand HTTP versions 1.0 and
       1.1.  */
    if (strcmp (protocol, "HTTP/1.0") && strcmp (protocol, "HTTP/1.1")) {
      /* We don't understand this protocol.  Report a bad response.  */
      write (connection_fd, bad_request_response, 
	     sizeof (bad_request_response));
    }
    else if (strcmp (method, "GET")) {
      /* This server only implements the GET method.  The client
	 specified some other method, so report the failure.  */
      char response[1024];

      snprintf (response, sizeof (response),
		bad_method_response_template, method);
      write (connection_fd, response, strlen (response));
    }
    else {
      struct page_request *req = malloc(sizeof(struct page_request));
      memset(req, 0xff, sizeof(struct page_request));
      req->connection_fd = connection_fd;

      char decoded_url[sizeof(buffer)];
      decode(decoded_url, url);
      parse_url(decoded_url, req);

      assert(req->nb_param >= 0);
      assert(req->nb_param < MAX_NB_PARAMETERS);
      assert(req->url[0] != '/');

      printf("page requested: %s with parameters: ", req->url);
      for(int i = 0; i<req->nb_param; i++) {
	printf("%s = %s,", req->parameters[i].key, req->parameters[i].value);
      }
      printf("\n");
      /* A valid request.  Process it.  */
      handle_get (req);
    }
  }
  else if (bytes_read == 0) {
    /* The client closed the connection before sending any data.
       Nothing to do.  */
  } else 
    /* The call to read failed.  */
    system_error ("read");
}


void server_run (uint16_t port, int ipVersion) {
  struct sockaddr_in socket_address;
  int rval;
  struct sigaction sigchld_action;
  int server_socket;
  int on = 1;
  struct addrinfo hints, *servinfo, *p;
  char service[16];

  /* Call student's initialization code */
  init();

  /* Install a handler for SIGCHLD that cleans up child processes that
     have terminated.  */
  memset (&sigchld_action, 0, sizeof (sigchld_action));
  sigchld_action.sa_handler = &clean_up_child_process;
  sigaction (SIGCHLD, &sigchld_action, NULL);

  /* Create a TCP socket (code compatible with IPv4 and IPv6). 
     This code is very much inspired from example in server code
     in man getaddrinfo. There is also some code in 
     http://livre.g6.asso.fr/index.php/Exemple_de_client/serveur_TCP 
     but less clear. */
  memset(&hints, 0, sizeof hints) ;
  switch(ipVersion){
  case 0:
    hints.ai_family = AF_UNSPEC;
    break;
  case 4:
    hints.ai_family = AF_INET;
    break;
  case 6:
    hints.ai_family = AF_INET6;
    break;
  default:
    system_error("Incorrect value in ipVersion");
  }
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP address

  sprintf(service,"%d",port);
  rval = getaddrinfo("localhost", service, &hints, &servinfo);
  if (rval != 0) {
    fprintf(stderr, "Had trouble with getaddrinfo: %s\n", gai_strerror(rval));
    system_error("getaddrinfo");
  }

  // loop through all the results and bind to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((server_socket = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1) {
        perror("socket");
        continue;
    }

    /* Specify that we can reuse this port if it is not bound by 
       another process */
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
      system_error ("setsockopt");
    }

    if (bind(server_socket, p->ai_addr, p->ai_addrlen) == -1) {
        close(server_socket);
        perror("bind");
	fprintf(stderr, "Vous devriez relancer le programme avec l'option -p pour definir un autre port\n");
        continue;
    }

    break; // if we get here, we must have connected successfully
  }

  if (p == NULL) {
    // looped off the end of the list with no successful bind
    system_error("failed to bind socket");
  }

  freeaddrinfo(servinfo); // all done with this structure

  /*  Instruct the socket to accept connections.  */
  rval = listen (server_socket, 10);
  if (rval != 0)
    system_error ("listen");

  if (verbose) {
    /* In verbose mode, display the local address and port number
       we're listening on.  */
    socklen_t address_length;
    
    /* Find the socket's local address.  */
    address_length = sizeof (socket_address);
    rval = getsockname (server_socket, (struct sockaddr *)&socket_address, &address_length);
    assert (rval == 0);
  }

  /* Print a message.  The port number needs to be converted from
     network byte order (big endian) to host byte order.  */
  printf ("server listening on localhost:%d\n",  port);

  /* Loop forever, handling connections.  */
  while (1) {
    struct sockaddr_in remote_address;
    socklen_t address_length;
    int connection;

    /* Accept a connection.  This call blocks until a connection is
       ready.  */
    address_length = sizeof (remote_address);
    connection = accept (server_socket, (struct sockaddr *)&remote_address, &address_length);
    
    if (connection == -1) {
      /* The call to accept failed.  */
      if (errno == EINTR) {
	/* The call was interrupted by a signal.  Try again.  */
	printf("interrupted accept\n");
	abort();
	continue;
      } else
	/* Something else went wrong.  */
	system_error ("accept");
    }

    /* We have a connection.  Print a message if we're running in
       verbose mode.  */
    if (verbose) {
      socklen_t address_length;

      /* Get the remote address of the connection.  */
      address_length = sizeof (socket_address);
      rval = getpeername (connection, (struct sockaddr *)&socket_address, &address_length);
      assert (rval == 0);
      /* Print a message.  */
      printf ("connection accepted from %s\n", inet_ntoa (socket_address.sin_addr));
    }

    /* Handling of this connection. */
    handle_connection(connection);
  }
}
