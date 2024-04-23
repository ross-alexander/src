#include <assert.h>
#include <cairo.h>
#include <cairo-svg.h>
#include <glib.h>
#include <math.h>
#include <libxml/tree.h>

#include "geometry.h"
#include "fcw.h"
#include "fcw-cairo.h"

extern int DefaultPalette[256];

int SymrefToCairo(Fcw *fcw, Entity *e, SYMREF *sref, int flags, cairo_t *cr)
{
  printf("Symref(%s)\n", sref->SName);

  printf("%f %f %f %f\n", sref->TMat.m11,sref->TMat.m21,sref->TMat.m31,sref->TMat.m41);
  printf("%f %f %f %f\n", sref->TMat.m12,sref->TMat.m22,sref->TMat.m32,sref->TMat.m42);
  printf("%f %f %f %f\n", sref->TMat.m13,sref->TMat.m23,sref->TMat.m33,sref->TMat.m43);

  Symbol *sym = (Symbol*)g_tree_lookup(fcw->symbolTab, sref->SName);
  assert(sym);

  cairo_save(cr);
  cairo_translate(cr, sref->TMat.m41, sref->TMat.m42);
  cairo_scale(cr, sref->TMat.m11, sref->TMat.m22);

  SublistToCairo(fcw, sym->sublist, flags, cr);
  cairo_restore(cr);
  return 1;
}


int LineToCairo(Fcw *fcw, Line2 *g, int flags, cairo_t *cr)
{
  cairo_save(cr);
  cairo_move_to(cr, g->Line.p1.x, g->Line.p1.y);
  cairo_line_to(cr, g->Line.p2.x, g->Line.p2.y);
  
  if (g->LWidth > 0.0)
    {
      cairo_set_line_width(cr, g->LWidth);
      cairo_set_source_rgb(cr,
			   (double)((DefaultPalette[g->EColor]>>16)%256)/256,
			   (double)((DefaultPalette[g->EColor]>>8)%256)/256,
			   (double)((DefaultPalette[g->EColor]>>0)%256)/256);
      cairo_stroke(cr);
    }
  cairo_restore(cr);
  
  return 1;
}

int CircleToCairo(Fcw* fcw, Cir2 *cir, int flags, cairo_t *cr)
{
  double x = cir->Circle.Center.x;
  double y = cir->Circle.Center.y;
  double r = cir->Circle.Radius;

  cairo_new_path(cr);
  cairo_arc(cr, x, y, r, 0, 2*M_PI);
  cairo_close_path(cr);

  if (cir->EFStyle != 0)
    {
      cairo_set_source_rgb(cr,
			   (double)((DefaultPalette[cir->EColor2]>>16)%256)/256,
			   (double)((DefaultPalette[cir->EColor2]>>8)%256)/256,
			   (double)((DefaultPalette[cir->EColor2]>>0)%256)/256);
      cairo_fill_preserve(cr);
    }
  if (cir->LWidth > 0.0)
    {
      cairo_set_line_width(cr, cir->LWidth);
      cairo_set_source_rgb(cr,
			   (double)((DefaultPalette[cir->EColor]>>16)%256)/256,
			   (double)((DefaultPalette[cir->EColor]>>8)%256)/256,
			   (double)((DefaultPalette[cir->EColor]>>0)%256)/256);
      cairo_stroke(cr);
    }

  return 1;
}

int PathToCairo(Fcw* fcw, Path2 *path, int flags, cairo_t *cr, int poly)
{
  int nodes = path->Count;
  int closed = path->Flags & NL_CLS;
  GPOINT2 *n = path->Nodes;

  if (path->SmType == 1)
    {
      printf("Found B-spline %f %f %d %d\n", (double)path->SParm, (double)path->EParm, (int)path->SRes, (int)nodes);
    }

  if (!poly)
    cairo_new_path(cr);

  if (path->SmType == 0)
    {
      cairo_move_to(cr, n[0].x, n[0].y);
      for (int k = 1; k < nodes; k++)
	cairo_line_to(cr, n[k].x, n[k].y);
    }
  if (path->SmType == 1)
    {
      int nn;
      int first = 1;

      GPOINT2* q;
      if (closed)
	{
	  printf("Closed cubic b-spline with %d points.\n", nodes);
	  nn = nodes + 3;
	  q = (GPOINT2*)calloc(sizeof(GPOINT2), nn);
	  for (int k = 0; k < nn; k++)
	    {
	    q[k].x = n[k%nodes].x;
	    q[k].y = n[k%nodes].y;
	    }
	}
      else
	{
	  printf("Open cubic b-spline with %d points.\n", nodes);
	  nn = nodes + 4;
	  q = (GPOINT2*)calloc(sizeof(GPOINT2), nn);
	  for (int k = 0; k < 2; k++)
	    {
	      q[k].x = n[0].x;
	      q[k].y = n[0].y;
	      q[nodes+k+2].x = n[nodes-1].x;
	      q[nodes+k+2].y = n[nodes-1].y;
	    }
	  for (int k = 0; k < nodes; k++)
	    {
	      q[k+2].x = n[k].x;
	      q[k+2].y = n[k].y;
	    }
	}
      for (int k = 0; k < nn-3; k++)
	{
	  GPOINT2 Q[4];
	  int ofs[4];
	  ofs[0] = k+0;
	  ofs[1] = k+1;
	  ofs[2] = k+2;
	  ofs[3] = k+3;
	  
	  Q[0].x = 1.0/6.0 * (q[ofs[0]].x + 4 * q[ofs[1]].x + q[ofs[2]].x);
	  Q[0].y = 1.0/6.0 * (q[ofs[0]].y + 4 * q[ofs[1]].y + q[ofs[2]].y); 
	  Q[1].x = 1.0/6.0 * (4 * q[ofs[1]].x + 2 * q[ofs[2]].x);
	  Q[1].y = 1.0/6.0 * (4 * q[ofs[1]].y + 2 * q[ofs[2]].y);
	  Q[2].x = 1.0/6.0 * (2 * q[ofs[1]].x + 4 * q[ofs[2]].x);
	  Q[2].y = 1.0/6.0 * (2 * q[ofs[1]].y + 4 * q[ofs[2]].y);
	  Q[3].x = 1.0/6.0 * (q[ofs[1]].x + 4 * q[ofs[2]].x + q[ofs[3]].x);
	  Q[3].y = 1.0/6.0 * (q[ofs[1]].y + 4 * q[ofs[2]].y + q[ofs[3]].y); 
	  if (first)
	    {
	      cairo_move_to(cr, Q[0].x, Q[0].y);
	      first = 0;
	    }
	  cairo_curve_to(cr, Q[1].x, Q[1].y, Q[2].x, Q[2].y,Q[3].x, Q[3].y);
	}
    }

  if (poly)
    return 0;
  
  if (path->Flags & NL_CLS)
    {
      cairo_close_path(cr);
      if (path->EFStyle != 0)
	{
	  cairo_set_source_rgb(cr,
			       (double)((DefaultPalette[path->EColor2]>>16)%256)/256,
			       (double)((DefaultPalette[path->EColor2]>>8)%256)/256,
			       (double)((DefaultPalette[path->EColor2]>>0)%256)/256);
	  cairo_fill_preserve(cr);
	}
    }
  if (path->LWidth > 0.0)
    {
      cairo_set_line_width(cr, path->LWidth);
      cairo_set_source_rgb(cr,
			   (double)((DefaultPalette[path->EColor]>>16)%256)/256,
			   (double)((DefaultPalette[path->EColor]>>8)%256)/256,
			   (double)((DefaultPalette[path->EColor]>>0)%256)/256);
      cairo_stroke(cr);
    }
  return 0;
}

/* ----------------------------------------------------------------------

   PolyPathToCairo

   ---------------------------------------------------------------------- */

int PolypathToCairo(Fcw *fcw, Entity *e, int flags, cairo_t *cr)
{
  if (!e->sublist || (e->sublist->len == 0))
    return 0;
  Common *common = (Common*)e->ptr;

  cairo_new_path(cr);
  
  for (int i = 0; i < e->sublist->len; i++)
    {
      Entity *subentity = (Entity*)g_ptr_array_index(e->sublist, i);
      if (subentity->etype != 3)
	{
	  fprintf(stderr, "Polypath with non path sub entity.\n");
	  exit(1);
	}
      PathToCairo(fcw, (Path2*)(subentity->ptr), flags, cr, 1);
    }

  cairo_close_path(cr);
  
  if (common->EFStyle != 0)
    {
      cairo_set_source_rgb(cr,
			   (double)((DefaultPalette[common->EColor2]>>16)%256)/256,
			   (double)((DefaultPalette[common->EColor2]>>8)%256)/256,
			   (double)((DefaultPalette[common->EColor2]>>0)%256)/256);
      cairo_fill_preserve(cr);
    }
  
  if (common->LWidth > 0.0)
    {
      cairo_set_line_width(cr, common->LWidth);
      cairo_set_source_rgb(cr,
			   (double)((DefaultPalette[common->EColor]>>16)%256)/256,
			   (double)((DefaultPalette[common->EColor]>>8)%256)/256,
			   (double)((DefaultPalette[common->EColor]>>0)%256)/256);
      cairo_stroke(cr);
    }
  return 0;
}


/* ----------------------------------------------------------------------
   --
   -- EntityToCairo
   --
   ---------------------------------------------------------------------- */

int EntityToCairo(Fcw *fcw, Entity *e, int flags, cairo_t *cr)
{
  Common *common = (Common*)e->ptr;
  int res = 0;

  printf("%hhu Color %hhu %hhu Style %hhu %hhu Thick %hhu Width %f\n",
	 common->EType,
	 common->EColor, common->EColor2, 
	 common->ELStyle, common->EFStyle, common->EThick, common->LWidth);
  
  switch(common->EType)
    {
    case 2: /* Line */
      {
	res = LineToCairo(fcw, (Line2*)common, flags, cr);
	break;
      }
    case 3:
      res = PathToCairo(fcw, (Path2*)common, flags, cr, 0);
      break;
    case 4:
      {
	XPENT *xp = (XPENT*)common;
	printf("Custom entity length %d\n", xp->ERLen);
	break;
      }
    case 6: /* Circle */
      res = CircleToCairo(fcw, (Cir2*)common, flags, cr);
      break;

    case 17:
      res = PolypathToCairo(fcw, e, flags, cr);
      break;
      
    case 28: /* Symbol definition */
      break;
    case 29:
      res = SymrefToCairo(fcw, e, (SYMREF*)common, flags, cr);
      break;
    case 41:
      {
	if (e->sublist)
	  {
	    cairo_save(cr);
	    SublistToCairo(fcw, e->sublist, flags, cr);
	    cairo_restore(cr);
	  }
	break;
      }
    default:
      printf("Entity type %d\n", common->EType);
      break;
    }
  return res;
}

/* ----------------------------------------------------------------------

   SublistToCairo

   ---------------------------------------------------------------------- */

int SublistToCairo(Fcw *fcw, GPtrArray *sublist, int flags, cairo_t *cairo)
{
  printf("Cairo Sublist [%d]\n", sublist->len);

  for (int i = 0; i < sublist->len; i++)
    EntityToCairo(fcw, (Entity*)g_ptr_array_index(sublist, i), flags, cairo);
  return 0;
}

/* ----------------------------------------------------------------------

   FcwToCairo

   ---------------------------------------------------------------------- */

int FcwToCairo(Fcw *fcw)
{
  HDR *hdr = (HDR*)fcw->iblock[0];
  double w = hdr->Hi.x - hdr->Low.x;
  double h = hdr->Hi.y - hdr->Low.y;
  int flags = 0;
  fcw->width = w;
  fcw->height = h;
  fcw->flags = flags;

  printf("Creating cairo surface %f x %f\n", w, h);

  cairo_surface_t *surface = cairo_svg_surface_create("cairo.svg", w, h);
  cairo_t *cairo = cairo_create(surface);
  cairo_scale(cairo, 1.0, -1.0);
  cairo_translate(cairo, -hdr->Low.x, -hdr->Hi.y);
  SublistToCairo(fcw, fcw->root->sublist, flags, cairo);
  cairo_destroy(cairo);
  cairo_surface_destroy(surface);
  return 0;
}
