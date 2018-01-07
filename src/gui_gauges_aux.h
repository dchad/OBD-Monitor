/*
  Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A GUI for communication with vehicle engine control units via 
                an OBD-II interface to obtain and display engine status 
                and fault codes. 


   Date: 01/01/2018
   
*/

#ifndef AUX_GAUGES_INCLUDED
#define AUX_GAUGES_INCLUDED

#include <gtk/gtk.h>

void open_aux_gauge_window(GtkWidget *widget, gpointer window);
gboolean draw_aux_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);


#endif

