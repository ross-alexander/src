#include <assert.h>
#include <stdio.h>

#include <X11/Intrinsic.h>	/* Include standard Toolkit Header file.
				   We do no need "StringDefs.h" */
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/Label.h>	/* Include the Label widget's header file. */
#include <X11/Xaw/Cardinals.h>	/* Definition of ZERO. */


#include <X11/Xaw/Box.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>

#include <cairo/cairo-xlib.h>
#include "common.h"
#include "visual.h"

String fallback_resources[] = { "*Label.Label:    Hello, World", NULL };

typedef struct xtk_xaw_t {
  xtk_t *xtk;
  Colormap cm;
  Widget toplevel, form, draw;
  Visual *vis;
  int width;
  int height;
  cairo_surface_t *surface;
} xtk_xaw_t;


/* ----------------------------------------------------------------------
--
-- do_configure
--
---------------------------------------------------------------------- */

void do_configure(Widget w, XtPointer client_data, XEvent *ev, Boolean *cont)
{
  Dimension width, height;
  xtk_xaw_t *x = (xtk_xaw_t*)client_data;

  XtVaGetValues(w,
		XtNwidth, &width,
		XtNheight, &height,
		NULL);


  x->width = width;
  x->height = height;
  cairo_xlib_surface_set_size(x->surface, x->width, x->height);
  printf("Xaw: Configure %d %d\n", width, height);
}

/* ----------------------------------------------------------------------
--
-- do_expose
--
---------------------------------------------------------------------- */

void do_expose(Widget w, XtPointer client_data, XEvent *ev, Boolean *cont)
{
  xtk_xaw_t *x = (xtk_xaw_t*)client_data;
  printf("Xaw: Expose\n");
  xtk_draw_surface(x->xtk, x->surface);
}

/* ----------------------------------------------------------------------
--
-- do_xaw
--
---------------------------------------------------------------------- */

int do_xtk(int argc, char *argv[], unsigned nwin, xtk_t **xtk)
{
  XtToolkitInitialize();
  XtAppContext appContext = XtCreateApplicationContext();
  Display *dpy = XtOpenDisplay(appContext, NULL, NULL, "Tcairo", NULL, 0, &argc, argv);
  
  int screenid = XDefaultScreen(dpy);
  Drawable root = XRootWindow(dpy, screenid);

  struct xtk_xaw_t xaw[nwin];
  
  for (int i = 0; i < nwin; i++)
    {
      struct xtk_xaw_t* t = xaw + i;
      t->xtk = xtk[i];
      t->vis = get_visual(dpy, t->xtk);
      t->cm = XCreateColormap(dpy, root, t->vis, AllocNone);

      char *title = t->xtk->title ? t->xtk->title : "xtk-xaw";
      
      t->toplevel = XtVaAppCreateShell(title, "Tcairo", applicationShellWidgetClass, dpy, 
				       XtNdepth,		t->xtk->depth,
				       XtNvisual,		t->vis,
				       XtNcolormap,		t->cm,
				       NULL);


      t->form = XtVaCreateManagedWidget("form",
					formWidgetClass,
					t->toplevel,
					NULL);
      
#ifdef Label
      Widget label = XtVaCreateManagedWidget("label",
					     labelWidgetClass,    form,
					     NULL);
#endif

      t->draw = XtVaCreateManagedWidget("draw",
					simpleWidgetClass,	t->form,
					XtNdepth,		t->xtk->depth,
					XtNvisual,		t->vis,
					XtNcolormap,		t->cm,
					XtNwidth,		t->xtk->width,
					XtNheight,		t->xtk->height,
					XtNbackground, 		0x00000000,
					NULL);

      XtAddEventHandler(t->draw, ExposureMask, 0, do_expose, (void*)t);
      XtAddEventHandler(t->draw, StructureNotifyMask, 0, (XtEventHandler)do_configure, (void*)t);
      XtRealizeWidget(t->toplevel);
      t->surface = cairo_xlib_surface_create(dpy, XtWindow(t->draw), t->vis, t->xtk->width, t->xtk->height);
      assert(cairo_surface_status(t->surface) == CAIRO_STATUS_SUCCESS);
    }
  XtAppMainLoop(appContext);
  return 1;
}

const char* id() { return "xaw"; }
