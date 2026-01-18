#ifndef GNOME_H

#define GNOME_H


typedef void (*GripSignalFunc) (void);

typedef struct _GnomeClient {
} GnomeClient;

typedef struct _GnomeApp {
} GnomeApp;

typedef enum
{
  /* update structure when adding an enum */
  GNOME_SAVE_GLOBAL,
  GNOME_SAVE_LOCAL,
  GNOME_SAVE_BOTH
} GnomeSaveStyle;

typedef enum
{
  GNOME_INTERACT_NONE,
  GNOME_INTERACT_ERRORS,
  GNOME_INTERACT_ANY
} GnomeInteractStyle;


#endif
