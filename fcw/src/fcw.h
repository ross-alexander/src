#include <geometry.h>
#include <header.h>
#include <glib.h>
#include <libxml/tree.h>

typedef struct Matrix3f {
  float v[3][3];
} Matrix3f;


typedef struct Vector3f {
  float v[3];
} Vector3f;

#define IB_Max 0x12

typedef struct {
  char *file;
  int flags;
  InfoBlock* iblock[IB_Max];
  int icount;
  struct Entity *root;
  GTree *symbolTab;
  unsigned int width;
  unsigned int height;
} Fcw;

typedef struct Entity {
  int etype;
  void *ptr;
  int level;
  GPtrArray *sublist;
  struct Entity *parent;
} Entity;

typedef struct Symbol {
  SYMDEF* original;
  GPtrArray *sublist;
  xmlNodePtr svg;
} Symbol;

typedef struct fcw_global_t {
  int dump;
  int cairo;
} fcw_global_t;


#define FCW_CATALOG 0x0001
#define ENT_TO_SVG_INHERIT 0x0001

extern int DefaultPalette[];
int FcwDecodeBuffer(fcw_global_t *, char *file, size_t size, unsigned char *buf);
int FcwDecodeSublist(Fcw*, GPtrArray*);
xmlNodePtr SublistToSvg(Fcw *fcw, GPtrArray *, Matrix3f, int);
extern void SymTabToSvg(Fcw *fcw);
extern int FcwToCairo(Fcw *);
extern Matrix3f Matrix3fIdentity();
