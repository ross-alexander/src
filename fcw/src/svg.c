#include <string.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <fcw.h>

extern int SublistToCairo(Fcw *fcw, GPtrArray *sublist, int flags, cairo_t *cairo);

typedef struct fcw_svg_catalog_t {
  Fcw *fcw;
  int flags;
  xmlDocPtr doc;
  xmlNodePtr defs;
  xmlNodePtr svg;
  double max_width;
  double x;
  double y;
  double lineh;
  double linew;
  double height;
  double width;
} fcw_svg_catalog_t;

/* ----------------------------------------------------------------------

   SymbolToSvg

   ---------------------------------------------------------------------- */

int SymbolToSvg(char *key, Symbol *sym, fcw_svg_catalog_t *catalog)
{
  Matrix3f transform = Matrix3fIdentity();
  transform.v[1][1] = -1.0; // reflection
  transform.v[1][2] = sym->original->Hi.y + sym->original->Low.y; // translate

  xmlNodePtr clone = SublistToSvg(catalog->fcw, sym->sublist, transform, catalog->flags);

    // xmlCopyNode(sym->svg, 1);
  char *id = g_strdup(key);
  g_strdelimit(id, " ", '-');
  xmlSetProp(clone, (xmlChar*)"id", (xmlChar*)id);
  xmlAddChild(catalog->defs, clone);

  double h = sym->original->Hi.y - sym->original->Low.y;
  double w = sym->original->Hi.x - sym->original->Low.x;

  
  
  xmlNodePtr use = xmlNewNode(NULL, (xmlChar*)"use");
  char *link = g_strdup_printf("#%s", id);

  double x = catalog->x - sym->original->Low.x + (catalog->width - w)/2;
  double y = catalog->y - sym->original->Low.y + (catalog->height - h)/2;

  char *x_prop = g_strdup_printf("%4.2f", x);
  char *y_prop = g_strdup_printf("%4.2f", y);

  xmlSetProp(use, (xmlChar*)"xlink:href", (xmlChar*)link);
  xmlSetProp(use, (xmlChar*)"x", (xmlChar*)x_prop);
  xmlSetProp(use, (xmlChar*)"y", (xmlChar*)y_prop);

  free(x_prop);
  free(y_prop);
  free(link);
  free(id);

  //  double fontsize = 5;
  
  //  if (w < (fontsize * strlen(key)))
  //    w = fontsize * strlen(key);

  xmlNodePtr box = xmlNewNode(NULL, (xmlChar*)"path");
  char *d = g_strdup_printf("M%f %f L %f %f L %f %f L %f %f z",
			    catalog->x, catalog->y,
			    catalog->x + catalog->width, catalog->y,
			    catalog->x + catalog->width, catalog->y + catalog->height,
			    catalog->x, catalog->y + catalog->height);
  char *style = g_strdup_printf("stroke:red; fill:none; stroke-width:1pt;");
  xmlSetProp(box, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(box, (xmlChar*)"d", (xmlChar*)d);

  free(style);
  free(d);

  // Add text
  
  xmlNodePtr text = xmlNewNode(NULL, (xmlChar*)"text");
  x_prop = g_strdup_printf("%4.2f", catalog->x + 4.0);
  y_prop = g_strdup_printf("%4.2f", catalog->y + catalog->height - 2.0);
  xmlSetProp(text, (xmlChar*)"x", (xmlChar*)x_prop);
  xmlSetProp(text, (xmlChar*)"y", (xmlChar*)y_prop);
  xmlSetProp(text, (xmlChar*)"style", (xmlChar*)"font-size:4pt;font-family:Courier;");
  xmlAddChild(text, xmlNewText((xmlChar*)key));
  free(x_prop);
  free(y_prop);

  xmlAddChild(catalog->svg, box);
  xmlAddChild(catalog->svg, text);
  xmlAddChild(catalog->svg, use);

  printf("Converting symbol %s [%f × %f @ %f × %f] to svg...", key, h, w, catalog->x, catalog->y);

  // Get maximum width
  
  if (catalog->x > catalog->linew)
    catalog->linew = catalog->x;
  
  catalog->x += catalog->width;
  if (catalog->x > catalog->max_width)
    {
      catalog->y += catalog->height;
      catalog->x = 0.0;
    }
  printf("\n");
  return 0;
}

/* ----------------------------------------------------------------------
   --
   -- fcw_svg_surface_sublist
   --
   ---------------------------------------------------------------------- */

int fcw_svg_surface_sublist(Fcw *fcw, const char *key, Symbol *sym, cairo_surface_t *surface, int flags, double w, double h, double scale, double border)
{
  printf("%s: %f × %f\n", key, w, h);
  cairo_t *cairo = cairo_create(surface);
  //  cairo_rectangle(cairo, 0, 0, w, h);
  //  cairo_set_source_rgb(cairo, 1, 0, 0);
  //  cairo_fill(cairo);
  cairo_save(cairo);
  char *dest = g_strdup_printf("name='%s'", key);
  cairo_tag_begin(cairo, CAIRO_TAG_DEST, dest);
  free(dest);
  cairo_scale(cairo, scale, -scale);
  cairo_translate(cairo, - sym->original->Low.x + border, - sym->original->Hi.y - border);
  SublistToCairo(fcw, sym->sublist, flags, cairo);
  cairo_tag_end(cairo, CAIRO_TAG_DEST);
  cairo_restore(cairo);
  cairo_destroy(cairo);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- SymTabToSvg
--
---------------------------------------------------------------------- */
void SymTabToSvg(Fcw *fcw)
{
  struct fcw_svg_catalog_t catalog;
  catalog.doc = xmlNewDoc((xmlChar*)"1.0");
  catalog.svg = xmlNewNode(NULL, (xmlChar*)"svg");
  catalog.defs = xmlNewNode(NULL, (xmlChar*)"defs");
  catalog.fcw = fcw;
  catalog.flags = 0;
  catalog.width = catalog.height = catalog.lineh = catalog.linew = 0.0;

  xmlDocSetRootElement(catalog.doc, catalog.svg);
  xmlAddChild(catalog.svg, catalog.defs);

  double w, h = 0.0;
  catalog.max_width = 1000;

  for (GTreeNode *node = g_tree_node_first(fcw->symbolTab); node != 0; node = g_tree_node_next(node))
    {
      Symbol *sym = (Symbol*)g_tree_node_value(node);
      SYMDEF* esym = sym->original;
      double scale = 1.0;
      double border = 0.0;
      double sym_w = (esym->Hi.x - esym->Low.x + 2*border) * scale;
      double sym_h = (esym->Hi.y - esym->Low.y + 2*border) * scale;
      w = w > sym_w ? w : sym_w;
      h = h > sym_h ? h : sym_h;
    }

  printf("Symbol table to SVG [%d symbols, %f × %f]\n", g_tree_nnodes(fcw->symbolTab), w, h);
  

  catalog.x = catalog.y = 0.0;
  catalog.width = w + 4.0;
  catalog.height = h + 12.0;
  
  g_tree_foreach(fcw->symbolTab, (GTraverseFunc)SymbolToSvg, &catalog);

  xmlSetProp(catalog.svg, (xmlChar*)"version", (xmlChar*)"1.1");
  xmlSetProp(catalog.svg, (xmlChar*)"xmlns", (xmlChar*)"http://www.w3.org/2000/svg");
  xmlSetProp(catalog.svg, (xmlChar*)"xmlns:xlink", (xmlChar*)"http://www.w3.org/1999/xlink");

  printf("width=%f height=%f linew=%f lineh=%f\n", catalog.width, catalog.height, catalog.linew, catalog.lineh);
  
  char *width = g_strdup_printf("%6.2f", catalog.linew);
  char *height = g_strdup_printf("%6.2f", catalog.y + catalog.height);

  xmlSetProp(catalog.svg, (xmlChar*)"width", (xmlChar*)width);
  xmlSetProp(catalog.svg, (xmlChar*)"height", (xmlChar*)height);

  free(width);
  free(height);

  char *fname = g_strdup_printf("%s.svg", fcw->file);
  xmlSaveFormatFile(fname, catalog.doc, 1);
  free(fname);
  xmlFreeDoc(catalog.doc);

  /* --------------------
     Create individual symtol output using cairo
     -------------------- */

  return;
  
  for (GTreeNode *node = g_tree_node_first(fcw->symbolTab); node != 0; node = g_tree_node_next(node))
    {
      const char *key = g_tree_node_key(node);
      Symbol *sym = (Symbol*)g_tree_node_value(node);
      SYMDEF* esym = sym->original;
      double scale = 10.0;
      double border = 1.0;
      double w = (esym->Hi.x - esym->Low.x + 2*border) * scale;
      double h = (esym->Hi.y - esym->Low.y + 2*border) * scale;
      cairo_surface_t *surface;

      printf(".. %s [%f × %f]\n", key, w, h);
      char *svg_path = g_strdup_printf("%s.svg", key);
      surface = cairo_svg_surface_create(svg_path, w, h);
      fcw_svg_surface_sublist(fcw, key, sym, surface, catalog.flags, w, h, scale, border);
      cairo_surface_destroy(surface);
      free(svg_path);      
      
      char *pdf_path = g_strdup_printf("%s.pdf", key);
      surface = cairo_pdf_surface_create(pdf_path, w, h);
      fcw_svg_surface_sublist(fcw, key, sym, surface, catalog.flags, w, h, scale, border);
      cairo_surface_destroy(surface);
      free(pdf_path);      
    }
}
