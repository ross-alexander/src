#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "fcw.h"

#ifdef SVG
#include <glib.h>
#include <libxml/tree.h>
#endif

int DefaultPalette[256]={
  0x000000000, // 0 = black
  0x00000FF00, // 1 = green
  0x000E00000, // 2 = red
  0x0000000FF, // 3 = blue
  0x000FFFF00, // 4 = yellow
  0x00000FFFF, // 5 = cyan
  0x000FF00FF, // 6 = magenta
  0x000AF00FF, // 7 = purple
  0x000FF6700, // 8 = orange
  0x0006888FF, // 9 = light blue
  0x000F89858, // 10 = sand
  0x000A84800, // 11 = brown
  0x000888838, // 12 = olive green
  0x000006838, // 13 = forest green
  0x000606060, // 14 = grey
  0x000FFFFFF, // 15 = white
  
  0x000100000, // 16 = button face
  0x000110000, // 17 = button text
  0x000120000, // 18 = button shadow
  0x000FFF8E0, // 19 = eggshell paper color
  
  0x000008080, // 20
  0x000E87640, // 21
  0x000678096, // 22
  0x000509E65, // 23
  0x000B1403D, // 24
  0x000A84A79, // 25
  0x000408080, // 26
  0x0004393BE, // 27
  0x0008080C0, // 28
  0x000FFF0BD, // 29
  0x00056ABAB, // 30
  0x000FFF8E0, // 31
  
  0x00025160B, // 32
  0x000372110, // 33
  0x000492C15, // 34
  0x0005B371A, // 35
  0x0006D421F, // 36
  0x0007F4D24, // 37
  0x000915829, // 38
  0x000A3632E, // 39
  0x000AD7445, // 40
  0x000B7855C, // 41
  0x000C19673, // 42
  0x000CBA78A, // 43
  0x000D5B8A1, // 44
  0x000DFC9B8, // 45
  0x000E9DACF, // 46
  0x000F3EBE6, // 47
  0x000080A3B, // 48
  0x0000B0B57, // 49
  0x0000E0C73, // 50
  0x000110D8F, // 51
  0x000140EAB, // 52
  0x000170FC7, // 53
  0x0001A10E3, // 54
  0x0001D11FF, // 55
  0x000362BFF, // 56
  0x0004F45FF, // 57
  0x000685FFF, // 58
  0x0008179FF, // 59
  0x0009A93FF, // 60
  0x000B3ADFF, // 61
  0x000CCC7FF, // 62
  0x000E5E1FF, // 63
  0x00003373B, // 64
  0x000044F57, // 65
  0x000056773, // 66
  0x000067F8F, // 67
  0x0000797AB, // 68
  0x00008AFC7, // 69
  0x00009C7E3, // 70
  0x0000ADFFF, // 71
  0x00025E2FF, // 72
  0x00040E5FF, // 73
  0x0005BE8FF, // 74
  0x00076EBFF, // 75
  0x00091EEFF, // 76
  0x000ACF1FF, // 77
  0x000C7F4FF, // 78
  0x000E2F7FF, // 79
  0x000062E17, // 80
  0x000084422, // 81
  0x0000A5A2D, // 82
  0x0000C7038, // 83
  0x0000E8643, // 84
  0x000109C4E, // 85
  0x00012B259, // 86
  0x00014C864, // 87
  0x0002ECE75, // 88
  0x00048D486, // 89
  0x00062DA97, // 90
  0x0007CE0A8, // 91
  0x00096E6B9, // 92
  0x000B0ECCA, // 93
  0x000CAF2DB, // 94
  0x000E4F8EC, // 95
  0x0000A3907, // 96
  0x0000F5509, // 97
  0x00014710B, // 98
  0x000198D0D, // 99
  0x0001EA90F, // 100
  0x00023C511, // 101
  0x00028E113, // 102
  0x0002DFD15, // 103
  0x00044FD2F, // 104
  0x0005BFD49, // 105
  0x00072FD63, // 106
  0x00089FD7D, // 107
  0x000A0FD97, // 108
  0x000B7FDB1, // 109
  0x000CEFDCB, // 110
  0x000E5FDE5, // 111
  0x0002D3A0C, // 112
  0x00043560F, // 113
  0x000597212, // 114
  0x0006F8E15, // 115
  0x00085AA18, // 116
  0x0009BC61B, // 117
  0x000B1E21E, // 118
  0x000C7FE21, // 119
  0x000CDFE39, // 120
  0x000D3FE51, // 121
  0x000D9FE69, // 122
  0x000DFFE81, // 123
  0x000E5FE99, // 124
  0x000EBFEB1, // 125
  0x000F1FEC9, // 126
  0x000F7FEE1, // 127
  
  0x0003B3709, // 128
  0x00057500C, // 129
  0x00073690F, // 130
  0x0008F8212, // 131
  0x000AB9B15, // 132
  0x000C7B418, // 133
  0x000E3CD1B, // 134
  0x000FFE61E, // 135

  0x000FFE837, // 136
  0x000FFEA50, // 137
  0x000FFEC69, // 138
  0x000FFEE82, // 139
  
  0x000FFF09B, // 140
  0x000FFF2B4, // 141
  0x000FFF4CD, // 142
  0x000FFF6E6, // 143
  0x0003A1D04, // 144
  0x000562806, // 145
  0x000723308, // 146
  0x0008E3E0A, // 147
  
  0x000AA490C, // 148
  0x000C6540E, // 149
  0x000E25F10, // 150
  0x000FE6A12, // 151
  0x000FE7A2C, // 152
  0x000FE8A46, // 153
  0x000FE9A60, // 154
  0x000FEAA7A, // 155
  0x000FEBA94, // 156
  0x000FECAAE, // 157
  0x000FEDAC8, // 158
  0x000FEEAE3, // 159
  0x0003A0303, // 160
  0x000560403, // 161
  0x000720503, // 162
  0x0008E0603, // 163
  0x000AA0703, // 164
  0x000C60803, // 165
  0x000E20903, // 166
  0x000FE0A03, // 167
  0x000FE251F, // 168
  0x000FE403B, // 169
  0x000FE5B57, // 170
  0x000FE7673, // 171
  0x000FE918F, // 172
  0x000FEACAB, // 173
  0x000FEC7C7, // 174
  0x000FEE2E3, // 175
  0x0003A071D, // 176
  0x000560729, // 177
  0x000720735, // 178
  0x0008E0741, // 179
  0x000AA074D, // 180
  0x000C60759, // 181
  0x000E20765, // 182
  0x000FE0771, // 183
  0x000FE2280, // 184
  0x000FE3D8F, // 185
  0x000FE589E, // 186
  0x000FE73AD, // 187
  0x000FE8EBC, // 188
  0x000FEA9CB, // 189
  0x000FEC4DA, // 190
  0x000FEDFE9, // 191
  0x0003A042B, // 192
  0x00056063F, // 193
  0x000720853, // 194
  0x0008E0A67, // 195
  0x000AA0C7B, // 196
  0x000C60E8F, // 197
  0x000E210A3, // 198
  0x000FE12B7, // 199
  0x000FE2CBF, // 200
  0x000FE46C7, // 201
  0x000FE60CF, // 202
  0x000FE7AD7, // 203
  0x000FE94DF, // 204
  0x000FEAEE7, // 205
  0x000FEC8EF, // 206
  0x000FEE2F7, // 207
  0x00034063B, // 208
  0x0004A0757, // 209
  0x000600873, // 210
  0x00076098F, // 211
  0x0008C0AAB, // 212
  0x000A70BC7, // 213
  0x000B80CE3, // 214
  0x000CE0DFF, // 215
  0x000D327FF, // 216
  0x000D841FF, // 217
  0x000DD5BFF, // 218
  0x000E275FF, // 219
  0x000E78FFF, // 220
  0x000ECA9FF, // 221
  0x000F1C3FF, // 222
  0x000F6DDFF, // 223
  0x00024063B, // 224
  0x000360657, // 225
  0x000480673, // 226
  0x0005A068F, // 227
  0x0006C06AB, // 228
  0x0007E06C7, // 229
  0x0009006E3, // 230
  0x000A206FF, // 231
  0x000AC21FF, // 232
  0x000B63CFF, // 233
  0x000C057FF, // 234
  0x000CA72FF, // 235
  0x000D48DFF, // 236
  0x000DEA8FF, // 237
  0x000E8C3FF, // 238
  0x000FFDEF2, // 239
  0x0001C1C1C, // 240
  0x000282828, // 241
  0x000343434, // 242
  0x000404040, // 243
  0x0004C4C4C, // 244
  0x000585858, // 245
  0x000646464, // 246
  0x000707070, // 247
  0x0007F7F7F, // 248
  0x0008E8E8E, // 249
  0x0009D9D9D, // 250
  0x000ACACAC, // 251
  0x000BBBBBB, // 252
  0x000CACACA, // 253
  0x000D9D9D9, // 254
  0x000E8E8E8, // 255
};

/* ----------------------------------------------------------------------
--
-- Note that vectors are row vectors so all matrix multiplies are
-- v' = v.M.  Matrices are M[col][row].
--
---------------------------------------------------------------------- */

Vector3f VectorMatrix3fMult(Vector3f v, Matrix3f m)
{
  Vector3f r;
  for (int i = 0; i < 3; i++)
    {
      r.v[i] = 0.0;
      for (int j = 0; j < 3; j++)
	r.v[i] += v.v[j] * m.v[i][j];
    }
  return r;
}

Matrix3f Matrix3fIdentity()
{
  Matrix3f r;
  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      r.v[i][j] = 0.0;
  for (int k = 0; k < 3; k++)
    r.v[k][k] = 1.0;
  return r;
}

Matrix3f MatrixMatrix3fMult(Matrix3f a, Matrix3f b)
{
  Matrix3f c;
  for (int i = 0; i < 3; i++) // cols
    for (int j = 0; j < 3; j++) // rows
      {
	double s = 0.0;
	for (int k = 0; k < 3; k++)
	  s += a.v[k][j] * b.v[i][k];
	c.v[i][j] = s;
      }
  return c;
}

Vector3f Vector3fFrom2f(float x, float y)
{
  Vector3f r;
  r.v[0] = x;
  r.v[1] = y;
  r.v[2] = 1.0;
  return r;
}

/* ----------------------------------------------------------------------
--
-- SymbolTableToSvg
--
---------------------------------------------------------------------- */

int SymbolTableToSvg(gpointer key, gpointer data, gpointer user_data)
{
  printf("Adding symbol %s to table.\n", key);
  Symbol *sym = (Symbol*)data;
  xmlNodePtr clone = xmlCopyNode(sym->svg, 1);
  xmlChar *id = xmlGetProp(clone, (xmlChar*)"id");
  g_strdelimit((char*)id, " ", '-');
  xmlSetProp(clone, (xmlChar*)"id", (xmlChar*)id);
  xmlAddChild((xmlNodePtr)user_data, clone);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- SymrefToSvg
--
---------------------------------------------------------------------- */
xmlNodePtr SymrefToSvg(Fcw *fcw, Entity *e, SYMREF *sref, Matrix3f transform, int flags)
{
  Matrix3f symtrans = Matrix3fIdentity();
  symtrans.v[0][2] = sref->TMat.m41;
  symtrans.v[1][2] = sref->TMat.m42;
  Matrix3f fulltrans = MatrixMatrix3fMult(symtrans, transform);
  Symbol *sym = (Symbol*)g_tree_lookup(fcw->symbolTab, sref->SName);

#ifdef UseSvgUse
  xmlNodePtr use = xmlNewNode(NULL, (xmlChar*)"use");
  if (sym)
    {
      char *link = g_strdup_printf("#%s", sref->SName);
      g_strdelimit(link, " ", '-');
      xmlSetProp(use, (xmlChar*)"xlink:href", (xmlChar*)link);
      char *xform = g_strdup_printf("matrix(%f %f %f %f %f %f)",
				    fulltrans.v[0][0], fulltrans.v[1][0],
				    fulltrans.v[0][1], fulltrans.v[1][1],
				    fulltrans.v[0][2], fulltrans.v[1][2]);
      xmlSetProp(use, (xmlChar*)"transform", (xmlChar*)xform);	    
      free(xform);
      free(link);
      return use;
    }
#else
  if (sym)
    {
      xmlNodePtr clone = xmlCopyNode(sym->svg, 1);
      xmlUnsetProp(clone, (xmlChar*)"id");
      char *xform = g_strdup_printf("matrix(%f %f %f %f %f %f)",
				    fulltrans.v[0][0], fulltrans.v[1][0],
				    fulltrans.v[0][1], fulltrans.v[1][1],
				    fulltrans.v[0][2], fulltrans.v[1][2]);
      xmlSetProp(clone, (xmlChar*)"transform", (xmlChar*)xform);
      free(xform);
      return clone;
    }
#endif
  return NULL;
}

/* ----------------------------------------------------------------------
--
-- PointToSvg
--
---------------------------------------------------------------------- */
xmlNodePtr PointToSvg(Fcw *fcw, Point2 *g, Matrix3f transform, int flags)
{
  char stroke[8], fill[8];
  sprintf(stroke, "#%06x", DefaultPalette[g->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[g->EColor2]);

  Vector3f p1 = VectorMatrix3fMult(Vector3fFrom2f(g->Point.x, g->Point.y), transform);
	
  char *x = g_strdup_printf("%f", p1.v[0]);
  char *y = g_strdup_printf("%f", p1.v[1]);
  char *r = g_strdup_printf("%fpt", 1.0);

  xmlNodePtr cir = xmlNewNode(NULL, (xmlChar*)"circle");
  char *style = g_strdup_printf("stroke:%s; fill:none;", stroke);


  if (!(flags & ENT_TO_SVG_INHERIT))
    xmlSetProp(cir, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(cir, (xmlChar*)"x", (xmlChar*)x);
  xmlSetProp(cir, (xmlChar*)"y", (xmlChar*)y);
  xmlSetProp(cir, (xmlChar*)"r", (xmlChar*)r);
  free(x);
  free(y);
  free(r);
  free(style);
  return cir;
}


/* ----------------------------------------------------------------------
--
-- LineToSvg
--
---------------------------------------------------------------------- */
xmlNodePtr LineToSvg(Fcw *fcw, Line2 *g, Matrix3f transform, int flags)
{
  char stroke[8], fill[8];
  sprintf(stroke, "#%06x", DefaultPalette[g->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[g->EColor2]);

  Vector3f p1 = VectorMatrix3fMult(Vector3fFrom2f(g->Line.p1.x, g->Line.p1.y), transform);
  Vector3f p2 = VectorMatrix3fMult(Vector3fFrom2f(g->Line.p2.x, g->Line.p2.y), transform);
	
  char *d = g_strdup_printf("M %f %f L %f %f", p1.v[0], p1.v[1], p2.v[0], p2.v[1]);
  xmlNodePtr path = xmlNewNode(NULL, (xmlChar*)"path");
  char *style = g_strdup_printf("stroke:%s; fill:none;stroke-width:0.1pt;", stroke);
  if (!(flags & ENT_TO_SVG_INHERIT))
    xmlSetProp(path, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(path, (xmlChar*)"d", (xmlChar*)d);
  free(d);
  free(style);
  return path;
}

/* ----------------------------------------------------------------------
--
-- PathToSvg
--
---------------------------------------------------------------------- */
xmlNodePtr PathToSvg(Fcw *fcw, Path2 *g, Matrix3f transform, int flags)
{
  char stroke[8], fill[8];
  sprintf(stroke, "#%06x", DefaultPalette[g->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[g->EColor2]);

  int strsize = 0;
  int nodes = g->Count;
  char cmd = 'M';
  char **strbits = (char**)calloc(sizeof(char*), nodes + 1);
  float *f = (float*)calloc(sizeof(float), nodes * 2);
  for (int k = 0; k < nodes; k++)
    {
      GPOINT2 *n = g->Nodes;
      Vector3f v = VectorMatrix3fMult(Vector3fFrom2f(n[k].x, n[k].y), transform);
      strbits[k] = g_strdup_printf("%c %hf %hf ", cmd, v.v[0], v.v[1]);
      strsize += strlen(strbits[k]);
      cmd = 'L';
    }
  if (g->Flags & NL_CLS)
    {
      strbits[nodes] = g_strdup("z");
      strsize += 1;
    }
  char *d = (char*)calloc(1, strsize + 2);
  
  strsize = 0;
  for (int k = 0; k < nodes + 1; k++)
    {
      if (strbits[k])
	{
	  strcat(d + strsize, strbits[k]);
	  free(strbits[k]);
	}
    }
  char *style;
  if (!(g->Flags && NL_CLS))
    {
      if (g->LWidth)
	style = g_strdup_printf("stroke:%s; fill:none; stroke-width:%hf;", stroke, g->LWidth);
      else
	style = g_strdup_printf("stroke:%s; fill:none; stroke-width:1px;", stroke);
    }
  else
    {
      if (g->EFStyle == 0)
	style = g_strdup_printf("stroke:%s; fill:none; stroke-width:%hf;", stroke, g->LWidth);
      else
	style = g_strdup_printf("stroke:%s; fill:%s; stroke-width:%hf;", stroke, g->LWidth == 0.0 ? fill : "none", g->LWidth);
    }
  xmlNodePtr path = xmlNewNode(NULL, (xmlChar*)"path");
  if (!(flags & ENT_TO_SVG_INHERIT))
    xmlSetProp(path, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(path, (xmlChar*)"d", (xmlChar*)d);
  free(style);
  free(d);
  return path;
}

/* ----------------------------------------------------------------------
--
-- CirToSvg
--
---------------------------------------------------------------------- */

xmlNodePtr CirToSvg(Fcw *fcw, Cir2 *cir, Matrix3f transform, int flags)
{
  char stroke[8], fill[8];
  sprintf(stroke, "#%06x", DefaultPalette[cir->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[cir->EColor2]);

  double x = cir->Circle.Center.x;
  double y = cir->Circle.Center.y;
  double r = cir->Circle.Radius;

  Vector3f p1 = VectorMatrix3fMult(Vector3fFrom2f(x, y), transform);

  char *xt = g_strdup_printf("%5.2f", p1.v[0]);
  char *yt = g_strdup_printf("%5.2f", p1.v[1]);
  char *rt = g_strdup_printf("%5.2f", r);

  xmlNodePtr circle = xmlNewNode(NULL, (xmlChar*)"circle");
  char *style = g_strdup_printf("stroke:%s; fill:none;stroke-width:1px;",
				stroke);
  if (!(flags & ENT_TO_SVG_INHERIT))
    xmlSetProp(circle, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(circle, (xmlChar*)"x", (xmlChar*)xt);
  xmlSetProp(circle, (xmlChar*)"y", (xmlChar*)yt);
  xmlSetProp(circle, (xmlChar*)"r", (xmlChar*)rt);
  free(xt);
  free(yt);
  free(rt);
  free(style);
  return circle;
}

/* ----------------------------------------------------------------------
--
-- ArcToSvg
--
---------------------------------------------------------------------- */

xmlNodePtr ArcToSvg(Fcw *fcw, Arc2 *arc, Matrix3f transform, int flags)
{
  char stroke[8], fill[8];
  sprintf(stroke, "#%06x", DefaultPalette[arc->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[arc->EColor2]);

  double x = arc->Arc.Circle.Center.x;
  double y = arc->Arc.Circle.Center.y;
  double r = arc->Arc.Circle.Radius;
  double a = arc->Arc.SAng;
  double b = arc->Arc.AngW;
  Vector3f p1 = Vector3fFrom2f(x + r * cos(a), y + r * sin(a));
  Vector3f p2 = Vector3fFrom2f(x + r * cos(a + b), y + r * sin(a + b));
  
  //  printf("Arc2 %hhu %f %f %f %f %f\n", arc->EColor, x, y, r, a * 180 / M_PI, b * 180 / M_PI);
  char *d = g_strdup_printf("M %f %f A %f %f %f %d %d %f %f",
			    p1.v[0], p1.v[1],
			    r, r, 0.0,
			    b > M_PI ? 1 : 0, 1,
			    p2.v[0], p2.v[1]);
  
  char *xform = g_strdup_printf("matrix(%f %f %f %f %f %f)",
				transform.v[0][0], transform.v[1][0],
				transform.v[0][1], transform.v[1][1],
				transform.v[0][2], transform.v[1][2]);

  //	printf("d = %s\n", d);
  xmlNodePtr path = xmlNewNode(NULL, (xmlChar*)"path");
  char *style = g_strdup_printf("stroke:%s; fill:none;stroke-width:0.1pt;",
				stroke);
  if (!(flags & ENT_TO_SVG_INHERIT))
    xmlSetProp(path, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(path, (xmlChar*)"transform", (xmlChar*)xform);
  xmlSetProp(path, (xmlChar*)"d", (xmlChar*)d);
  free(d);
  free(xform);
  free(style);
  return path;
}

/* ----------------------------------------------------------------------
--
-- ElpToSvg
--
---------------------------------------------------------------------- */

xmlNodePtr ElpToSvg(Fcw *fcw, Elp2 *cir, Matrix3f transform, int flags)
{
  char stroke[8], fill[8];
  sprintf(stroke, "#%06x", DefaultPalette[cir->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[cir->EColor2]);

  double x = cir->Circle.Circle.Center.x;
  double y = cir->Circle.Circle.Center.y;
  double r = cir->Circle.Circle.Radius;
  double ecc = cir->Circle.Ecc;
  double incl = cir->Circle.Incl;

  Vector3f p1 = VectorMatrix3fMult(Vector3fFrom2f(x, y), transform);

  //  printf("Elp %f %f %f %f\n", x, y, ecc, incl);

  char *cxt = g_strdup_printf("%f", 0.0);
  char *cyt = g_strdup_printf("%f", 0.0);
  char *rxt = g_strdup_printf("%f", r);
  char *ryt = g_strdup_printf("%f", r * ecc);
  char *rotate = g_strdup_printf("matrix(%f %f %f %f %f %f) translate(%f %f) rotate(%f)",
				 transform.v[0][0], transform.v[1][0],
				 transform.v[0][1], transform.v[1][1],
				 transform.v[0][2], transform.v[1][2],
				 x, y,
				 incl * 180 / M_PI);

  xmlNodePtr circle = xmlNewNode(NULL, (xmlChar*)"ellipse");
  char *style = g_strdup_printf("stroke:%s; fill:none;stroke-width:0.1pt;",
				stroke);
  if (!(flags & ENT_TO_SVG_INHERIT))
    xmlSetProp(circle, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(circle, (xmlChar*)"cx", (xmlChar*)cxt);
  xmlSetProp(circle, (xmlChar*)"cy", (xmlChar*)cyt);
  xmlSetProp(circle, (xmlChar*)"rx", (xmlChar*)rxt);
  xmlSetProp(circle, (xmlChar*)"ry", (xmlChar*)ryt);
  xmlSetProp(circle, (xmlChar*)"transform", (xmlChar*)rotate);
  free(cxt);
  free(cyt);
  free(rxt);
  free(ryt);
  free(rotate);
  free(style);
  return circle;
}

/* ----------------------------------------------------------------------
--
-- ElaToSvg
--
---------------------------------------------------------------------- */

xmlNodePtr ElaToSvg(Fcw *fcw, Ela2 *cir, Matrix3f transform, int flags)
{
  char stroke[8], fill[8];
  sprintf(stroke, "#%06x", DefaultPalette[cir->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[cir->EColor2]);

  double x = cir->Circle.Elp.Circle.Center.x;
  double y = cir->Circle.Elp.Circle.Center.y;
  double r = cir->Circle.Elp.Circle.Radius;
  double ecc = cir->Circle.Elp.Ecc;
  double incl = cir->Circle.Elp.Incl;

  double sx, sy;
  double a = fmod(cir->Circle.SAng, 2 * M_PI);
  if (a > 3.0 * M_PI/2)
    {
      sx = + cos(atan(tan(2*M_PI - a) / ecc));
      sy = - sin(atan(tan(2*M_PI - a) / ecc));
    }
  else if (a > M_PI)
    {
      sx = - cos(atan(tan(a - M_PI) / ecc));
      sy = - sin(atan(tan(a - M_PI) / ecc));
    }
  else if (a > M_PI/2)
    {
      sx = - cos(atan(tan(M_PI - a) / ecc));
      sy = + sin(atan(tan(M_PI - a) / ecc));
    }
  else
    {
      double theta = atan(tan(a) / ecc);
      sx = + cos(theta);
      sy = + sin(theta);
    }

  double ex, ey;
  double b = fmod(cir->Circle.SAng + cir->Circle.AngW, 2 * M_PI);
  if (b >= 3 * M_PI/2)
    {
      ex = + cos(atan(tan(2*M_PI - b) / ecc));
      ey = - sin(atan(tan(2*M_PI - b) / ecc));
    }
  else if (b >= M_PI)
    {
      ex = - cos(atan(tan(b - M_PI) / ecc));
      ey = - sin(atan(tan(b - M_PI) / ecc));
    }
  else if (b >= M_PI/2)
    {
      ex = - cos(atan(tan(M_PI - b) / ecc));
      ey = + sin(atan(tan(M_PI - b) / ecc));
    }
  else
    {
      double theta = atan(tan(b) / ecc);
      ex = + cos(theta);
      ey = + sin(theta);
    }

  sx *= r;
  sy *= r * ecc;
  ex *= r;
  ey *= r * ecc;

  // T = arctan(A/B tan(Theta))

  char *xform = g_strdup_printf("matrix(%f %f %f %f %f %f)",
				transform.v[0][0], transform.v[1][0],
				transform.v[0][1], transform.v[1][1],
				transform.v[0][2], transform.v[1][2]);

  char *d = g_strdup_printf("M %f %f A %f %f %f %d %d %f %f",
			    x + sx, y + sy,
			    r, r * ecc, fmod(incl, M_PI),
			    cir->Circle.AngW > M_PI ? 1 : 0,
			    1,
			    x + ex, y + ey);

  xmlNodePtr path = xmlNewNode(NULL, (xmlChar*)"path");
  char *style = g_strdup_printf("stroke:%s; fill:none;stroke-width:0.1pt;", stroke);
  if (!(flags & ENT_TO_SVG_INHERIT))
    xmlSetProp(path, (xmlChar*)"style", (xmlChar*)style);
  xmlSetProp(path, (xmlChar*)"transform", (xmlChar*)xform);
  xmlSetProp(path, (xmlChar*)"d", (xmlChar*)d);
  free(d);
  free(xform);
  free(style);

  return path;
}

/* ----------------------------------------------------------------------
--
-- SymdefToSvg
--
---------------------------------------------------------------------- */
xmlNodePtr SymdefToSvg(Fcw *fcw, Entity *e, SYMDEF *sdef, Matrix3f transform, int flags)
{
  Symbol *s = (Symbol*)calloc(sizeof(Symbol), 1);
  Matrix3f ident = Matrix3fIdentity();
  xmlNodePtr svg = SublistToSvg(fcw, e->sublist, ident, flags);
  char *name = g_strdup(sdef->SName);
  xmlSetProp(svg, (xmlChar*)"id", (xmlChar*)name);

  s->original = sdef;
  s->sublist = e->sublist;
  s->svg = svg;
  g_tree_insert(fcw->symbolTab, name, s);
  return NULL;
}

/* ----------------------------------------------------------------------
--
-- EntityToSvg
--
---------------------------------------------------------------------- */

xmlNodePtr EntityToSvg(Fcw *fcw, Entity *e, Matrix3f transform, int flags)
{
  Common *common = (Common*)e->ptr;

  xmlNodePtr res = NULL;
  char stroke[8], fill[8];

  printf("EntityToSvg(%d %d)\n", common->EColor, common->EColor2);
  
  sprintf(stroke, "#%06x", DefaultPalette[common->EColor]);
  sprintf(fill, "#%06x", DefaultPalette[common->EColor2]);

  //  printf("Flags %04x\n", flags);
  //  printf("EntityToSvg(%d - %s %s)\n", common->EType, stroke, fill);

  switch(common->EType)
    {
    case 1:
      res = PointToSvg(fcw, (Point2*)common, transform, flags);
      break;
    case 2:
      res = LineToSvg(fcw, (Line2*)common, transform, flags);
      break;
    case 3:
      res = PathToSvg(fcw, (Path2*)common, transform, flags);
      break;
    case 6:
      res = CirToSvg(fcw, (Cir2*)common, transform, flags);
      break;
    case 7:
      res = ArcToSvg(fcw, (Arc2*)common, transform, flags);
      break;
    case 8:
      res = ElpToSvg(fcw, (Elp2*)common, transform, flags);
      break;
    case 9:
      res = ElaToSvg(fcw, (Ela2*)common, transform, flags);
      break;
    case 17:
      {
	if (e->sublist)
	  {
	    Common *g = common;
	    char *style;
	    xmlNodePtr ptr;
	    if (g->EFStyle == 0)
	      style = g_strdup_printf("fill-rule: evenodd; stroke:%s; fill:none; stroke-width:%hf;", stroke, g->LWidth);
	    else
	      style = g_strdup_printf("fill-rule: evenodd; stroke:%s; fill:%s; stroke-width:%hf;", stroke, g->LWidth == 0.0 ? fill : "none", g->LWidth);
	    xmlNodePtr sublist = SublistToSvg(fcw, e->sublist, transform, flags | ENT_TO_SVG_INHERIT);

	    int slen = 0;
	    if (sublist->children)
	      for (ptr = sublist->children; ptr != NULL; ptr = ptr->next)
		{
		  char *p = (char*)xmlGetProp((xmlNode*)ptr, (xmlChar*)"d");
		  if (p)
		    slen += strlen(p) + 1;
		}
	    char *np = (char*)calloc(1, slen + 1);
	    if (sublist->children)
	      for (ptr = sublist->children; ptr != NULL; ptr = ptr->next)
		{
		  char *p = (char*)xmlGetProp((xmlNode*)ptr, (xmlChar*)"d");
		  if (p)
		    {
		      strcat(np, p);
		      strcat(np, " ");
		    }
		}
	    xmlNodePtr path = xmlNewNode(NULL, (xmlChar*)"path");
	    xmlSetProp(path, (xmlChar*)"d", (xmlChar*)np);
	    xmlSetProp(path, (xmlChar*)"style", (xmlChar*)style);
	    free(np);
	    free(style);
	    res = path;
	    xmlFreeNode(sublist);
	  }
	break;
      }
    case 18:
      {
	res = SublistToSvg(fcw, e->sublist, transform, flags);
	break;
      }
    case 28:
      {
	res = SymdefToSvg(fcw, e, (SYMDEF*)common, transform, flags);
	break;
      }
    case 29:
      {
	res = SymrefToSvg(fcw, e, (SYMREF*)common, transform, flags);
	break;
      }
    case 41:
      {
	if (e->sublist)
	  {
	    xmlNodePtr sheet = SublistToSvg(fcw, e->sublist, transform, flags);
	    res = sheet;
	  }
	break;
      }
    default:
      fprintf(stderr, "Unknown Entity %d\n", common->EType);
      break;
    }
  return res;
}

/* ----------------------------------------------------------------------
--
-- SublistToSvg
--
---------------------------------------------------------------------- */

xmlNodePtr SublistToSvg(Fcw *fcw, GPtrArray *sublist, Matrix3f transform, int flags)
{
  xmlNodePtr g = xmlNewNode(NULL, (xmlChar*)"g");

  printf("SublistToSvg(%d)\n", sublist->len);

  for (int i = 0; i < sublist->len; i++)
    {
      xmlNodePtr res = EntityToSvg(fcw, (Entity*)g_ptr_array_index(sublist, i), transform, flags);
      if (res)
	xmlAddChild(g, res);
    }
  if (xmlIsBlankNode(g))
    {
      xmlFreeNode(g);
      return NULL;
    }
  return g;
}

/* ----------------------------------------------------------------------
--
-- FcwToSvg
--
---------------------------------------------------------------------- */
int FcwToSvg(Fcw *fcw)
{

  HDR *hdr = (HDR*)fcw->iblock[0];

  xmlDocPtr doc = xmlNewDoc((xmlChar*)"1.0");
  xmlNodePtr svg = xmlNewNode(NULL, (xmlChar*)"svg");
  xmlDocSetRootElement(doc, svg);

  /* --------------------
     Set width and height
     -------------------- */

  Matrix3f transform = Matrix3fIdentity();
  transform.v[1][1] = -1.0; // reflection
  transform.v[1][2] = hdr->Hi.y + hdr->Low.y; // translate

  float w = hdr->Hi.x - hdr->Low.x;
  float h = hdr->Hi.y - hdr->Low.y;

  char *width = g_strdup_printf("%hfpt", w);
  char *height = g_strdup_printf("%hfpt", h);
  char *x = g_strdup_printf("%hfpt", hdr->Low.x);
  char *y = g_strdup_printf("%hfpt", hdr->Low.y);
  char *viewbox = g_strdup_printf("%hf %hf %hf %hf", hdr->Low.x, hdr->Low.y, w, h);
  xmlSetProp(svg, (xmlChar*)"x", (xmlChar*)x);
  xmlSetProp(svg, (xmlChar*)"y", (xmlChar*)y);
  xmlSetProp(svg, (xmlChar*)"width", (xmlChar*)width);
  xmlSetProp(svg, (xmlChar*)"height", (xmlChar*)height);
  xmlSetProp(svg, (xmlChar*)"viewBox", (xmlChar*)viewbox);
  xmlSetProp(svg, (xmlChar*)"version", (xmlChar*)"1.1");
  xmlSetProp(svg, (xmlChar*)"xmlns", (xmlChar*)"http://www.w3.org/2000/svg");
  xmlSetProp(svg, (xmlChar*)"xmlns:xlink", (xmlChar*)"http://www.w3.org/1999/xlink");

  free(width);
  free(height);
  free(viewbox);
  free(x);
  free(y);

  xmlNodePtr res = SublistToSvg(fcw, fcw->root->sublist, transform, 0x0000);
  if (g_tree_nnodes(fcw->symbolTab))
    {
      xmlNodePtr defs = xmlNewNode(NULL, (xmlChar*)"defs");
      g_tree_foreach(fcw->symbolTab, SymbolTableToSvg, defs);
      xmlAddChild(svg, defs);
    }
  xmlAddChild(svg, res);

  if (!(fcw->flags & FCW_CATALOG))
    {
      char *ofile = g_strdup_printf("%s.svg", fcw->file);
      xmlSaveFormatFile(ofile, doc, 1);
      free(ofile);
    }
  xmlFreeDoc(doc);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- FcwInfoBlock
--
---------------------------------------------------------------------- */

int FcwInfoBlock(Fcw *fcw, InfoBlock *ib)
{
  printf("Infoblock\n");
  switch(ib->IType)
    {
    case 0:
      {
	printf("...Header\n");
	HDR *hdr = (HDR*)ib;
	printf("Size = %d\n", hdr->ERLen);
	printf("Etype = %hhd\n", hdr->EType);
	printf("Itype = %hhd\n", hdr->IType);
	printf("Creator = %d\n", hdr->Creator);
	printf("AllocLen = %d\n", hdr->AllocLen);
	printf("Version = %hhd\n", hdr->HdrVersion);
	printf("DBVersion = %hhd\n", hdr->DBVer);
	printf("(%hf %hf) -> (%hf %hf)\n", hdr->Low.x, hdr->Low.y, hdr->Hi.x, hdr->Hi.y);
	//  printf("CurColor = %d\n", hdr->CurColor);
	printf("UnitV = %hf\n", hdr->UnitV);
	printf("Decimals = %hhd\n", hdr->DecP);
	printf("Pen Thickness %4.2f\n", (double)hdr->PThick / 100.0);      
      break;
      }
    case 2:
      {
	printf("...View\n");
	break;
      }
    case 3:
      {
	printf("...Layer\n");
	break;
      }
    case 4:
      {
	printf("...Grid\n");
	break;
      }
    case 5:
      {
	printf("...Printer settings\n");
	break;
      }
    case 6:
      {
	printf("...Line styles\n");
	break;
      }
    case 7:
      {
	printf("...Fill styles\n");
	break;
      }
    case 8:
      {
	printf("...Named views\n");
	break;
      }
    case 9:
      {
	printf("...Colormap data\n");
	CMAP *cmap = (CMAP*)ib;
	printf("Size = %d\n", cmap->ERLen);
	printf("AllocLen = %hd\n", cmap->AllocLen);
	printf("Ver = %hhd\n", cmap->IBVer);
	break;
      }
    case 16:
      {
	printf("...Dimensions\n");
	break;
      }
    case 17:
      {
	printf("...Fonts\n");
	break;
      }
    default:
      {
	printf("...Unknown (%02hhx size %d)\n", ib->IType, ib->ERLen);
	break;
      }
    }
  return 0;
}

/* ----------------------------------------------------------------------
--
-- FcwEStruct
--
---------------------------------------------------------------------- */

int FcwEStruct(Fcw *fcw, Entity *e)
{
  Common *common = (Common*)e->ptr;
  for (int j = 0; j < e->level; j++)
    printf("..");
  if (common->EType)
    {
      printf("%hhu Color %hhu %hhu Style %hhu %hhu Thick %hhu Width %hf ",
	     common->EType,
	     common->EColor, common->EColor2, 
	     common->ELStyle, common->EFStyle, common->EThick, common->LWidth);
      switch(common->EType)
	{
	case 1:
	  printf("Point2\n");
	  break;
	case 2:
	  {
	    Line2 *g = (Line2*)(common);
	    printf("Line2 %hf %hf %hf %hf\n", 
		   g->Line.p1.x, g->Line.p1.y, g->Line.p2.x, g->Line.p2.y);
	    break;
	  }
	case 3:
	  {
	    Path2 *g = (Path2*)(common);
	    int nodes = g->Count;
	    printf("Path2 with %d nodes flags %0hhx.\n", nodes, g->Flags);
	    break;
	  }
	case 4:
	  printf("Poly2\n");
	  break;
	case 5:
	  printf("Text2\n");
	  break;
	case 6:
	  printf("Circle2\n");
	  break;
	case 7:
	  printf("Arc2\n");
	  break;
	case 8:
	  printf("Ellipse2\n");
	  break;
	case 9:
	  printf("EllipiticArc\n");
	  break;
	case 17:
	  printf("2D Multipoly\n");
	  break;
	case 18:
	  printf("2D Group\n");
	  break;
	case 28:
	  {
	    SYMDEF *sdef = (SYMDEF*)common;
	    printf("SymDef %hf %hf %hf %hf %s\n",
		   sdef->Low.x,
		   sdef->Low.y,
		   sdef->Hi.x,
		   sdef->Hi.y,
		   sdef->SName);
	    break;
	  }
	case 29:
	  {
	    SYMREF *sref = (SYMREF*)common;
	    printf("SymRef %hf %hf %hf %hf %s\n",
		   sref->Low.x,
		   sref->Low.y,
		   sref->Hi.x,
		   sref->Hi.y,
		   sref->SName);
	    printf("%14.4hf %14.4hf %14.4hf %14.4hf\n",
		   sref->TMat.m11, 
		   sref->TMat.m12, 
		   sref->TMat.m13,
		   (float)0.0);
	    printf("%14.4hf %14.4hf %14.4hf %14.4hf\n",
		   sref->TMat.m21, 
		   sref->TMat.m22, 
		   sref->TMat.m23,
		   (float)0.0);
	    printf("%14.4hf %14.4hf %14.4hf %14.4hf\n",
		   sref->TMat.m31, 
		   sref->TMat.m32, 
		   sref->TMat.m33,
		   (float)0.0);
	    printf("%14.4hf %14.4hf %14.4hf %14.4hf\n",
		   sref->TMat.m41, 
		   sref->TMat.m42, 
		   sref->TMat.m43,
		   (float)1.0);
	    break;
	  }
	case 41:
	  printf("Sheet\n");
	  break;
	default:
	  printf("-------------------- Unknown --------------------\n");
	  printf("%d ... %d\n", common->EType, common->ERLen);
	  break;
	}
    }
  else
    {
      InfoBlock *ib = (InfoBlock*)common;
      printf("InfoBlock %d\n", ib->IType);
    }
  if (e->sublist)
    FcwDecodeSublist(fcw, e->sublist);
  return 0;
}

int FcwDecodeSublist(Fcw *fcw, GPtrArray *sublist)
{
  for (int i = 0; i < sublist->len; i++)
    FcwEStruct(fcw, (Entity*)g_ptr_array_index(sublist, i));
  return 0;
}


/* ----------------------------------------------------------------------
--
-- FcwDecodeBuffer
--
---------------------------------------------------------------------- */

int FcwDecodeBuffer(char *file, size_t size, unsigned char *buf)
{
  Fcw* fcw = (Fcw*)calloc(sizeof(Fcw), 1);
  fcw->symbolTab = g_tree_new((GCompareFunc)strcmp);

  char *fcwStr = g_utf8_casefold(".fcw", 4);
  char *fscStr = g_utf8_casefold(".fsc", 4);
  char *fileStr = g_utf8_casefold(file + strlen(file) - 4, 4);

  char *nfile;
  if (!g_utf8_collate(fcwStr, fileStr))
    {
      nfile = (char*)calloc(1, strlen(file) - 4 + 1);
      strncpy(nfile, file, strlen(file) - 4);
    }
  else if (!g_utf8_collate(fscStr, fileStr))
    {
      nfile = (char*)calloc(1, strlen(file) - 4 + 1);
      strncpy(nfile, file, strlen(file) - 4);
      fcw->flags |= FCW_CATALOG;
    }
  else
    {
      nfile = g_strdup(file);
    }
  fcw->file = nfile;

  Entity *root = (Entity*)calloc(sizeof(Entity), 1);
  root->sublist = g_ptr_array_new();
  int sublist = 1;
  GTrashStack *stack = NULL;
  g_trash_stack_push(&stack, root);

  for (int offset = 0; offset < size; )
    {
      Common *ptr = (Common*)(buf + offset);
      int esize = ptr->ERLen;

      /* --------------------
	 If size == 0 then end of entities
	 -------------------- */

      if (esize == 0)
	break;

      int etype = ptr->EType;

      /* --------------------
	 If size == 0 then sublist marker
	 -------------------- */

      Entity *e = (Entity*)g_trash_stack_peek(&stack);

      if (esize == 5)
	{
	  if (etype == 0)
	    {
	      g_trash_stack_push(&stack, g_ptr_array_index(e->sublist, e->sublist->len - 1));
	      sublist++;
	      printf("++ sublist %d\n", sublist);
	    }
	  else
	    {
	      g_trash_stack_pop(&stack);
	      sublist--;
	      printf("-- sublist %d\n", sublist);
	    }
	}
      else if (etype == 0)
	{
	  InfoBlock *ib = (InfoBlock*)(ptr);
	  fcw->iblock[ib->IType] = ib;
	}
      else
	{
	  Entity *n = (Entity*)calloc(sizeof(Entity), 1);
	  n->etype = etype;
	  n->ptr = ptr;
	  n->level = sublist;
	  n->sublist = g_ptr_array_new();
	  FcwEStruct(fcw, n);
	  g_ptr_array_add(e->sublist, n);
	}
      offset += esize;
    }

  fcw->root = (Entity*)g_trash_stack_pop(&stack);
  FcwInfoBlock(fcw, fcw->iblock[0]);

  //  FcwDecodeSublist(fcw, fcw->root->sublist);
  return 0;

  FcwToSvg(fcw);
  if (fcw->flags & FCW_CATALOG)
    SymTabToSvg(fcw);
  return 1;
}
