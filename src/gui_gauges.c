/* 
   Project: OBD-II Monitor (On-Board Diagnostics)

   Author: Derek Chadwick

   Description: A GUI for communication with vehicle engine control units via 
                an OBD-II interface to obtain and display engine status 
                and fault codes. 


   Date: 30/11/2017
   
*/


#include <gtk/gtk.h>
#include <math.h>
#include "obd_monitor.h"
#include "protocols.h"
#include "gui_dialogs.h"

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


/* Guage Draw Callbacks. */

void draw_dial_background(cairo_t *cr, double width, double height)
{
   /* */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 10.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   /* Draw the background shapes. */
   
   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
   cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
   cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
   cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);

   return;
}

void draw_dial_tick_gauge(cairo_t *cr, double width, double height, double lower, double upper, double radius, double angle)
{
   /* Draw the gauge and needle */
   
   DialPoint points[6];
   double s,c;
   double theta, last, increment;
   double xc, yc;
   double tick_length;
   int i, inc;
   double pointer_width = 10.0;
  
   xc = width / 2.0;
   yc = height / 2.0;

   /* Draw ticks */

   if ((upper - lower) == 0)
     return;

   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width(cr, 2.0);
   
   increment = (100*NUM_PI) / (radius*radius);

   inc = (upper - lower);

   while (inc < 100) inc *= 10;
   while (inc >= 1000) inc /= 10;
   last = -1;

   for (i = 0; i <= inc; i++)
   {
      theta = ((double)i*NUM_PI / (18*inc/24.0) - NUM_PI/6.0);

      if ((theta - last) < (increment))
        continue;     
      last = theta;

      s = sin (theta);
      c = cos (theta);

      tick_length = (i%(inc/10) == 0) ? pointer_width : pointer_width / 2.0;

      cairo_move_to(cr, xc + c*(radius - tick_length), yc - s*(radius - tick_length));
      cairo_line_to(cr, xc + c*radius, yc - s*radius);
      
   }
   cairo_stroke(cr);

   /* Draw pointer */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 2.0);
   cairo_move_to(cr, xc, yc);
   
   s = sin (angle);
   c = cos (angle);

   points[0].x = xc + s*pointer_width/2;
   points[0].y = yc + c*pointer_width/2;
   points[1].x = xc + c*radius;
   points[1].y = yc - s*radius;
   points[2].x = xc - s*pointer_width/2;
   points[2].y = yc - c*pointer_width/2;
   points[3].x = xc - c*radius/10;
   points[3].y = yc + s*radius/10;
   points[4].x = points[0].x;
   points[4].y = points[0].y;


   for(i = 0; i < 4; i++)
   {
     cairo_move_to(cr, points[i].x, points[i].y);
     cairo_line_to(cr, points[i+1].x, points[i+1].y);
   }
   

   cairo_stroke(cr);
    
   return;
}

void draw_dial_text(cairo_t *cr, char *gauge_label, char *gauge_numerals, char *gauge_units)
{
   cairo_text_extents_t ctext;
   double xc;
   double yc;
   
   cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    
   cairo_set_font_size(cr, 24);
   cairo_text_extents (cr, gauge_numerals, &ctext);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   xc = (100.0 - (0.5 * ctext.width + ctext.x_bearing));
   yc = (75.0 - (0.5 * ctext.height + ctext.y_bearing));
   cairo_move_to(cr, xc, yc);
   cairo_show_text(cr, gauge_numerals);

   /* TEST TEXT PLACEMENT
   xc = (100.0 - (0.5 * ctext.width  + ctext.x_bearing));
   yc = (75 - (0.5 * ctext.height + ctext.y_bearing));   
   cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
   cairo_set_line_width (cr, 6.0);
   cairo_arc (cr, xc, yc, 10.0, 0, 2*M_PI);
   cairo_fill (cr);
   cairo_move_to (cr, xc,yc);
   cairo_rel_line_to (cr, 0, -ctext.height);
   cairo_rel_line_to (cr, ctext.width, 0);
   cairo_rel_line_to (cr, ctext.x_bearing, -ctext.y_bearing);
   cairo_stroke (cr);
   */
 
   cairo_set_font_size(cr, 10);
   cairo_text_extents (cr, gauge_units, &ctext);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   xc = (100.0 - (0.5 * ctext.width + ctext.x_bearing));
   yc = (100.0 - (0.5 * ctext.height + ctext.y_bearing));
   cairo_move_to(cr, xc, yc);
   cairo_show_text(cr, gauge_units);  
   
   cairo_set_font_size(cr, 15);
   cairo_text_extents (cr, gauge_label, &ctext);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (100.0 - (0.5 * ctext.width  + ctext.x_bearing)), 135);
   cairo_show_text(cr, gauge_label);
  
   return;
}


gboolean draw_rpm_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   double rpm_scale_factor = 38.2;
   double engine_rpm = get_engine_rpm(); /* Get rpm value from protocol module. */
   double gauge_rpm = engine_rpm / rpm_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_rpm / radius); /* Angle in radians. */
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI; /* 210 degrees */
   cairo_text_extents_t ctext;
   double cpx;
   double cpy;
   char gauge_numerals[16];
   
   /* SCALE FACTOR:
   
      scale factor = max rpm on gauge / arc length of gauge.    
      arc length = 2 * PI * radius * theta/360 if angle is in degrees. 
      arc length = radius * theta  if the angle is in radians.   

      Example: max gauge rpm = 8000 ... min gauge rpm = 0 
               gauge min angle = 1.167 * PI (210 degrees) 
               gauge max angle = -0.167 * PI (-30 degrees or 330 degrees)
               scale factor 8000 / (arc length between gauge min and max angles)
               where arc length = (radius * 1.334 * PI) radians.
              
               Thence:
              
               scale factor = 8000 / (radius * 2 * PI * ((30 + 210) / 360))
               
               or 
               
               scale factor = 8000 / (radius * (1.167 * PI + 0.167 * PI))
                            = 8000 / (50 * 1.334 * PI)
                            = 8000 / 209.5
                            = 38.2
              
     NOTE: cairo arc drawing is counter clock-wise when 
           the cairo_arc_negative() function is used, and clock-wise when
           the cairo_arc() function is used, which is the opposite
           of geometry/trigonometry, so what the hell.
           
           Check the API docs for a confusing explanation:
           
           https://cairographics.org/manual/cairo-Paths.html#cairo-arc
   */
                                   
 
   /* double dial_start_angle = 0.25 * NUM_PI;  45 degrees */
   /* double dial_end_angle = -1.25 * NUM_PI;   225 degrees */
   
   draw_dial_background(cr, 190, 140);

   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);   
   cairo_arc_negative (cr, xc, yc, radius, gauge_start_angle, gauge_end_angle);
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
   sprintf(gauge_numerals, "%.0f", engine_rpm);
   draw_dial_text(cr, "Engine RPM", gauge_numerals, "rpm");
  
   return TRUE;
}


gboolean draw_speed_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI; /* 210 degrees */
   double speed_scale_factor = 0.954; /* 200km/h / (radius * (1.167 * PI + 0.167 * PI)) = 200 / 209.5 = 0.954 */
   double vehicle_speed = get_vehicle_speed(); /* Get speed value from protocol module. */
   double gauge_speed = vehicle_speed / speed_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_speed / radius); /* Angle in radians. */
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
   sprintf(gauge_numerals, "%.0f", vehicle_speed);
   draw_dial_text(cr, "Speedometer", gauge_numerals, "km/h");
  
   return TRUE;
}

gboolean draw_ect_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double ect_scale_factor = 0.764;   /* 160C / (radius * (1.167 * PI + 0.167 * PI)) = 160 / 209.5 = 0.764 */
   double coolant_temperature = get_coolant_temperature(); /* Get ect value from protocol module. */
   double gauge_temp = coolant_temperature / ect_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_temp / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;
   double cpx;
   double cpy;
   char gauge_numerals[16];
   
   /* Draw gauge background and arc. */
   draw_dial_background(cr, 190, 140);
   
   cairo_arc_negative(cr, xc, yc, radius, gauge_start_angle, gauge_end_angle);
   cairo_stroke(cr);

   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_set_line_width(cr, 3.0);
   cairo_arc(cr, xc, yc, radius, gauge_end_angle, needle_angle); 
   cairo_get_current_point(cr, &cpx, &cpy);
   cairo_stroke(cr);
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", coolant_temperature);
   draw_dial_text(cr, "Coolant Temp", gauge_numerals, "℃");

  
   return TRUE;
}

gboolean draw_iat_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double iat_scale_factor = 0.764;   /* 160C / (radius * (1.167 * PI + 0.167 * PI)) = 160 / 209.5 = 0.764 */
   double air_temperature = get_intake_air_temperature();     /* Get iat value from protocol module. */
   double gauge_temp = air_temperature / iat_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_temp / radius); /* Angle in radians. */
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
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0); /* Indicator Dot */
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", air_temperature);
   draw_dial_text(cr, "Intake Air Temp", gauge_numerals, "℃");
   
   return TRUE;
}

gboolean draw_map_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double map_scale_factor = 1.217;   /* 255 / (radius * (1.167 * PI + 0.167 * PI)) = 255 / 209.5 = 1.217 */
   double air_pressure = get_manifold_pressure(); /* Get manifold pressure from protocol module. */
   double gauge_temp = air_pressure / map_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_temp / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;
   char gauge_numerals[16];
   double cpx;
   double cpy;
   
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
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0); /* Indicator Dot */
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", air_pressure);
   draw_dial_text(cr, "Manifold Pressure", gauge_numerals, "kPa");
  
   return TRUE;
}

gboolean draw_egr_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   draw_dial_background(cr, 190, 140);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"EGR Pressure",&ctext);
   printf("Text width: %f\n", ctext.width);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "EGR Pressure");
  
   return TRUE;
}

gboolean draw_oil_pressure_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double angle1 = 0.25 * NUM_PI; /* 45.0  * (M_PI/180.0);   angles are specified */
   double angle2 = NUM_PI;        /* 180.0 * (M_PI/180.0);   in radians           */
   
   double dial_start_angle = 0.25 * NUM_PI;
   double dial_end_angle = -1.25 * NUM_PI;

   cairo_text_extents_t ctext;
   
   draw_dial_background(cr, 190, 140);
   
   cairo_arc_negative(cr, xc, yc, radius, dial_start_angle, dial_end_angle);
   cairo_stroke(cr);

   /* draw helping lines */
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   cairo_set_line_width(cr, 3.0);

   cairo_arc (cr, xc, yc, 5.0, 0.0, 2*NUM_PI);
   cairo_fill(cr);

   cairo_arc(cr, xc, yc, radius, angle2, angle2);
   cairo_line_to(cr, xc, yc);
   cairo_stroke(cr);

   cairo_text_extents (cr,"Oil Pressure",&ctext);
   /* printf("Text width: %f\n", ctext.width); */
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (80.0 - (0.5 * ctext.width)), 135);
   cairo_set_font_size(cr, 15);
   cairo_show_text(cr, "Oil Pressure");
  
   return TRUE;
}


gboolean draw_oil_temp_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double oil_temp_scale_factor = 0.764;   /* 160C / (radius * (1.167 * PI + 0.167 * PI)) = 160 / 209.5 = 0.764 */
   double oil_temperature = get_oil_temperature(); /* TODO: Get oil temperature from protocol module. */
   double gauge_temp = oil_temperature / oil_temp_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_temp / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;
   char gauge_numerals[16];
   double cpx;
   double cpy;
   
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
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0); /* Indicator Dot */
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", oil_temperature);
   draw_dial_text(cr, "Oil Temperature", gauge_numerals, "℃");

   return TRUE;
}

gboolean draw_fuel_flow_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double fuel_flow_scale_factor = 0.783; /* 164.0 / (radius * (1.167 * PI + 0.167 * PI)) */
   double fuel_flow = get_fuel_flow_rate();     /* Get fuel flow rate from protocol module. */
   double gauge_temp = fuel_flow / fuel_flow_scale_factor; /* this is the gauge arc length for the needle. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_temp / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;
   char gauge_numerals[16];
   double cpx;
   double cpy;
   
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
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0); /* Indicator Dot */
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", fuel_flow);
   draw_dial_text(cr, "Fuel Flow Rate", gauge_numerals, "L/h");
   
   return TRUE;
}

gboolean draw_fuel_pressure_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double fuel_pressure_scale_factor = 3.651; /* 765.0 / (radius * (1.167 * PI + 0.167 * PI)) */
   double fuel_pressure = get_fuel_pressure();     /* Get fuel value from protocol module. */
   double gauge_pressure = fuel_pressure / fuel_pressure_scale_factor; /* this is the gauge arc length. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_pressure / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;
   char gauge_numerals[16];
   double cpx;
   double cpy;
   
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
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0); /* Indicator Dot */
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", fuel_pressure);
   draw_dial_text(cr, "Fuel Pressure", gauge_numerals, "kPa");
   
  
   return TRUE;
}

gboolean draw_fuel_tank_level_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double fuel_level_scale_factor = 0.477; /* 100.0 / (radius * (1.167 * PI + 0.167 * PI)) */
   double fuel_level = get_fuel_tank_level(); /* Get fuel level from protocol module. */
   double gauge_level = fuel_level / fuel_level_scale_factor; /* this is the gauge arc length. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_level / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;
   char gauge_numerals[16];
   double cpx;
   double cpy;
   
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
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0); /* Indicator Dot */
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", fuel_level);
   draw_dial_text(cr, "Fuel Level", gauge_numerals, "%");
   
   return TRUE;
}

gboolean draw_timing_advance_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 100.0;
   double yc = 75.0;
   double radius = 50.0;
   double gauge_start_angle = 0.167 * NUM_PI; /* 30 degrees */
   double gauge_end_angle = -1.167 * NUM_PI;  /* 210 degrees */
   double timing_advance_scale_factor = 0.477; /* 63.5 / (radius * (1.167 * PI + 0.167 * PI)) */
   double timing_advance = get_timing_advance(); /* Get the timing advance from protocol module. */
   double gauge_level = timing_advance / timing_advance_scale_factor; /* TODO: check for negative values. */
   double needle_angle = (-1.167 * NUM_PI) + (gauge_level / radius); /* Angle in radians. */
   cairo_text_extents_t ctext;
   char gauge_numerals[16];
   double cpx;
   double cpy;
   
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
   
   cairo_set_source_rgb(cr, 0.634, 0.0, 0.0); /* Indicator Dot */
   cairo_arc (cr, cpx, cpy, 5.0, 0.0, 2*NUM_PI); 
   cairo_fill(cr);

   /* Draw gauge text. */
   sprintf(gauge_numerals, "%.0f", timing_advance);
   draw_dial_text(cr, "Timing Advance", gauge_numerals, "Deg");
   
   return TRUE;
}


void draw_large_dial_background(cairo_t *cr)
{
   /* a custom shape that could be wrapped in a function */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          width         = 290,
          height        = 220,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 10.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees);
   cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees);
   cairo_arc (cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees);
   cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees);
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);
   
   return;
}

gboolean draw_pid_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   cairo_text_extents_t ctext;
   
   /* TODO: get the last requested PID info. */
   draw_large_dial_background(cr);

   cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 28);                        
   cairo_text_extents (cr,"PID",&ctext);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (150.0 - (0.5 * ctext.width + ctext.x_bearing)), 210);
   cairo_show_text(cr, "PID");
  
   return TRUE;
}

gboolean draw_dtc_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   int mil_status;
   int dtc_count;
   char dtc_msg[256];
   char mil_msg[256];
   char dtc_code[256];
   cairo_text_extents_t ctext;

   draw_large_dial_background(cr);

   memset(dtc_msg, 0, 256);
   memset(mil_msg, 0, 256);
   memset(dtc_code, 0, 256);
   
   mil_status = get_mil_status();
   dtc_count = get_dtc_count();
   get_last_dtc_code(dtc_code);
   
   if (mil_status == 1)
   {
      sprintf(mil_msg, "MIL ON");
      cairo_set_source_rgb(cr, 0.634, 0.0, 0.0);
   }
   else
   {
      sprintf(mil_msg, "MIL OFF");
      cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   }
      
   sprintf(dtc_msg, "DTC Count: %d", dtc_count);
   
   cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 16);
   
   cairo_text_extents (cr,mil_msg,&ctext);
   cairo_move_to(cr, (150.0 - (0.5 * ctext.width + ctext.x_bearing)), 30);
   cairo_show_text(cr, mil_msg);
   
   cairo_text_extents (cr,dtc_msg,&ctext);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (150.0 - (0.5 * ctext.width + ctext.x_bearing)), 50);
   cairo_show_text(cr, dtc_msg);

   cairo_set_font_size(cr, 36);
   cairo_text_extents (cr,dtc_code,&ctext);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (150.0 - (0.5 * ctext.width + ctext.x_bearing)), 120);
   cairo_show_text(cr, dtc_code);
   
   cairo_set_font_size(cr, 28);
   cairo_text_extents (cr,"DTC",&ctext);
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (150.0 - (0.5 * ctext.width + ctext.x_bearing)), 210);
   cairo_show_text(cr, "DTC");
   
   return TRUE;
}

void draw_small_dial_background(cairo_t *cr)
{
   /* a custom shape that could be wrapped in a function */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          width         = 290,
          height        = 30,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 2.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 90 * degrees);
   /* cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees); */
   cairo_arc (cr, x + radius, y + radius, radius, 90 * degrees, 270 * degrees);
   /* cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees); */
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);
   
   return;
}

gboolean draw_battery_voltage_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 10.0;
   double yc = 25.0;
   double battery_voltage = get_battery_voltage();
   cairo_text_extents_t ctext;
   char volts[256];
   
   memset(volts, 0, 256);
   sprintf(volts, "Battery Voltage: %.1fV", battery_voltage);
   
   draw_small_dial_background(cr);

   cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 16);
   cairo_text_extents (cr, volts, &ctext); /* TODO: get voltage from protocol module. */
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, (150.0 - (0.5 * ctext.width + ctext.x_bearing)), yc);
   cairo_show_text(cr, volts);
  
   return TRUE;
}

void draw_notification_dial_background(cairo_t *cr)
{
   /* a custom shape that could be wrapped in a function */
   double x         = 5.0,                /* parameters like cairo_rectangle */
          y         = 5.0,
          width         = 970,
          height        = 30,
          aspect        = 1.0,             /* aspect ratio */
          corner_radius = height / 2.0;   /* and corner curvature radius */

   double radius = corner_radius / aspect;
   double degrees = NUM_PI / 180.0;

   cairo_new_sub_path (cr);
   cairo_arc (cr, x + width - radius, y + radius, radius, -90 * degrees, 90 * degrees);
   /* cairo_arc (cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees); */
   cairo_arc (cr, x + radius, y + radius, radius, 90 * degrees, 270 * degrees);
   /* cairo_arc (cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees); */
   cairo_close_path (cr);

   cairo_set_source_rgb (cr, 0.125, 0.29, 0.53);
   cairo_fill_preserve (cr);
   cairo_set_source_rgb (cr, 0.447, 0.624, 0.812);
   cairo_set_line_width (cr, 5.0);
   cairo_stroke (cr);
   
   return;
}

gboolean draw_notification_dial(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
   double xc = 20.0;
   double yc = 25.0;
   cairo_text_extents_t ctext;
   char notification[256];
   
   memset(notification, 0, 256); /* TODO: get notifcation message.  */
   get_status_bar_msg(notification);
   if (strlen(notification) < 1)
   {
      strcpy(notification, "String Error.");
   }
   
   draw_notification_dial_background(cr);

   cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 16);
   cairo_text_extents (cr, notification, &ctext); 
   cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
   cairo_move_to(cr, xc, yc);
   cairo_show_text(cr, notification);
  
   return TRUE;
}



