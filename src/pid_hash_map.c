
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
   pid_hash_map.c

   Title : OBD Monitor
   Author: Derek Chadwick
   Date  : 06/07/2017

   Purpose: A hashmap wrapper for uthash, used to store PID parameters.

*/

#include "obd_monitor.h"
#include "pid_hash_map.h"

PID_Parameters *pid_map = NULL; /* the hash map head record */

void add_pid(PID_Parameters *flpid)
{
    PID_Parameters *s;

    HASH_FIND_STR(pid_map, flpid->pid_code , s);  /* id already in the hash? */
    if (s == NULL)
    {
      HASH_ADD_STR(pid_map, pid_code, flpid);  /* id: name of key field */
    }

}

PID_Parameters *find_pid(char *lookup_string)
{
    PID_Parameters *s;

    HASH_FIND_STR(pid_map, lookup_string, s);  /* s: output pointer */
    return s;
}

PID_Parameters *get_first_pid_record()
{
   return(pid_map);
}

PID_Parameters *get_last_pid_record()
{
   PID_Parameters *s = get_first_pid_record();
   if (s != NULL)
      return((PID_Parameters *)s->hh.prev);
   return(NULL);
}

void delete_pid(PID_Parameters *pid_record)
{
    HASH_DEL(pid_map, pid_record);  /* event: pointer to deletee */
    free(pid_record);
}

void delete_all_pids()
{
  PID_Parameters *current_pid, *tmp;

  HASH_ITER(hh, pid_map, current_pid, tmp)
  {
    HASH_DEL(pid_map,current_pid);  /* delete it (pid_map advances to next) */
    free(current_pid);              /* free it */
  }
}

void write_pid_map(FILE *outfile)
{
    PID_Parameters *s;

    for(s=pid_map; s != NULL; s=(PID_Parameters *)(s->hh.next))
    {
        fputs(s->pid_code, outfile);
    }
}

void send_pid_map()
{
    PID_Parameters *s;

    for(s=pid_map; s != NULL; s=(PID_Parameters *)(s->hh.next))
    {
        /* TODO: send_event(s->pid_code); */
    }
}

void print_pid_map()
{
    PID_Parameters *s;

    for(s=pid_map; s != NULL; s=(PID_Parameters *)(s->hh.next))
    {
        printf("<%s> %s\n", s->pid_code, s->pid_description);
    }
}

