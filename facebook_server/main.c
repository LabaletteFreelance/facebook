/***********************************************************************
* Code listing from "Advanced Linux Programming," by CodeSourcery LLC  *
* Copyright (C) 2001 by New Riders Publishing                          *
* See COPYRIGHT for license information.                               *
*                                                                      *
* Code adapted by Michel SIMATIC (TELECOM & Management SudParis)       *
* for ASR3 2008 graded lab                                             *
***********************************************************************/

#include <assert.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "server.h"

/* Description of long options for getopt_long.  */

static const struct option long_options[] = {
  { "help",             0, NULL, 'h' },
  { "ipversion",        1, NULL, 'i' },
  { "port",             1, NULL, 'p' },
  { "verbose",          0, NULL, 'v' },
};

/* Description of short options for getopt_long.  */

static const char* const short_options = "hi:p:v";

/* Usage summary text.  */

static const char* const usage_template = 
  "Usage: %s [ options ]\n"
  "  -h, --help                      Print this information.\n"
  "  -i, --ipversion version_of_IP   Use specific version of IP protoco. Can be 0=unspecified, 4=IPv4 or 6=IPv6 (default = 0).\n"
  "  -p, --port PORT                 Bind to specified port (default = 8080).\n"
  "  -v, --verbose                   Print verbose messages.\n";

/* Print usage information and exit.  If IS_ERROR is non-zero, write to
   stderr and use an error exit code.  Otherwise, write to stdout and
   use a non-error termination code.  Does not return.  */

static void print_usage (int is_error) {
  fprintf (is_error ? stderr : stdout, usage_template, program_name);
  exit (is_error ? 1 : 0);
}

int main (int argc, char* const argv[]) {
  uint16_t port;
  int next_option;
  int ipVersion;

  /* Store the program name, which we'll use in error messages.  */
  program_name = argv[0];

  /* Set defaults for options. */
  port = (uint16_t) 8080; /* We do not need to call htons because we
			     use directly port in server_run */
  ipVersion = 0; /* ip version is unspecified. */
  verbose = 0;   /* Don't print verbose messages.  */

  /* Parse options.  */
  do {
    next_option = 
      getopt_long (argc, argv, short_options, long_options, NULL);
    switch (next_option) {
    case 'h':  
      /* User specified -h or --help.  */
      print_usage (0);

    case 'i':  
      /* User specified -i or --ipversion.  */
      {
	long value;
	char* end;

	value = strtol (optarg, &end, 10);
	if (*end != '\0')
	  /* The user specified non-digits in the port number.  */
	  print_usage (1);
	if (value != 0 && value != 4 && value != 6)
	  /* The user gave incorrect IP version number. */
	  print_usage (1);
	ipVersion = value;
      }
      break;

    case 'p':  
      /* User specified -p or --port.  */
      {
	long value;
	char* end;

	value = strtol (optarg, &end, 10);
	if (*end != '\0')
	  /* The user specified non-digits in the port number.  */
	  print_usage (1);
	/* We do not need to call htons because we use directly port in 
	   server_run */
	port = (uint16_t) value;
      }
      break;

    case 'v':  
      /* User specified -v or --verbose.  */
      verbose = 1;
      break;

    case '?':  
      /* User specified an nrecognized option.  */
      print_usage (1);

    case -1:  
      /* Done with options.  */
      break;

    default:
      abort ();
    }
  } while (next_option != -1);

  /* This program takes no additional arguments.  Issue an error if the
     user specified any.  */
  if (optind != argc)
    print_usage (1);

  /* Run the server.  */
  server_run (port, ipVersion);

  return 0;
}
