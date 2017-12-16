cairo_text_extents_t extents;

const char *utf8 = "cairo";
double x,y;

cairo_select_font_face (cr, "Sans",
    CAIRO_FONT_SLANT_NORMAL,
    CAIRO_FONT_WEIGHT_NORMAL);

cairo_set_font_size (cr, 100.0);
cairo_text_extents (cr, utf8, &extents);

x=25.0;
y=150.0;

cairo_move_to (cr, x,y);
cairo_show_text (cr, utf8);

/* draw helping lines */
cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
cairo_set_line_width (cr, 6.0);
cairo_arc (cr, x, y, 10.0, 0, 2*M_PI);
cairo_fill (cr);
cairo_move_to (cr, x,y);
cairo_rel_line_to (cr, 0, -extents.height);
cairo_rel_line_to (cr, extents.width, 0);
cairo_rel_line_to (cr, extents.x_bearing, -extents.y_bearing);
cairo_stroke (cr);




cairo_text_extents_t extents;

const char *utf8 = "cairo";
double x,y;

cairo_select_font_face (cr, "Sans",
    CAIRO_FONT_SLANT_NORMAL,
    CAIRO_FONT_WEIGHT_NORMAL);

cairo_set_font_size (cr, 52.0);
cairo_text_extents (cr, utf8, &extents);
x = 128.0-(extents.width/2 + extents.x_bearing);
y = 128.0-(extents.height/2 + extents.y_bearing);

cairo_move_to (cr, x, y);
cairo_show_text (cr, utf8);

/* draw helping lines */
cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
cairo_set_line_width (cr, 6.0);
cairo_arc (cr, x, y, 10.0, 0, 2*M_PI);
cairo_fill (cr);
cairo_move_to (cr, 128.0, 0);
cairo_rel_line_to (cr, 0, 256);
cairo_move_to (cr, 0, 128.0);
cairo_rel_line_to (cr, 256, 0);
cairo_stroke (cr);



