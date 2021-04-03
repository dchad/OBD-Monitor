/*
   util.c

   Project: OBD-II Monitor
   Author : Derek Chadwick 18910502
   Date   : 24/09/2017
  
   Purpose: Wrapper functions for various standard C lib functions to
            make them safer!!!.
   
*/

#include <ctype.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
/* #include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> */
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "obd_monitor.h"

/* Redefine malloc with a fatal exit. */
void *xmalloc (size_t size)
{
   register void *value = malloc (size);
   if (value == 0)
   {
      xfatal("xmalloc() <FATAL> Virtual Memory Exhausted!!!");
   }
   return value;
}

/* Redefine calloc with a fatal exit. */
void *xcalloc (size_t size)
{
   register void *value = calloc (size, 1);
   if (value == 0)
   {
      xfatal("xmalloc() <FATAL> Virtual Memory Exhausted!!!");
   }
   return value;
}

/* Redefine realloc with a fatal exit. */
void *xrealloc (void *ptr, size_t size)
{
   register void *value = realloc (ptr, size);
   if (value == 0)
   {
      xfatal ("xmalloc() <FATAL> Virtual Memory Exhausted");
   }
   return value;
}

/* Redefine free with buffer zeroing. */
int xfree(char *buf, int len)
{
   memset(buf, 0, len);
   free(buf);
   return(0);
}

/*
   Copy a string segment specified by start and end indices. 
   Start and end values must be 0...strlen()-1, with start
   being less than the end value.
*/
int xstrcpy(char *out_buf, char *in_buf, int start, int end)
{
   int ii, len, result, ij;
   
   result = -1;
   len = strlen(in_buf);
   if ((start >= 0) && (end > start) && (end < len))
   {
      ij = 0;
      for (ii = start; ii <= end; ii++)
      {
         out_buf[ij] = in_buf[ii];
         ij++;
      }
      result = ij;
   }
   return(result);
}

/*
   Converts a string of hexadecimal values encoded as ascii characters to
   the equivalent ascii string.
   
   Example hexadecimal string:
   
   "30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A"
   
   converts to ascii string:
   
   "0 1 2 3 4 5 6 7 8 9 A B C D E F G H I J K L M N O P Q R S T U V W X Y Z"
   
*/
int xhextoascii(char *out_buf, char *in_buf)
{
   int ii;
   long lnum;
   char temp_buf[256];
   char *token;
   char vin_char[1];
   
   ii = 0;
   memset(out_buf, 0, 256);
   memset(temp_buf, 0, 256);
   
   token = strtok(in_buf, " ");
   
   /* Parse the tokens. */
   while( token != NULL ) 
   {
      /* printf("xhextoascii() <DEBUG>: %s\n", token); */
      lnum = strtol(token, 0, 16);
      if ((lnum > 31) && (lnum < 124)) /* Only printable characters. */
      {
         vin_char[0] = (char)lnum;
         strncat(out_buf, vin_char, 1);
         strcat(out_buf, " ");
      }

      token = strtok(NULL, " ");
      ii++;
   }
   
   return(ii);
}

/* Bail Out */
int xfatal(char *str)
{
   printf("%s\n", str);
   exit(1);
}

/* help */
int print_help()
{
   printf("\nOBD Monitor Version 1.0\n\n");
   printf("Command: obd_gui <options> <obd-protocol>\n\n");
   printf("Log Filename     : -l log.txt\n");
   printf("Server UDP Port  : -p 8989\n");
   printf("Server IP Address: -s 127.0.0.1 \n");
   printf("OBD Protocol     : [0..C]\n"); 
   printf("\n");

   return(0);
}

/**
 * Modified version of char* style "itoa" with buffer length check.
 * (Original by Kernighan and Ritchie)
 * Just try to understand it, I dare you.
 */
char *xitoa(int value, char* result, int len, int base) 
{

   if (base < 2 || base > 36) { *result = '\0'; return result; }
	
   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;
   int i = 0;
	
   do {
         tmp_value = value;
         value /= base;
         *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
      i++;
   } while ((i < len) && value );

   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) 
   {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}

/*
   Function: get_time_string()

   Purpose : Gets current date and time in a string.
           :
   Input   : String for date and time.
   Output  : Formatted date and time string.
*/
int get_time_string(char *tstr, int slen)
{
   time_t curtime;
   struct tm *loctime;
   int len;

   if ((tstr == NULL) || (slen < 15))
   {
      printf("get_time_string() <ERROR> Invalid string or length.\n");
      return(0);
   }
   /* Get the current time. */

   curtime = time (NULL);
   loctime = localtime (&curtime);
   if ((len = strftime(tstr, slen - 1, "-%Y%m%d-%H%M%S", loctime)) < 1)
   {
      printf("get_time_string() <WARNING> Indeterminate time string: %s\n", tstr);
   }

   return(len);
}


int validate_ipv4_address(char *ipv4_addr)
{
	/* TODO: a regex would be nice = m/\d+\.\d+\.\d+\.\d+/ */
	
	return(0);
}

int validate_ipv6_address(char *ipv6_addr)
{
	/* TODO: definitely need a regex for this one */

	return(0);
}

char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

void uppercase(char *s)
{
   while ( *s != '\0' )
   {
      *s = toupper((unsigned char) *s);
      ++s;
   }
   
   return;
}

int replacechar(char *str, char orig, char rep) 
{
    char *ix = str;
    int n = 0;
    while((ix = strchr(ix, orig)) != NULL) 
    {
        *ix++ = rep;
        n++;
    }
    return(n);
}


