# csheet

csheet - for contact sheet - creates a output image based on images in
aa directory and a CSS stylesheet.  It uses GEGL internally to do the
image scaling etc.  To get the styling libxml2 is used to create a DOM
object then libcroco to apply the CSS.

# thumbnail

Based on gdk-pixbuf and cairo (also requires GDK for the function that
converts gdk-pixbuf to cairo).  At the time (2021) I could not find a
good CSS library so used lua to do all the glue logic.
