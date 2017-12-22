/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A GUI for communication with vehicle engine control units via 
                an OBD-II interface to obtain and display engine status 
                and fault codes. 


   Date: 30/11/2017
   
*/

#ifndef OBD_GAUGES_INCLUDED
#define OBD_GAUGES_INCLUDED

#include <gtk/gtk.h>

gboolean draw_rpm_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_speed_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_ect_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_iat_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_map_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_egr_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_oil_pressure_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_oil_temp_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_fuel_flow_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_fuel_pressure_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_fuel_tank_level_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_pid_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_dtc_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_battery_voltage_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);
gboolean draw_notification_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data);

/* Draw dial labels and values. */
void draw_dial_text(cairo_t *cr, char *gauge_label, char *gauge_numerals, char *gauge_units);

/* Draw dial backgrounds. */
void draw_small_dial_background(cairo_t *cr);
void draw_large_dial_background(cairo_t *cr);
void draw_dial_background(cairo_t *cr, double width, double height);
void draw_dial_tick_gauge(cairo_t *cr, double width, double height, double lower, double upper, double radius, double angle);


#endif

