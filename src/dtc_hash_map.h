/*
   dtc_hash_map.h

   Title : OBD Monitor
   Author: Derek Chadwick
   Date  : 06/07/2017

   Purpose: A wrapper for uthash, used to store DTC parameters.

*/

#ifndef DTC_MAP_INCLUDED
#define DTC_MAP_INCLUDED

#include "protocols.h"

/* dtc_hash_map.c */
void add_dtc(DTC_Parameters *fldtc);
DTC_Parameters *find_dtc(char *lookup_string);
DTC_Parameters *get_first_dtc_record();
DTC_Parameters *get_last_dtc_record();
void delete_dtc(DTC_Parameters *dtc_record);
void delete_all_dtcs();
void write_dtc_map(FILE *outfile);
void print_dtc_map();

#endif

