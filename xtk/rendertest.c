/* NB: should work better under a compositing window manager */
/* gcc -std=c99 `pkg-config --cflags --libs xcb-renderutil` render_gradients.c */

#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/render.h>
#include <xcb/xcb_renderutil.h>


/* try to find a picture/visual id pair which matchs given format */
int find_pictvisual(xcb_connection_t *conn,
		    xcb_pict_standard_t query,
		    xcb_render_pictvisual_t *dest)
{
    xcb_render_query_pict_formats_cookie_t cookie;
    xcb_render_query_pict_formats_reply_t *formats;

    xcb_render_pictforminfo_t *pictforminfo;
    int result = 0;

    if(!dest)
	return -1;

    /* request a list of available formats */
    cookie = xcb_render_query_pict_formats(conn);
    formats = xcb_render_query_pict_formats_reply(conn, cookie, 0);

    pictforminfo = xcb_render_util_find_standard_format(formats, query);

    for(xcb_render_pictscreen_iterator_t screen =
	    xcb_render_query_pict_formats_screens_iterator(formats);
	screen.rem; xcb_render_pictscreen_next(&screen))
    {
	for(xcb_render_pictdepth_iterator_t depth =
		xcb_render_pictscreen_depths_iterator(screen.data);
	    depth.rem; xcb_render_pictdepth_next(&depth))
	{
	    if(depth.data->depth != pictforminfo->depth)
		continue /* not a depth to check */;
	    for(xcb_render_pictvisual_iterator_t visual =
		    xcb_render_pictdepth_visuals_iterator(depth.data);
		visual.rem; xcb_render_pictvisual_next(&visual))
	    {
		if(visual.data->format == pictforminfo->id){
		    dest->format = visual.data->format;
		    dest->visual = visual.data->visual;
		    goto found;
		}
	    }
	}
    }
    result = -1;
found:
    free(formats);
    return result;
}

int show_window(xcb_connection_t *conn)
{
    xcb_screen_t *screen;
    xcb_render_pictvisual_t pictvisual;
    xcb_colormap_t cmap;
    xcb_window_t window;
    xcb_render_picture_t window_pict;
    xcb_render_picture_t radial_grad, conical_grad, linear_grad;

    xcb_generic_event_t *event;

    xcb_rectangle_t rect = {
	.x = 0,
	.y = 0,
	.width = 600,
	.height = 600,
    };
    xcb_render_pointfix_t rect_center = {
	.x = rect.width<<15,
        .y = rect.height<<15,
    };

    /* find a picture/visual which supports alpha */
    if(find_pictvisual(conn, XCB_PICT_STANDARD_ARGB_32, &pictvisual) < 0){
        return -1;
    }

    screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;

    cmap = xcb_generate_id(conn);
    xcb_create_colormap(conn, XCB_COLORMAP_ALLOC_NONE, cmap,
                        screen->root, pictvisual.visual);

    window = xcb_generate_id(conn);
    xcb_create_window(
        conn,
        32 /* argb32 */,
        window,/* new window's id */
        screen->root, /* parent */
        rect.x, rect.y,
        rect.width, rect.height,
        0, /* border  */
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        pictvisual.visual,
        XCB_CW_BACK_PIXEL|XCB_CW_BORDER_PIXEL|XCB_CW_EVENT_MASK|XCB_CW_COLORMAP,
        (uint32_t [])
        {0, 0, XCB_EVENT_MASK_EXPOSURE|XCB_EVENT_MASK_BUTTON_PRESS, cmap});

    /* prepare pictures */
    window_pict = xcb_generate_id(conn);
    xcb_render_create_picture(
        conn,
        window_pict, (xcb_drawable_t)window, pictvisual.format,
        XCB_RENDER_CP_REPEAT, (uint32_t [1]){XCB_RENDER_REPEAT_NORMAL});

    linear_grad = xcb_generate_id(conn);
    xcb_render_create_linear_gradient(
        conn,
        linear_grad /* new picture's id*/,
        (xcb_render_pointfix_t){0, 0} /* from */,
        (xcb_render_pointfix_t){rect.width<<16, rect.height<<16} /* to */,
        4 /* num_stops */,
        (xcb_render_fixed_t []){0, 1<<14, 3<<14, 4<<14},
        (xcb_render_color_t []){
            {.alpha=0x7fff}, {.alpha=0}, {.alpha=0}, {.alpha=0x7fff}
        });

    radial_grad = xcb_generate_id(conn);
    xcb_render_create_radial_gradient(
        conn,
        radial_grad /* new picture's id*/,
        rect_center /* inner center */,
        rect_center /* outer center */,
        (xcb_render_fixed_t)(0) /* inner radious */,
        (xcb_render_fixed_t)(rect.width<<15) /* outer radious */,
        3 /* num_stops */,
        (xcb_render_fixed_t []){1<<14, 3<<14, 4<<14},
        (xcb_render_color_t []){
            {.alpha=0},
            {.alpha=0xffff},
            {.alpha=0}
        });

    conical_grad = xcb_generate_id(conn);
    xcb_render_create_conical_gradient(
        conn,
        conical_grad /* new picture's id*/,
        rect_center /* center */,
        (xcb_render_fixed_t)
        (3.14159265*(180<<16)) /* angle: conpensate pixman's internal offset to draw a full circle */,
        13 /* num_stops */,
        (xcb_render_fixed_t []){0, (1<<16)/12, (2<<16)/12, (3<<16)/12, (4<<16)/12, (5<<16)/12,
            (6<<16)/12, (7<<16)/12, (8<<16)/12, (9<<16)/12, (10<<16)/12, (11<<16)/12, 1<<16},
        (xcb_render_color_t []){
            {.alpha=0x7fff,.red = 0xffff} /*R*/,
            {.alpha=0,     .red = 0xffff,.green = 0x7fff},
            {.alpha=0x7fff,.red = 0xffff,.green = 0xffff},
            {.alpha=0,     .red = 0x7fff,.green = 0xffff},
            {.alpha=0x7fff,              .green = 0xffff} /*G*/,
            {.alpha=0,                   .green = 0xffff,.blue = 0x7fff},
            {.alpha=0x7fff,              .green = 0xffff,.blue = 0xffff},
            {.alpha=0,                   .green = 0x7fff,.blue = 0xffff},
            {.alpha=0x7fff,                              .blue = 0xffff}, /*B*/
            {.alpha=0,     .red = 0x7fff,                .blue = 0xffff},
            {.alpha=0x7fff,.red = 0xffff,                .blue = 0xffff},
            {.alpha=0,     .red = 0xffff,                .blue = 0x7fff},
            {.alpha=0x7fff,.red = 0xffff} /*another R (to connect 1.0->0.0 smoothly)*/,
        });

    xcb_map_window(conn, window);
    xcb_flush(conn);

    while( (event = xcb_wait_for_event(conn)) ) {
        if(event->response_type == XCB_EXPOSE){
            /* redraw contents */
            /* initialize the window background */
            xcb_render_composite(conn, XCB_RENDER_PICT_OP_SRC,
                                 linear_grad, 0 /* no alpha_pict */,
                                 window_pict,
                                 0, 0, /*src*/
                                 0, 0, /*mask*/
                                 0, 0, /*dst*/
                                 rect.width, rect.height);

            /* compose a color wheel with a radial alpha mask */
            xcb_render_composite(conn, XCB_RENDER_PICT_OP_OVER,
                                 conical_grad, radial_grad,
                                 window_pict,
                                 0, 0, /*src*/
                                 0, 0, /*mask*/
                                 0, 0, /*dst*/
                                 rect.width, rect.height);
            xcb_flush(conn);
        }
        if(event->response_type == XCB_BUTTON_PRESS){
            free(event);
            break;
        }
        free(event);
    }

    xcb_render_free_picture(conn, linear_grad);
    xcb_render_free_picture(conn, conical_grad);
    xcb_render_free_picture(conn, radial_grad);

    xcb_render_free_picture(conn, window_pict);

    xcb_free_colormap(conn, cmap);
    xcb_flush(conn);

    return 0;
}

int main()
{
    xcb_connection_t *conn;
    int screen_num;

    conn = xcb_connect(NULL, &screen_num);

    show_window(conn);

    xcb_disconnect(conn);

    exit(0);
}


