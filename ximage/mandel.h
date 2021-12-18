typedef struct rgb3i {
  guint8 red;
  guint8 green;
  guint8 blue;
} rgb3i;

typedef struct rgb3d {
  double red, green, blue;
} rgb3d;

typedef struct {
  int depth;
  int width;
  int height;
  int palette_size;
  rgb3i *palette;
  guint8 *data;
} mandel_image;

mandel_image * mandel_image_create_default(int, int, int);
mandel_image * mandel_image_create(int, double, double, double, double, int, int);
void mandel_image_create_hsv_palette(mandel_image*);
void mandel_image_destroy(mandel_image*);
void mandel_image_8to24(mandel_image*);
