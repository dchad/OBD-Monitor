/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A UDP server and GUI that communicates with vehicle
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

#ifndef OBD_MONITOR_INCLUDED
#define OBD_MONITOR_INCLUDED


#include <stdio.h>
#include <sys/types.h>

/* Constant Definitions. */

#define MAX_ECU_QUERY_LEN 16
#define BUFFER_MAX_LEN 4096
#define MAX_PATH_LEN 4096
#define MAX_SERIAL_BUF_LEN 256
#define OBD_WAIT_TIMEOUT 100000
#define NUM_PI 3.1415926535897932384626433832795028841971693993751
#define LOG_FILE "./obd-mon-data.log"

/* TODO: PID Message Codes. */

#define PID_SUPPORTED_01 "01 00\n"
#define PID_DTC_COUNT    "01 01\n"
#define PID_ECT          "01 05\n"

#define PID_DTC_CODE     "03\n"

#define PID_VIN "09 02\n"
#define PID_ECU "09 0A\n"



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
void update_comms_log_view(char *msg);
void set_status_bar_msg(char *msg);
void get_status_bar_msg(char *msg);

/* util.c */
int xfatal(char *str);
void *xcalloc (size_t size);
void *xmalloc (size_t size);
void *xrealloc (void *ptr, size_t size);
int xfree(char *buf, int len);
char* xitoa(int value, char* result, int len, int base);
int xstrcpy(char *out_buf, char *in_buf, int start, int end);
int xhextoascii(char *out_buf, char *in_buf);
int print_help();
int get_time_string(char *tstr, int slen);
int get_ip_address(char *interface, char *ip_addr);
int validate_ipv4_address(char *ipv4_addr);
int validate_ipv6_address(char *ipv6_addr);
char *ltrim(char *s);
char *rtrim(char *s);
char *trim(char *s);
void uppercase(char *s);
int replacechar(char *str, char orig, char rep);


/* log.c */
int open_log_file(char *startup_path, char *log_file_name);
int print_log_entry(char *estr);
void close_log_file();

/* sockets.c */
int init_server_socket(char *server, char *port);
int send_ecu_msg(char *query);
int recv_ecu_msg(char *msg);
int init_obd_comms(char *obd_msg);
int server_connect();
int get_ecu_connected();
void set_ecu_connected(int cstatus);

/* Unit Test Functions */
int unit_tests(FILE *log_file);

#endif

