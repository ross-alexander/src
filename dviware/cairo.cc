#include <assert.h>
#include <stdio.h>

#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>
#include <gtk/gtk.h>

#include "common.h"

DviCairo::DviCairo(int mh, int mv, double conv, Cairo::RefPtr<Cairo::Context> ref)
{
  scale = 1.0;
  maxh = mh * conv * scale;
  maxv = mv * conv * scale;
  dvi_conv = conv;

  //  maxh = 8.0 * 600;
  //  maxv = 11.5 * 600;

  if (ref)
    {
      cr = ref;
    }
  else
    {
      surface = Cairo::ImageSurface::create(Cairo::Surface::Format::RGB24, maxh, maxv);
      printf("Create surface %d × %d\n", maxh, maxv);
      cr = Cairo::Context::create(surface);
    }
  
  cr->set_source_rgb(1,1,1);
  cr->scale(scale, scale);
  cr->rectangle(0, 0, maxh, maxv);
  cr->fill();
  //  cr->translate(600, 600);
}

void DviCairo::Save(const char *filename)
{
  surface->write_to_png("dvi.png");
}


void DviCairo::Place(int h, int v, DviGlyph *g)
{
  cr->set_source_rgb(1.0,0,0);
  h *= dvi_conv;
  v *= dvi_conv;

  Cairo::RefPtr<Cairo::ImageSurface> s = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, g->w, g->h);
  Cairo::RefPtr<Cairo::Context> c = Cairo::Context::create(s);
  
  c->set_source_rgb(0.0, 0.0, 0.0);
  c->mask(g->surface, 0.0, 0.0);

  assert(g->font->name);

  // char *ff = g_strdup_printf("%s-%d.png", g->font->name, g->cc);
  //  s->write_to_png(ff);
  // free(ff);

  // printf("Place(%d, %d, %d, %d, %d)\n", g->cc, h, v, g->x_offset, g->y_offset);
 
  cr->save();
  cr->set_source(s, h - g->x_offset, v - g->y_offset);
  cr->paint();
  cr->restore();
}

