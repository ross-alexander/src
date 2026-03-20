#include <gtk/gtk.h>

typedef struct _Data Data;
struct _Data
{
  GtkWidget *box, *image, *entry;
  GdkPixbuf *pixbuf;
};

static cairo_t   *pixbuf_cairo_create ( GdkPixbuf *pixbuf );
static GdkPixbuf *pixbuf_cairo_destroy( cairo_t   *cr, gboolean   create_new_pixbuf );
static void       cb_new_image        ( GtkButton *button, Data      *data );
static void       cb_update_image     ( GtkButton *button, Data      *data );

/* Key for automated pixbuf updating and destruction */

static const cairo_user_data_key_t pixbuf_key;

int main( int  argc, char **argv )
{
   GtkWidget *window,
           *vbox,
           *swindow,
           *hbox,
           *image,
           *entry,
           *button;
   GdkPixbuf *pixbuf;
   Data      *data;

   gtk_init( &argc, &argv );

   data = g_slice_new( Data );

   window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
   g_signal_connect( G_OBJECT( window ), "destroy",
                 G_CALLBACK( gtk_main_quit ), NULL );

   vbox = gtk_vbox_new( FALSE, 6 );
   gtk_container_add( GTK_CONTAINER( window ), vbox );

   swindow = gtk_scrolled_window_new( NULL, NULL );
   gtk_box_pack_start( GTK_BOX( vbox ), swindow, TRUE, TRUE, 0 );

   hbox = gtk_hbox_new( TRUE, 6 );
   data->box = hbox;
   gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW( swindow ),
                                 hbox );

   pixbuf = gdk_pixbuf_new( GDK_COLORSPACE_RGB, FALSE, 8, 200, 200 );
   gdk_pixbuf_fill( pixbuf, 0xffff00ff );
   data->pixbuf = pixbuf;
   image = gtk_image_new_from_pixbuf( pixbuf );
   data->image = image;
   g_object_unref( G_OBJECT( pixbuf ) );
   gtk_box_pack_start( GTK_BOX( hbox ), image, FALSE, FALSE, 0 );

   hbox = gtk_hbox_new( FALSE, 6 );
   gtk_box_pack_start( GTK_BOX( vbox ), hbox, FALSE, FALSE, 0 );

   entry = gtk_entry_new();
   data->entry = entry;
   gtk_box_pack_start( GTK_BOX( hbox ), entry, TRUE, TRUE, 0 );

   button = gtk_button_new_with_label( "Create new image" );
   g_signal_connect( G_OBJECT( button ), "clicked",
                 G_CALLBACK( cb_new_image ), data );
   gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );

   button = gtk_button_new_with_label( "Update original image" );
   g_signal_connect( G_OBJECT( button ), "clicked",
                 G_CALLBACK( cb_update_image ), data );
   gtk_box_pack_start( GTK_BOX( hbox ), button, FALSE, FALSE, 0 );

   gtk_widget_show_all( window );

   gtk_main();

   g_slice_free( Data, data );

   return( 0 );
}

/**
* pixbuf_cairo_create:
* @pixbuf: GdkPixbuf that you wish to wrap with cairo context
*
* This function will initialize new cairo context with contents of @pixbuf. You
* can then draw using returned context. When finished drawing, you must call
* pixbuf_cairo_destroy() or your pixbuf will not be updated with new contents!
*
* Return value: New cairo_t context. When you're done with it, call
* pixbuf_cairo_destroy() to update your pixbuf and free memory.
*/
static cairo_t *
pixbuf_cairo_create( GdkPixbuf *pixbuf )
{
   gint             width,        /* Width of both pixbuf and surface */
     height,       /* Height of both pixbuf and surface */
     p_stride,     /* Pixbuf stride value */
     p_n_channels, /* RGB -> 3, RGBA -> 4 */
     s_stride;     /* Surface stride value */
   guchar          *p_pixels,     /* Pixbuf's pixel data */
     *s_pixels;     /* Surface's pixel data */
   cairo_surface_t *surface;      /* Temporary image surface */
   cairo_t         *cr;           /* Final context */

   g_object_ref( G_OBJECT( pixbuf ) );

   /* Inspect input pixbuf and create compatible cairo surface */
   g_object_get( G_OBJECT( pixbuf ), "width",           &width,
                             "height",          &height,
                             "rowstride",       &p_stride,
                             "n-channels",      &p_n_channels,
                             "pixels",          &p_pixels,
                             NULL );
   surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, width, height );
   s_stride = cairo_image_surface_get_stride( surface );
   s_pixels = cairo_image_surface_get_data( surface );

   /* Copy pixel data from pixbuf to surface */
   while( height-- )
   {
      gint    i;
      guchar *p_iter = p_pixels,
            *s_iter = s_pixels;

      for( i = 0; i < width; i++ )
      {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
         /* Pixbuf:  RGB(A)
          * Surface: BGRA */
         if( p_n_channels == 3 )
         {
            s_iter[0] = p_iter[2];
            s_iter[1] = p_iter[1];
            s_iter[2] = p_iter[0];
            s_iter[3] = 0xff;
         }
         else /* p_n_channels == 4 */
         {
            gdouble alpha_factor = p_iter[3] / (gdouble)0xff;

            s_iter[0] = (guchar)( p_iter[2] * alpha_factor + .5 );
            s_iter[1] = (guchar)( p_iter[1] * alpha_factor + .5 );
            s_iter[2] = (guchar)( p_iter[0] * alpha_factor + .5 );
            s_iter[3] =           p_iter[3];
         }
#elif G_BYTE_ORDER == G_BIG_ENDIAN
         /* Pixbuf:  RGB(A)
          * Surface: ARGB */
         if( p_n_channels == 3 )
         {
            s_iter[3] = p_iter[2];
            s_iter[2] = p_iter[1];
            s_iter[1] = p_iter[0];
            s_iter[0] = 0xff;
         }
         else /* p_n_channels == 4 */
         {
            gdouble alpha_factor = p_iter[3] / (gdouble)0xff;

            s_iter[3] = (guchar)( p_iter[2] * alpha_factor + .5 );
            s_iter[2] = (guchar)( p_iter[1] * alpha_factor + .5 );
            s_iter[1] = (guchar)( p_iter[0] * alpha_factor + .5 );
            s_iter[0] =           p_iter[3];
         }
#else /* PDP endianness */
         /* Pixbuf:  RGB(A)
          * Surface: RABG */
         if( p_n_channels == 3 )
         {
            s_iter[0] = p_iter[0];
            s_iter[1] = 0xff;
            s_iter[2] = p_iter[2];
            s_iter[3] = p_iter[1];
         }
         else /* p_n_channels == 4 */
         {
            gdouble alpha_factor = p_iter[3] / (gdouble)0xff;

            s_iter[0] = (guchar)( p_iter[0] * alpha_factor + .5 );
            s_iter[1] =           p_iter[3];
            s_iter[2] = (guchar)( p_iter[2] * alpha_factor + .5 );
            s_iter[3] = (guchar)( p_iter[1] * alpha_factor + .5 );
         }
#endif
         s_iter += 4;
         p_iter += p_n_channels;
      }
      s_pixels += s_stride;
      p_pixels += p_stride;
   }

   /* Create context and set user data */
   cr = cairo_create( surface );
   cairo_surface_destroy( surface );
   cairo_set_user_data( cr, &pixbuf_key, pixbuf, g_object_unref );

   /* Return context */
   return( cr );
}

/**
* pixbuf_cairo_destroy:
* @cr: Cairo context that you wish to destroy
* @create_new_pixbuf: If TRUE, new pixbuf will be created and returned. If
*                     FALSE, input pixbuf will be updated in place.
*
* This function will destroy cairo context, created with pixbuf_cairo_create().
*
* Return value: New or updated GdkPixbuf. You own a new reference on return
* value, so you need to call g_object_unref() on returned pixbuf when you don't
* need it anymore.
*/
static GdkPixbuf *
pixbuf_cairo_destroy( cairo_t  *cr,
                    gboolean  create_new_pixbuf )
{
   gint             width,        /* Width of both pixbuf and surface */
                height,       /* Height of both pixbuf and surface */
                p_stride,     /* Pixbuf stride value */
                p_n_channels, /* RGB -> 3, RGBA -> 4 */
                s_stride;     /* Surface stride value */
   guchar          *p_pixels,     /* Pixbuf's pixel data */
               *s_pixels;     /* Surface's pixel data */
   cairo_surface_t *surface;      /* Temporary image surface */
   GdkPixbuf       *pixbuf,       /* Pixbuf to be returned */
               *tmp_pix;      /* Temporary storage */

   /* Obtain pixbuf to be returned */
   tmp_pix = cairo_get_user_data( cr, &pixbuf_key );
   if (create_new_pixbuf)
      pixbuf = gdk_pixbuf_copy(tmp_pix);
   else
     pixbuf = GDK_PIXBUF(g_object_ref(G_OBJECT(tmp_pix)));

   /* Obtain surface from where pixel values will be copied */
   surface = cairo_get_target( cr );

   /* Inspect pixbuf and surface */
   g_object_get( G_OBJECT( pixbuf ), "width",           &width,
                             "height",          &height,
                             "rowstride",       &p_stride,
                             "n-channels",      &p_n_channels,
                             "pixels",          &p_pixels,
                             NULL );
   s_stride = cairo_image_surface_get_stride( surface );
   s_pixels = cairo_image_surface_get_data( surface );

   /* Copy pixel data from surface to pixbuf */
   while( height-- )
   {
      gint    i;
      guchar *p_iter = p_pixels,
            *s_iter = s_pixels;

      for( i = 0; i < width; i++ )
      {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
         /* Pixbuf:  RGB(A)
          * Surface: BGRA */
         gdouble alpha_factor = (gdouble)0xff / s_iter[3];

         p_iter[0] = (guchar)( s_iter[2] * alpha_factor + .5 );
         p_iter[1] = (guchar)( s_iter[1] * alpha_factor + .5 );
         p_iter[2] = (guchar)( s_iter[0] * alpha_factor + .5 );
         if( p_n_channels == 4 )
            p_iter[3] = s_iter[3];
#elif G_BYTE_ORDER == G_BIG_ENDIAN
         /* Pixbuf:  RGB(A)
          * Surface: ARGB */
         gdouble alpha_factor = (gdouble)0xff / s_iter[0];

         p_iter[0] = (guchar)( s_iter[1] * alpha_factor + .5 );
         p_iter[1] = (guchar)( s_iter[2] * alpha_factor + .5 );
         p_iter[2] = (guchar)( s_iter[3] * alpha_factor + .5 );
         if( p_n_channels == 4 )
            p_iter[3] = s_iter[0];
#else /* PDP endianness */
         /* Pixbuf:  RGB(A)
          * Surface: RABG */
         gdouble alpha_factor = (gdouble)0xff / s_iter[1];

         p_iter[0] = (guchar)( s_iter[0] * alpha_factor + .5 );
         p_iter[1] = (guchar)( s_iter[3] * alpha_factor + .5 );
         p_iter[2] = (guchar)( s_iter[2] * alpha_factor + .5 );
         if( p_n_channels == 4 )
            p_iter[3] = s_iter[1];
#endif
         s_iter += 4;
         p_iter += p_n_channels;
      }
      s_pixels += s_stride;
      p_pixels += p_stride;
   }

   /* Destroy context */
   cairo_destroy( cr );

   /* Return pixbuf */
   return( pixbuf );
}

static void
cb_new_image( GtkButton *button,
           Data      *data )
{
   GtkWidget   *image;
   GdkPixbuf   *new_pix;
   PangoLayout *layout;
   cairo_t     *cr;

   cr = pixbuf_cairo_create( data->pixbuf );
   cairo_set_source_rgb( cr, 0, 0, 0 );
   cairo_move_to( cr, 0, 0 );
   layout = pango_cairo_create_layout( cr );
   pango_layout_set_text( layout,
                     gtk_entry_get_text( GTK_ENTRY( data->entry ) ), -1 );
   pango_cairo_show_layout( cr, layout );
   g_object_unref( G_OBJECT( layout ) );
   new_pix = pixbuf_cairo_destroy( cr, TRUE );

   image = gtk_image_new_from_pixbuf( new_pix );
   g_object_unref( G_OBJECT( new_pix ) );
   gtk_box_pack_start( GTK_BOX( data->box ), image, FALSE, FALSE, 0 );
   gtk_widget_show( image );
}

static void
cb_update_image( GtkButton *button,
             Data      *data )
{
   GdkPixbuf   *new_pix;
   PangoLayout *layout;
   cairo_t     *cr;

   cr = pixbuf_cairo_create( data->pixbuf );
   cairo_set_source_rgb( cr, 0, 0, 0 );
   cairo_move_to( cr, 0, 0 );
   layout = pango_cairo_create_layout( cr );
   pango_layout_set_text( layout,
                     gtk_entry_get_text( GTK_ENTRY( data->entry ) ), -1 );
   pango_cairo_show_layout( cr, layout );
   g_object_unref( G_OBJECT( layout ) );
   new_pix = pixbuf_cairo_destroy( cr, FALSE );
   g_object_unref( G_OBJECT( new_pix ) );

   gtk_widget_queue_draw( data->image );
}

/* EOF */
