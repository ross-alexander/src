/* ----------------------------------------------------------------------
--
-- struct xtk_t
--
----------------------------------------------------------------------*/

typedef struct xtk_t {
  char *title;
  int width;
  int height;
  int depth;
  int visual;
  cairo_surface_t *source;
} xtk_t;

typedef struct xtk_info_t {
  char *id;
} xtk_info_t;

extern void xtk_draw_cairo(struct xtk_t*, cairo_t*);
extern void xtk_draw_surface(struct xtk_t*, cairo_surface_t*);
