#include <string.h>

#include <fcw.h>

struct SymToSvgStuff {
  Fcw *fcw;
  int flags;
  xmlDocPtr doc;
  xmlNodePtr defs;
  xmlNodePtr svg;
  double lineh;
  double linew;
  double height;
  double width;
};

int SymbolToSvg(char *key, Symbol *sym, struct SymToSvgStuff *stuff)
{
  Matrix3f transform = Matrix3fIdentity();
  transform.v[1][1] = -1.0; // reflection
  transform.v[1][2] = sym->original->Hi.y + sym->original->Low.y; // translate

  xmlNodePtr clone = SublistToSvg(stuff->fcw, sym->sublist, transform, stuff->flags);

  printf("Converting symbol %s to svg...", key);

    // xmlCopyNode(sym->svg, 1);
  xmlChar *id = g_strdup(key);
  g_strdelimit(id, " ", '-');
  xmlSetProp(clone, "id", id);
  xmlAddChild(stuff->defs, clone);

  double h = sym->original->Hi.y - sym->original->Low.y + 12.0;
  double w = sym->original->Hi.x - sym->original->Low.x + 4.0;

  xmlNodePtr use = xmlNewNode(NULL, "use");
  char *link = g_strdup_printf("#%s", id);
  char *x = g_strdup_printf("%4.2f", stuff->width - sym->original->Low.x);
  char *y = g_strdup_printf("%4.2f", stuff->height - sym->original->Low.y);

  xmlSetProp(use, "xlink:href", (xmlChar*)link);
  xmlSetProp(use, "x", x);
  xmlSetProp(use, "y", y);

  free(x);
  free(y);
  free(link);
  free(id);

  if (w < strlen(key))
    w = 5 * strlen(key);

  xmlNodePtr box = xmlNewNode(NULL, "path");
  char *d = g_strdup_printf("M%f %f L %f %f L %f %f L %f %f z",
			    stuff->width, stuff->height,
			    stuff->width + w, stuff->height,
			    stuff->width + w, stuff->height + h,
			    stuff->width, stuff->height + h);
  char *style = g_strdup_printf("stroke:red; fill:none; stroke-width:1pt;");
  xmlSetProp(box, "style", style);
  xmlSetProp(box, "d", d);

  free(style);
  free(d);

  xmlNodePtr text = xmlNewNode(NULL, "text");
  x = g_strdup_printf("%4.2f", stuff->width);
  y = g_strdup_printf("%4.2f", stuff->height + h - 2);
  xmlSetProp(text, "x", x);
  xmlSetProp(text, "y", y);
  xmlSetProp(text, "style", "font-size:4pt;font-family:Courier;");
  xmlAddChild(text, xmlNewText(key));
  free(x);
  free(y);

  xmlAddChild(stuff->svg, box);
  xmlAddChild(stuff->svg, text);
  xmlAddChild(stuff->svg, use);
  if (h > stuff->lineh)
    stuff->lineh = h;

  stuff->width += w;
  if (stuff->width > 400)
    {
      if (stuff->width > stuff->linew)
	stuff->linew = stuff->width;
      stuff->height += stuff->lineh;
      stuff->lineh = stuff->width = 0.0;
    }
  printf("\n");
  return 0;
}

/* ----------------------------------------------------------------------
--
-- SymTabToSvg
--
---------------------------------------------------------------------- */
void SymTabToSvg(Fcw *fcw)
{
  struct SymToSvgStuff stuff;
  stuff.doc = xmlNewDoc((xmlChar*)"1.0");
  stuff.svg = xmlNewNode(NULL, (xmlChar*)"svg");
  stuff.defs = xmlNewNode(NULL, (xmlChar*)"defs");
  stuff.fcw = fcw;
  stuff.flags = 0;
  stuff.width = stuff.height = stuff.lineh = 0.0;

  xmlDocSetRootElement(stuff.doc, stuff.svg);
  xmlAddChild(stuff.svg, stuff.defs);
  stuff.width = stuff.height = 0.0;
  g_tree_foreach(fcw->symbolTab, (GTraverseFunc)SymbolToSvg, &stuff);

  xmlSetProp(stuff.svg, "version", "1.1");
  xmlSetProp(stuff.svg, "xmlns", "http://www.w3.org/2000/svg");
  xmlSetProp(stuff.svg, "xmlns:xlink", "http://www.w3.org/1999/xlink");

  char *width = g_strdup_printf("%6.2f", stuff.linew);
  char *height = g_strdup_printf("%6.2f", stuff.height + stuff.lineh);

  xmlSetProp(stuff.svg, "width", width);
  xmlSetProp(stuff.svg, "height", height);

  free(width);
  free(height);

  char *fname = g_strdup_printf("%s.svg", fcw->file);
  xmlSaveFormatFile(fname, stuff.doc, 1);
  free(fname);

  xmlFreeDoc(stuff.doc);
}
