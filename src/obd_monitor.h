/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A UDP server that communicates with vehicle
                engine control units via an OBD-II interface to obtain 
                engine status and fault codes. 

                Implements two functions:

                1. A UDP datagram server that receives requests for vehicle
                status information from a client application (GUI) and 
                returns the requested information to the client. 

                2. Serial communications to request vehicle status
                information and fault codes from the engine control unit using 
                the OBD-II protocol.

   Date: 30/11/2017
   
*/

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>


/* Constant Definitions. */

#define MAX_ECU_QUERY_LEN 16
#define BUFFER_MAX_LEN 4096
#define MAX_PATH_LEN 4096
#define OBD_WAIT_TIMEOUT 100000
#define NUM_PI 3.1415926535897932384626433832795028841971693993751
#define LOG_FILE "./obd-mon-data.log"

/* Type Definitions. */

struct _DialPoint {
   double x;
   double y;
};

typedef struct _DialPoint DialPoint;

   
/* Function Prototypes. */

/* obd_monitor_gui.c */
void set_interface_on();
void set_interface_off();
int get_interface_status();

/* util.c */
int fatal(char *str);
void *xcalloc (size_t size);
void *xmalloc (size_t size);
void *xrealloc (void *ptr, size_t size);
int xfree(char *buf, int len);
int print_help();
char* xitoa(int value, char* result, int len, int base);

/* log.c */
FILE *open_log_file(char *startup_path);
int print_log_entry(char *estr, FILE *log_file);

/* Unit Test Functions */
int unit_tests(FILE *log_file);



