/*
   pid_hash_map.h

   Title : OBD Monitor
   Author: Derek Chadwick
   Date  : 06/07/2017

   Purpose: A wrapper for uthash, used to store PID parameters.

*/

#ifndef PID_MAP_INCLUDED
#define PID_MAP_INCLUDED

#include "protocols.h"

/* pid_hash_map.c */
void add_pid(PID_Parameters *flpid);
PID_Parameters *find_pid(char *lookup_string);
PID_Parameters *get_first_pid_record();
PID_Parameters *get_last_pid_record();
void delete_pid(PID_Parameters *pid_record);
void delete_all_pids();
void write_pid_map(FILE *outfile);
void print_pid_map();

#endif

