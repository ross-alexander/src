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
  double lineh;
  double linew;
  double height;
  double width;
} fcw_svg_catalog_t;


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

  double h = sym->original->Hi.y - sym->original->Low.y + 12.0;
  double w = sym->original->Hi.x - sym->original->Low.x + 4.0;
  
  xmlNodePtr use = xmlNewNode(NULL, (xmlChar*)"use");
  char *link = g_strdup_printf("#%s", id);
  char *x = g_strdup_printf("%4.2f", catalog->width - sym->original->Low.x);
  char *y = g_strdup_printf("%4.2f", catalog->height - sym->original->Low.y);

  xmlSetProp(use, (xmlChar*)"xlink:href", (xmlChar*)link);
  xmlSetProp(use, (xmlChar*)"x", (xmlChar*)x);
  xmlSetProp(use, (xmlChar*)"y", (xmlChar*)y);

  free(x);
  free(y);
  free(link);
  free(id);

  double fontsize = 5;
  
  if (w < (fontsize * strlen(key)))
    w = fontsize * strlen(key);

  xmlNodePtr box = xmlNewNode(NULL, (xmlChar*)"path");
  char *d = g_strdup_printf("M%f %f L %f %f L %f %f L %f %f z",
			    catalog->width, catalog->height,
			    catalog->width + w, catalog->height,
			    catalog->width + w, catalog->height + h,
			    catalog->width, catalog->height + h);
  char *style = g_strdup_printf("stroke:red; fill:none; stroke-width:1pt;");
  xmlSetProp(box, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(box, (xmlChar*)"d", (xmlChar*)d);

  free(style);
  free(d);

  xmlNodePtr text = xmlNewNode(NULL, (xmlChar*)"text");
  x = g_strdup_printf("%4.2f", catalog->width);
  y = g_strdup_printf("%4.2f", catalog->height + h - 2);
  xmlSetProp(text, (xmlChar*)"x", (xmlChar*)x);
  xmlSetProp(text, (xmlChar*)"y", (xmlChar*)y);
  xmlSetProp(text, (xmlChar*)"style", (xmlChar*)"font-size:4pt;font-family:Courier;");
  xmlAddChild(text, xmlNewText((xmlChar*)key));
  free(x);
  free(y);

  xmlAddChild(catalog->svg, box);
  xmlAddChild(catalog->svg, text);
  xmlAddChild(catalog->svg, use);

  printf("Converting symbol %s [%f %f @ %f %f] to svg...", key, h, w, catalog->width, catalog->height);
  
  if (h > catalog->lineh)
    catalog->lineh = h;

  catalog->width += w;
  if (catalog->width > 400)
    {
      if (catalog->width > catalog->linew)
	catalog->linew = catalog->width;
      catalog->height += catalog->lineh;
      catalog->lineh = catalog->width = 0.0;
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

  printf("Symbol table to SVG [%d symbols]\n", g_tree_nnodes(fcw->symbolTab));
  
  xmlDocSetRootElement(catalog.doc, catalog.svg);
  xmlAddChild(catalog.svg, catalog.defs);
  g_tree_foreach(fcw->symbolTab, (GTraverseFunc)SymbolToSvg, &catalog);

  xmlSetProp(catalog.svg, (xmlChar*)"version", (xmlChar*)"1.1");
  xmlSetProp(catalog.svg, (xmlChar*)"xmlns", (xmlChar*)"http://www.w3.org/2000/svg");
  xmlSetProp(catalog.svg, (xmlChar*)"xmlns:xlink", (xmlChar*)"http://www.w3.org/1999/xlink");

  printf("width=%f height=%f linew=%f lineh=%f\n", catalog.width, catalog.height, catalog.linew, catalog.lineh);
  
  char *width = g_strdup_printf("%6.2f", catalog.linew);
  char *height = g_strdup_printf("%6.2f", catalog.height + catalog.lineh);

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
  
  for (GTreeNode *node = g_tree_node_first(fcw->symbolTab); node != 0; node = g_tree_node_next(node))
    {
      const char *key = g_tree_node_key(node);
      Symbol *sym = (Symbol*)g_tree_node_value(node);
      SYMDEF* esym = sym->original;
      double scale = 20.0;
      double border = 2.0;
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
