// typedef struct tile_surface_t tile_surface_t;

typedef struct {
  cairo_surface_t *surface;
  int width;
  int height;
} tile_surface_t;

typedef struct {
  cairo_t *cr;
} tile_context_t;

typedef struct {
  cairo_pattern_t *pattern;
} tile_pattern_t;

tile_surface_t* tile_surface_new(int, int);
tile_surface_t* tile_surface_new_from_file(const char*);
tile_surface_t* tile_surface_new_from_surface(cairo_surface_t *, int, int);
