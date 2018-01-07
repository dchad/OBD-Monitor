/*
  Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A GUI for communication with vehicle engine control units via 
                an OBD-II interface to obtain and display engine status 
                and fault codes. 


   Date: 01/01/2018
   
*/


#include <gtk/gtk.h>
#include <math.h>
#include "obd_monitor.h"
#include "protocols.h"
#include "gui_gauges.h"

/* Constant Definitions. */

#define DIAL_X_CENTER 100.0
#define DIAL_Y_CENTER 75.0
#define GAUGE_RADIUS 50.0
#define GAUGE_START_ANGLE 0.167 * NUM_PI /* 30 degrees */
#define GAUGE_END_ANGLE -1.167 * NUM_PI  /* 210 degrees */
#define RPM_SCALE_FACTOR 38.2    /* 8000 / (radius * 2 * PI * ((30 + 210) / 360)) = 8000 / 209.5 */
#define IAT_SCALE_FACTOR 0.764   /* 160 / (radius * (1.167 * PI + 0.167 * PI)) = 160 / 209.5 */
#define ECT_SCALE_FACTOR 0.764   /* 160C / (radius * (1.167 * PI + 0.167 * PI)) = 160 / 209.5 = 0.764 */
#define MAP_SCALE_FACTOR 1.217   /* 255 / (radius * (1.167 * PI + 0.167 * PI)) = 255 / 209.5 = 1.217 */
#define FUEL_FLOW_SCALE_FACTOR 0.783 /* 164.0 / (radius * (1.167 * PI + 0.167 * PI)) */
#define GAUGE_ARC_LENGTH 209.5 /* Cairo user space dial arc length for 200 x 150 drawing area. */

char info_msg[] = "AUXILLIARY GAUGES.";

void open_aux_gauge_window(GtkWidget *widget, gpointer window) 
{
    
  GtkWidget *dialog;
  dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "%s",
            info_msg);
  gtk_window_set_title(GTK_WINDOW(dialog), "Auxilliary Gauges");
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);

  return;
}


gboolean draw_aux_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI; /* 210 degrees */
   double pid_scale_factor = 1.0; /* 200km/h / (radius * (1.167 * PI + 0.167 * PI)) = 200 / 209.5 = 0.954 */
   double pid_value = 0; /* TODO: Get pid value from protocol module. */
   double gauge_value = pid_value / pid_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_value / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;   
   double cpx;
   double cpy;
   char gauge_numerals[16];

   
   /* Draw gauge background and arc. */
   draw_dial_background(cr, 190, 140);
   
   cairo_arc_negative(cr, xc, yc, radius, gauge_start_angle, gauge_end_angle);
   cairo_stroke(cr);

   /* Draw gauge indicator arc and dot. */
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_set_line_width(cr, 3.0);
   cairo_arc(cr, xc, yc, radius, gauge_end_angle, needle_angle); 
   cairo_get_current_point(cr, &cpx, &cpy);
   cairo_stroke(cr);
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", pid_value);
   draw_dial_text(cr, "Aux 1", gauge_numerals, "?");
  
   return TRUE;
}



