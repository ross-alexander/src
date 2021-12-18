#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/PanedW.h>

#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Frame.h>

#include <cairo.h>
#include <cairo-xlib.h>

GC gc;
Pixmap pixmap;
Dimension width, height;  /* CUrrent size of the DrawingArea */

String colors[] = {
   "Black", "Red", "Green", "Blue", "White", "Navy", "Orange", "Yellow",
   "Pink", "Magenta", "Cyan", "Brown", "Grey", "LimeGreen", "Turquoise",
   "Violet", "Wheat", "Purple"
   };

typedef struct {
  int width, height;
  cairo_surface_t *surface;
  //  Pixmap pixmap;
} FractSpecs;

#define TABLESIZE 128	/* size of colortable */

#define N_TOP_BORDER        10
#define N_LEFT_BORDER       10
#define N_RIGHT_BORDER      10
#define N_BOTTOM_BORDER     10

#define N_MAIN_WIDTH       500
#define N_MAIN_HEIGHT      600

#define N_LABEL_HEIGHT      15
#define N_LABEL_LEFT_MARGIN  5

#define N_MID_SPACE         10

typedef struct {
  unsigned char R, B, G;
} ColorRGB;

ColorRGB HSL2RGB(double h, double sl, double l)
{
  double v;
  double r,g,b;

  r = l;   // default to gray
  g = l;
  b = l;
  v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
  
  if (v > 0)
    {
      double m;
      double sv;
      int sextant;
      double fract, vsf, mid1, mid2;

      m = l + l - v;
      sv = (v - m ) / v;
      h *= 6.0;
      sextant = (int)h;
      fract = h - sextant;
      vsf = v * sv * fract;
      mid1 = m + vsf;
      mid2 = v - vsf;

      switch (sextant)
	{
	case 0:
	  r = v;
	  g = mid1;
	  b = m;
	  break;

	case 1:
	  r = mid2;
	  g = v;
	  b = m;
	  break;

	case 2:
	  r = m;
	  g = v;
	  b = mid1;
	  break;
	  
	case 3:
	  r = m;
	  g = mid2;	 
	  b = v;
	  break;
	case 4:
	  r = mid1;
	  g = m;	  
	  b = v;	  
	  break;
	case 5:
	  r = v;
	  g = m;	  
	  b = mid2;	  
	  break;	  
	}
    }
  ColorRGB rgb;
  rgb.R = (unsigned char)(r * 255.0f);
  rgb.G = (unsigned char)(g * 255.0f);
  rgb.B = (unsigned char)(b * 255.0f);
  return rgb;
}

/* ---------------------------------------------------------------------- */

void SetColor(Widget widget, XtPointer client_data)
{
  String color = (String)client_data;
  Display *dpy = XtDisplay(widget);
  Colormap cmap = DefaultColormapOfScreen(XtScreen(widget));
  XColor col, unused;

  if (!XAllocNamedColor(dpy, cmap, color, &col, &unused)) {
    char buf[32];
    sprintf(buf, "Can't alloc %s", color);
    XtWarning(buf);
    return;
  }
  XSetForeground (dpy, gc, col.pixel);
}

/* ---------------------------------------------------------------------- */

int MandelColor(int iters, double a, double b)
{	
  register int count;
  double t, tx, ty, tx2, ty2;

  count = 0;
  tx = a;
  ty = b;
  
  /* general Mandelbrot algorithm ... */
  while ((((tx2 = tx*tx) + (ty2 = ty*ty)) < 4.0) && (count <= iters))
    {
      t = tx2 - ty2 + a;
      ty = 2*tx*ty + b;
      tx = t;
      count++;
    }
  return (count > iters) ? 0 : count;
}

unsigned char* Mandel(int iters, double xmin, double xmax, double ymin, double ymax, int width, int height)
{
  unsigned char* map = (unsigned char*)malloc(width * height * sizeof(unsigned char*));
  int c, xplot, yplot;
  double x, y;
  double xscale = (xmax - xmin) / width;
  double yscale = (ymax - ymin) / height;

  iters--;
  y = ymin;
  yplot = 0;
  while (y <= ymax)
    {
      x = xmin;
      xplot = 0;
      while (x <= xmax)
	{
	  c = MandelColor(iters, x,y);
	  map[(height - yplot) * width + xplot] = c;
	  x += xscale;
	  xplot++;
	}
      y += yscale;
      yplot++;
    }
  return map;
}

/* ---------------------------------------------------------------------- */

void MandelCallback(Widget w, caddr_t call_data, caddr_t client_data)
{
  Dimension new_width,new_height;
  int width, height;
  int iters = 10;
  XImage *image;
  Display *dpy;
  int screen;
  Window win;
  Pixmap PicturePixmap;
  XmDrawingAreaCallbackStruct *info = (XmDrawingAreaCallbackStruct*)client_data;
  XEvent *event = info->event;
  XExposeEvent *expose;
  XResizeRequestEvent *resize;
  FractSpecs *specs = (FractSpecs*)call_data;

  dpy = XtDisplay(w);
  win = XtWindow(w);
  screen = XScreenNumberOfScreen(XtScreen(w));

  int count;
  int mask = VisualIDMask;
  XVisualInfo vinfo;
  vinfo.visualid = DefaultVisual(dpy, screen)->visualid;
  XVisualInfo *vinfos = XGetVisualInfo(dpy, mask, &vinfo, &count);

  XtVaGetValues(w,
		XmNwidth, &new_width,
		XmNheight, &new_height,
		NULL);
  width = new_width;
  height = new_height;

  switch(info->reason)
    {
    case XmCR_RESIZE:
      printf("Resize %d %d\n", width, height);
      if (specs->surface)
	{
	  unsigned char *d = cairo_image_surface_get_data(specs->surface);
	  free(d);
	  cairo_surface_destroy(specs->surface);
	}

      specs->width = width;
      specs->height = height;
      unsigned char *data = Mandel(256, -1.0, 1.0, -1.0, 1.0, width, height);
      unsigned int stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, width);
      unsigned char *data24 = calloc(stride, height);
      specs->surface = cairo_image_surface_create_for_data(data24, CAIRO_FORMAT_RGB24, width, height, stride);

      for (int l = 0; l < height; l++)
	for (int m = 0; m < width; m++)
	  {
	    unsigned char c = data[width * l + m];
	    unsigned int offset = stride * l + 4 * m;
	    ColorRGB col = HSL2RGB(c / 256.0, 0.5, 0.5);
	    data24[offset + 0] = col.R;
	    data24[offset + 1] = col.G;
	    data24[offset + 2] = col.B;
	    data24[offset + 3] = 0;
	  }
      free(data);
      break;

    case XmCR_EXPOSE:
      printf("Expose\n");
      expose = (XExposeEvent*)event;
      XtVaGetValues(w,
		    XmNwidth, &new_width,
		    XmNheight, &new_height,
		    NULL);
      width = new_width;
      height = new_height;
      XClearArea(dpy, win, 0, 0, width, height, 0);
      cairo_surface_t* surface = cairo_xlib_surface_create(dpy, win, vinfos[0].visual, width, height);
      cairo_t *cr = cairo_create(surface);
      cairo_set_source_rgb(cr, 0, 0, 1);
      cairo_new_path(cr);
      cairo_rectangle(cr, 0, 0, width, height);
      cairo_fill(cr);
      
      cairo_set_source_surface(cr, specs->surface, 0.0, 0.0);
      cairo_new_path(cr);
      cairo_rectangle(cr, 0, 0, width, height);
      cairo_save(cr);
      cairo_clip(cr);
      cairo_paint(cr);
      cairo_restore(cr);
      cairo_destroy(cr);
      cairo_surface_destroy(surface);
      break;
    }
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  Widget toplevel, mainWin, framed, label, fractal, form;
  XtAppContext appContext;
  String translations = "<Configure>:  configure()";
  XGCValues gcv;
  FractSpecs specs;
  XVisualInfo vinfo;

  specs.height = specs.width = 0;
  specs.surface = NULL;

  toplevel = XtVaAppInitialize(&appContext, "toplevel", NULL, 0, 
			       &argc, argv, NULL, NULL);

  /* Create a MainWindow to contain the drawing area */
  mainWin = XtVaCreateManagedWidget ("mainWin",
				     xmFormWidgetClass, toplevel, 
				     XmNwidth,            1000, // N_MAIN_WIDTH,
				     XmNheight,           1000, //N_MAIN_HEIGHT,
				     NULL);
  
  label = XtVaCreateManagedWidget("label",
				  xmLabelWidgetClass, mainWin,
				  XmNbackground,       WhitePixelOfScreen(XtScreen (mainWin)),
				  XmNwidth,            N_MAIN_WIDTH,
				  XmNheight,           N_LABEL_HEIGHT,
				  XmNalignment,        XmALIGNMENT_BEGINNING,
				  XmNbottomAttachment, XmATTACH_FORM,
				  XmNleftAttachment,   XmATTACH_FORM,
				  XmNrightAttachment,  XmATTACH_FORM,
				  XmNmarginLeft,       N_LABEL_LEFT_MARGIN,
				  XmNleftOffset,       N_LEFT_BORDER,
				  XmNrightOffset,      N_RIGHT_BORDER,
				  XmNbottomOffset,     N_BOTTOM_BORDER,
				  XmNbackground,       100,
				  NULL);
  framed = XtVaCreateManagedWidget("framed",
				   xmFrameWidgetClass, mainWin,
				   XmNtopAttachment,    XmATTACH_FORM,
				   XmNtopOffset,        N_TOP_BORDER,
				   XmNleftAttachment,   XmATTACH_FORM,
				   XmNleftOffset,       N_LEFT_BORDER,
				   XmNrightAttachment,  XmATTACH_FORM,
				   XmNrightOffset,      N_RIGHT_BORDER,
				   XmNbottomAttachment, XmATTACH_WIDGET,
				   XmNbottomOffset,     N_BOTTOM_BORDER,
				   XmNbottomWidget,     label,
				   NULL);
  fractal = XtVaCreateManagedWidget ("fractal",
				     xmDrawingAreaWidgetClass, framed,
				     XmNbackground,       WhitePixelOfScreen (XtScreen (mainWin)),
				     XmNresizePolicy,     XmRESIZE_ANY,
				     XmNtopAttachment,    XmATTACH_FORM,
				     XmNtopOffset,        N_TOP_BORDER,
				     XmNleftAttachment,   XmATTACH_FORM,
				     XmNleftOffset,       N_LEFT_BORDER,
				     XmNrightAttachment,  XmATTACH_FORM,
				     XmNrightOffset,      N_RIGHT_BORDER,
				     XmNbottomAttachment, XmATTACH_WIDGET,
				     XmNbottomOffset,     N_BOTTOM_BORDER,
				     NULL);
  //  XtAddCallback(fractal, XmNexposeCallback, (XtCallbackProc)MandelCallback, (void*)&specs);
  //  XtAddCallback(fractal, XmNresizeCallback, (XtCallbackProc)MandelCallback, (void*)&specs);

  int count;
  int mask = 0;
  Screen *s = XtScreen(mainWin);
  Display *dpy = XtDisplay(mainWin);

  vinfo.screen = DefaultScreen(dpy);
  XVisualInfo *vinfos = XGetVisualInfo(dpy, mask, &vinfo, &count);
  Visual *v = DefaultVisual(dpy, DefaultScreen(dpy));
  if (count == 0)
    {
      fprintf(stderr, "No matching visual found.\n");
      exit(1);
    }

  gcv.foreground = WhitePixelOfScreen(XtScreen(mainWin));
  gc = XCreateGC(XtDisplay(mainWin), RootWindowOfScreen(XtScreen(mainWin)), GCForeground, &gcv);
  /*  XtAppAddActions(appContext, actions, XtNumber(actions)); */
  XtRealizeWidget(toplevel);
  XtAppMainLoop(appContext);
}
