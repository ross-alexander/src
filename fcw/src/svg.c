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

    // xmlCopyNode(sym->svg, 1);
  char *id = g_strdup(key);
  g_strdelimit(id, " ", '-');
  xmlSetProp(clone, (xmlChar*)"id", (xmlChar*)id);
  xmlAddChild(stuff->defs, clone);

  double h = sym->original->Hi.y - sym->original->Low.y + 12.0;
  double w = sym->original->Hi.x - sym->original->Low.x + 4.0;
  
  xmlNodePtr use = xmlNewNode(NULL, (xmlChar*)"use");
  char *link = g_strdup_printf("#%s", id);
  char *x = g_strdup_printf("%4.2f", stuff->width - sym->original->Low.x);
  char *y = g_strdup_printf("%4.2f", stuff->height - sym->original->Low.y);

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
			    stuff->width, stuff->height,
			    stuff->width + w, stuff->height,
			    stuff->width + w, stuff->height + h,
			    stuff->width, stuff->height + h);
  char *style = g_strdup_printf("stroke:red; fill:none; stroke-width:1pt;");
  xmlSetProp(box, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(box, (xmlChar*)"d", (xmlChar*)d);

  free(style);
  free(d);

  xmlNodePtr text = xmlNewNode(NULL, (xmlChar*)"text");
  x = g_strdup_printf("%4.2f", stuff->width);
  y = g_strdup_printf("%4.2f", stuff->height + h - 2);
  xmlSetProp(text, (xmlChar*)"x", (xmlChar*)x);
  xmlSetProp(text, (xmlChar*)"y", (xmlChar*)y);
  xmlSetProp(text, (xmlChar*)"style", (xmlChar*)"font-size:4pt;font-family:Courier;");
  xmlAddChild(text, xmlNewText((xmlChar*)key));
  free(x);
  free(y);

  xmlAddChild(stuff->svg, box);
  xmlAddChild(stuff->svg, text);
  xmlAddChild(stuff->svg, use);

  printf("Converting symbol %s [%f %f @ %f %f] to svg...", key, h, w, stuff->width, stuff->height);
  
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
  stuff.width = stuff.height = stuff.lineh = stuff.linew = 0.0;

  printf("Symbol table to SVG\n");
  
  xmlDocSetRootElement(stuff.doc, stuff.svg);
  xmlAddChild(stuff.svg, stuff.defs);
  stuff.width = stuff.height = 0.0;
  g_tree_foreach(fcw->symbolTab, (GTraverseFunc)SymbolToSvg, &stuff);

  xmlSetProp(stuff.svg, (xmlChar*)"version", (xmlChar*)"1.1");
  xmlSetProp(stuff.svg, (xmlChar*)"xmlns", (xmlChar*)"http://www.w3.org/2000/svg");
  xmlSetProp(stuff.svg, (xmlChar*)"xmlns:xlink", (xmlChar*)"http://www.w3.org/1999/xlink");

  printf("width=%f height=%f linew=%f lineh=%f\n", stuff.width, stuff.height, stuff.linew, stuff.lineh);
  
  char *width = g_strdup_printf("%6.2f", stuff.linew);
  char *height = g_strdup_printf("%6.2f", stuff.height + stuff.lineh);

  xmlSetProp(stuff.svg, (xmlChar*)"width", (xmlChar*)width);
  xmlSetProp(stuff.svg, (xmlChar*)"height", (xmlChar*)height);

  free(width);
  free(height);

  char *fname = g_strdup_printf("%s.svg", fcw->file);
  xmlSaveFormatFile(fname, stuff.doc, 1);
  free(fname);

  xmlFreeDoc(stuff.doc);
}
