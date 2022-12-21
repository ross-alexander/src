#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>

#include <cairo/cairo-xlib.h>

#include "common.h"
#include "visual.h"

#define N_TOP_BORDER        10
#define N_LEFT_BORDER       10
#define N_RIGHT_BORDER      10
#define N_BOTTOM_BORDER     10

#define N_LABEL_HEIGHT      15
#define N_LABEL_LEFT_MARGIN  5

/* ----------------------------------------------------------------------
--
-- motif_callback
--
---------------------------------------------------------------------- */

void motif_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  Dimension new_width,new_height;
  Display *dpy;
  Window win;
  XmDrawingAreaCallbackStruct *info = (XmDrawingAreaCallbackStruct*)call_data;
  XEvent *event = info->event;
  XExposeEvent *expose;
  //  XResizeRequestEvent *resize;

  struct Xtk *xtk = (struct Xtk*)client_data;

  dpy = XtDisplay(w);
  win = XtWindow(w);

  switch(info->reason)
    {
    case XmCR_RESIZE:
      XtVaGetValues(w,
		    XmNwidth, &new_width,
		    XmNheight, &new_height,
		    NULL);
      xtk->width = new_width;
      xtk->height = new_height;
      printf("Resize %d %d\n", xtk->width, xtk->height);
      cairo_xlib_surface_set_size(xtk->surface, xtk->width, xtk->height);

      break;

    case XmCR_EXPOSE:
      expose = (XExposeEvent*)event;
      printf("Expose %d %d\n", xtk->width, xtk->height);
      cairo_draw_surface(xtk);
      break;
    }
}

/* ----------------------------------------------------------------------
--
-- do_motif
--
---------------------------------------------------------------------- */

int do_xtk(int argc, char *argv[], unsigned nwin, struct Xtk **xtk)
{
  XtToolkitInitialize();
  XtAppContext appContext = XtCreateApplicationContext();
  Display *dpy = XtOpenDisplay(appContext, NULL, NULL, "Tcairo", NULL, 0, &argc, argv);

  int screenid = XDefaultScreen(dpy);
  Drawable root = XRootWindow(dpy, screenid);



  //  XtAppContext appContext;
  //  Widget toplevel = XtVaAppInitialize(&appContext, "toplevel", NULL, 0, &argc, argv, NULL, NULL);

  /* Create a MainWindow to contain the drawing area */

  for (int i = 0; i < nwin; i++)
    {
      struct Xtk *t = xtk[i];
      printf("Creating Motif window.\n");
      Visual *visual = get_visual(dpy, t);
      Colormap cm = XCreateColormap(dpy, root, visual, AllocNone);
      
      Widget toplevel = XtVaAppCreateShell("tcairo", "Tcairo", applicationShellWidgetClass, dpy, 
					   XtNdepth,		t->depth,
					   XtNvisual,		visual,
					   XtNcolormap,		cm,
					   NULL);
      
      Widget mainWin = XtVaCreateManagedWidget ("mainWin",
						xmFormWidgetClass,   toplevel, 
						NULL);
      
      Widget label = XtVaCreateManagedWidget("label",
					     xmLabelWidgetClass, mainWin,
					     XmNbackground,       0xFFFF0000,
					     XmNalignment,        XmALIGNMENT_BEGINNING,
					     XmNbottomAttachment, XmATTACH_FORM,
					     XmNleftAttachment,   XmATTACH_FORM,
					     XmNrightAttachment,  XmATTACH_FORM,
					     XmNmarginLeft,       N_LABEL_LEFT_MARGIN,
					     XmNleftOffset,       N_LEFT_BORDER,
					     XmNrightOffset,      N_RIGHT_BORDER,
					     XmNbottomOffset,     N_BOTTOM_BORDER,
					     NULL);
      
      Widget framed = XtVaCreateManagedWidget("framed",
					      xmFrameWidgetClass, mainWin,
					      XmNtopOffset,        N_TOP_BORDER,
					      XmNleftOffset,       N_LEFT_BORDER,
					      XmNrightOffset,      N_RIGHT_BORDER,
					      XmNbottomOffset,     N_BOTTOM_BORDER,
					      XmNtopAttachment,    XmATTACH_FORM,
 					      XmNleftAttachment,   XmATTACH_FORM,
					      XmNrightAttachment,  XmATTACH_FORM,
					      XmNbottomAttachment, XmATTACH_WIDGET,
					      XmNbottomWidget,     label,
					      NULL);
      
      Widget draw = XtVaCreateManagedWidget ("draw",
					     xmDrawingAreaWidgetClass, framed,
					     XmNbackground,       0x00000000,
					     XmNresizePolicy,     XmRESIZE_ANY,
					     XmNtopAttachment,    XmATTACH_FORM,
					     XmNtopOffset,        N_TOP_BORDER,
					     XmNleftAttachment,   XmATTACH_FORM,
					     XmNleftOffset,       N_LEFT_BORDER,
					     XmNrightAttachment,  XmATTACH_FORM,
					     XmNrightOffset,      N_RIGHT_BORDER,
					     XmNbottomAttachment, XmATTACH_WIDGET,
					     XmNbottomOffset,     N_BOTTOM_BORDER,
					     XmNwidth,            t->width,
					     XmNheight,           t->height,
					     NULL);

      XtAddCallback(draw, XmNexposeCallback, (XtCallbackProc)motif_callback, (void*)t);
      XtAddCallback(draw, XmNresizeCallback, (XtCallbackProc)motif_callback, (void*)t);
      XtRealizeWidget(toplevel);
      t->surface = cairo_xlib_surface_create(dpy, XtWindow(draw), visual, t->width, t->height);
    }
  XtAppMainLoop(appContext);
  return 1;
}

const char *id() { return "xm"; }
