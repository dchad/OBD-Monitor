
/*  Copyright 2017 Derek Chadwick

    This file is part of the OBD Monitor project.

    Fineline is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Fineline is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Fineline.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
   dtc_hash_map.c

   Title : OBD Monitor
   Author: Derek Chadwick
   Date  : 06/07/2017

   Purpose: A hashmap wrapper for uthash, used to store DTC parameters.

*/

#include <stdlib.h>
#include <stdio.h>

#include "obd_monitor.h"
#include "dtc_hash_map.h"

DTC_Parameters *dtc_map = NULL; /* the hash map head record */

void add_dtc(DTC_Parameters *fldtc)
{
    DTC_Parameters *s;

    HASH_FIND_STR(dtc_map, fldtc->dtc_code , s);  /* id already in the hash? */
    if (s == NULL)
    {
      HASH_ADD_STR(dtc_map, dtc_code, fldtc);  /* id: name of key field */
    }

}

DTC_Parameters *find_dtc(char *lookup_string)
{
    DTC_Parameters *s;

    HASH_FIND_STR(dtc_map, lookup_string, s);  /* s: output pointer */
    return s;
}

DTC_Parameters *get_first_dtc_record()
{
   return(dtc_map);
}

DTC_Parameters *get_last_dtc_record()
{
   DTC_Parameters *s = get_first_dtc_record();
   if (s != NULL)
      return((DTC_Parameters *)s->hh.prev);
   return(NULL);
}

void delete_dtc(DTC_Parameters *dtc_record)
{
    HASH_DEL(dtc_map, dtc_record);  /* event: pointer to deletee */
    free(dtc_record);
}

void delete_all_dtcs()
{
  DTC_Parameters *current_dtc, *tmp;

  HASH_ITER(hh, dtc_map, current_dtc, tmp)
  {
    HASH_DEL(dtc_map,current_dtc);  /* delete it (dtc_map advances to next) */
    free(current_dtc);              /* free it */
  }
}

void write_dtc_map(FILE *outfile)
{
    DTC_Parameters *s;

    for(s=dtc_map; s != NULL; s=(DTC_Parameters *)(s->hh.next))
    {
        fputs(s->dtc_code, outfile);
    }
}

void send_dtc_map()
{
    DTC_Parameters *s;

    for(s=dtc_map; s != NULL; s=(DTC_Parameters *)(s->hh.next))
    {
        /* TODO: send_event(s->dtc_code); */
    }
}

void print_dtc_map()
{
    DTC_Parameters *s;

    for(s=dtc_map; s != NULL; s=(DTC_Parameters *)(s->hh.next))
    {
        printf("<%s> %s\n", s->dtc_code, s->dtc_description);
    }
}

