/* ----------------------------------------------------------------------
   --
   -- cairomask
   --
   -- 2026-09-11: Example cairomm program to create a random mask
   --
   ---------------------------------------------------------------------- */

#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

int main()
{
  Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(Cairo::Surface::Format::RGB24, 512, 512);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);


  // Fill image with background
  
  cr->set_source_rgb(1,1,0);
  cr->paint();

  // Generate random noise
  // The matrix should be 32 x 256 as A1 is a single bit so the stride for 256 bits is 32
  
  unsigned char data[256*256];
  for (int j = 0; j < 256; j++)
    for (int i = 0; i < 256; i++)
      data[j * 256 + i] = random() % 256;

  int stride = Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::A1, 256);
  Cairo::RefPtr<Cairo::ImageSurface> mask = Cairo::ImageSurface::create(data, Cairo::Surface::Format::A1, 256, 256, stride);

  // Paint source using alpha channel of a pattern as a transparency
  // filter
  
  cr->set_source_rgb(1,0,0);
  cr->translate(100, 100);
  cr->mask(mask, 0, 0);

  // Write result to a PNG file
  
  surface->write_to_png("dvi.png");
}
