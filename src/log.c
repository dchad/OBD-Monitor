/*
   log.c

   Project: OBD-II Monitor
   
   Author : Derek Chadwick 18910502
   
   Date   : 24/12/2017
  
   Purpose: Logging, reporting and debug functions.
   
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <libgen.h>
#include <string.h>
#include <time.h>

#include "obd_monitor.h"

FILE *log_file = NULL;

/*
   Function: open_log_file()
 
   Purpose : Changes the current working directory to the location of the fsic binary
           : then opens the log file.
   Input   : Start file path.
   Output  : Returns 1 or -1.
*/
int open_log_file(char *startup_path, char *log_file_name)
{
   char current_working_dir[MAX_PATH_LEN];
   char *temp_path = NULL;
   int ret_val = 1;

   if (current_working_dir == NULL)
   {
      printf("open_log_file() <ERROR>: malloc failed.\n");
   }
   else
   {
      if (startup_path[0] != '.')  /* if started from some other directory then change to the fsic directory */
      {
         temp_path = dirname(startup_path);
         if (chdir(temp_path) != 0)
         {
            printf("open_log_file() <ERROR>: Could not change directory: %s\n", temp_path);  
         }
      }
      if (getcwd(current_working_dir, MAX_PATH_LEN) == 0)
      {
         printf("open_log_file() <ERROR>: Could not get current working directory.\n");  
      }
      else
      {
         log_file = fopen(log_file_name, "a");
         if (log_file == NULL)
         {
            printf("open_log_file() <ERROR>: could not open logfile: %s\n", current_working_dir); 
            ret_val = -1;
         }
         /* printf("DEBUG: open_log_file(): %s\n", current_working_dir); */ 
      }
   }


   return(ret_val);
}


/*
   Function: close_log_file()
 
   Purpose : Closes the log file.
   
*/
void close_log_file()
{
   print_log_entry("------------------------");
   print_log_entry(">>> Closing log session.");
   print_log_entry("------------------------");
   
   fclose(log_file);
   
   return;
}


/*
   Function: print_log_entry()
 
   Purpose : Creates a log entry and prints to the log file and stdin.
           : 
   Input   : Log string and log file.
   Output  : Timestamped log entry.
*/
int print_log_entry(char *estr)
{
   time_t curtime;
   struct tm *loctime;
   int slen = strlen(estr);
   char *log_entry = xcalloc(slen + 100);
     
   /* Get the current time. */
   curtime = time (NULL);
   loctime = localtime (&curtime);
   char *time_str = asctime(loctime);   
   strncpy(log_entry, time_str, strlen(time_str) - 1);
   strncat(log_entry, " ", 1);
   strncat(log_entry, estr, slen);

   if (log_file != NULL)
   {
      fputs (log_entry, log_file);
   }
   else
   {
      printf("print_log_entry(): %s\n", log_entry);
   }

   xfree(log_entry, slen + 100);
  
   return(0);
}



