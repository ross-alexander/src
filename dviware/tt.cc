#include <cairomm/cairomm.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

int main()
{
  Cairo::RefPtr<Cairo::ImageSurface> surface = Cairo::ImageSurface::create(Cairo::Surface::Format::RGB24, 512, 512);
  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);
  cr->set_source_rgb(1,1,0);
  cr->paint();

  unsigned char data[256*256];
  for (int j = 0; j < 256; j++)
    for (int i = 0; i < 256; i++)
      data[j * 256 + i] = random() % 256;

  Cairo::RefPtr<Cairo::ImageSurface> mask = Cairo::ImageSurface::create(data, Cairo::Surface::Format::A1, 256, 256,
									Cairo::ImageSurface::format_stride_for_width(Cairo::Surface::Format::A1, 256));
  
  cr->set_source_rgb(1,0,0);
  cr->translate(100, 100);
  cr->mask(mask, 0, 0);
  surface->write_to_png("dvi.png");
}
