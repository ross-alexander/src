class DviStream {
 public:
  FILE *stream;
  DviStream(FILE*);
  unsigned int u8();
  unsigned int u16();
  unsigned int u24();
  unsigned int u32();
  signed int s8();
  signed int s16();
  signed int s24();
  signed int s32();
  signed int seek(signed long, int);
  signed long tell();
  unsigned char* String(int);
};

class DviState {
public:
  int h, v, w, x, y, z;
  DviState() {
    h = v = w = x = y = 0;
  };
};

class DviGlyph {
public:
  class DviFont *font;
  int flags;
  int cc, w, h, dx, dy, tfm;
  int x_offset, y_offset;
  Cairo::RefPtr<Cairo::ImageSurface> surface;
  DviGlyph()
    {
      flags = w = h = 0;
    };
};

class DviFont {
public:
  int id;
  unsigned int res;
  unsigned int checksum;
  unsigned int scaled_size;
  unsigned int design_size;
  unsigned char *name;
  int type;
  int debug;
  DviGlyph *glyph;
  int Load(int);
  int PkLoad(unsigned char*);
  DviFont(DviStream*);
};

class DviPage {
public:
  off_t offset;
  class DviFile *file;
  DviPage(class DviFile*, int);
};

struct DviFile {
  DviStream *stream;
  size_t size;
  int num;
  int den;
  int mag;
  double conv;
  double tfm_conv;
  int maxv;
  int maxh;
  int maxs;
  int maxp;
  unsigned char *comment;
  struct DviFont** fonts;
  struct DviPage **pages;
};

class DviCairo {
 public:
  Cairo::RefPtr<Cairo::ImageSurface> surface;
  Cairo::RefPtr<Cairo::Context> cr;
  int maxh, maxv;
  double dvi_conv, scale;
  DviCairo(int, int, double, Cairo::RefPtr<Cairo::Context>);
  void Place(int, int, DviGlyph *g);
  void Save(const char*);
};
