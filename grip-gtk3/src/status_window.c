/* status_window.c -- routines for display status information
 *
 * Copyright (c) 1998-2004  Mike Oliphant <grip@nostatic.org>
 *
 *   http://www.nostatic.org/grip
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <vte/vte.h>
#include "status_window.h"
#include <gdk/gdk.h>

static int PipeCB(GIOChannel* source, GIOCondition condition, gpointer data);

/* Create a new status window */
StatusWindow *NewStatusWindow(GtkWidget *box)
{
  StatusWindow *sw;
  //  GtkWidget *vscrollbar;
  GtkWidget *hbox;

  sw=g_new(StatusWindow,1);

  if(!sw) return NULL;

  sw->pipe[0]=sw->pipe[1]=-1;

  if(box)
    {
      sw->embedded=TRUE;
    }
  else
    {
      sw->embedded=FALSE;
    /* Create new window here */
    }

  hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

  /*  sw->term_widget=zvt_term_new_with_size(40,10);

  gtk_box_pack_start(GTK_BOX(hbox),sw->term_widget,FALSE,FALSE,0);
  gtk_widget_show(sw->term_widget);

  vscrollbar=gtk_vscrollbar_new(ZVT_TERM(sw->term_widget)->adjustment);
  gtk_box_pack_start(GTK_BOX(hbox),vscrollbar,FALSE,FALSE,0);
  gtk_widget_show(vscrollbar);*/

  sw->term_widget=vte_terminal_new();
  
  vte_terminal_set_encoding(VTE_TERMINAL(sw->term_widget),"UTF-8", 0);

  /*  vte_terminal_set_size(VTE_TERMINAL(sw->term_widget),40,10);*/

  gtk_box_pack_start(GTK_BOX(hbox),sw->term_widget,TRUE,TRUE,0);
  gtk_widget_show(sw->term_widget);

  //  vscrollbar = gtk_vscrollbar_new(VTE_TERMINAL(sw->term_widget)->adjustment);
  //  gtk_box_pack_start(GTK_BOX(hbox),vscrollbar,FALSE,FALSE,0);
  //  gtk_widget_show(vscrollbar);

  gtk_box_pack_start(GTK_BOX(box),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);

  return sw;
}

/* Write a line of output to a status window */
void StatusWindowWrite(StatusWindow *sw, char *msg)
{
  char *buf;
  gsize len;
  int pos=0;

  len=strlen(msg);

  buf=(char *)malloc((len*2)+1);

  while(*msg) {
    if(1) { //!(*msg & (1<<7))) {
      if(*msg=='\n') {
	buf[pos++]='\r';
        buf[pos++]='\n';
      } 
      else {
	buf[pos++]=*msg;
      }
    }
    msg++;
  }
  
  buf[pos]='\0';


  /*  zvt_term_feed((ZvtTerm *)sw->term_widget,buf,strlen(buf));*/

  vte_terminal_feed(VTE_TERMINAL(sw->term_widget),buf,strlen(buf));
  free(buf);
}

/* Return the output pipe fd for a status window, opening the pipe
   if necessary */
int GetStatusWindowPipe(StatusWindow *sw)
{
  if(sw->pipe[1]>0) return sw->pipe[1];

  pipe(sw->pipe);

  GIOChannel* channel = g_io_channel_unix_new(sw->pipe[0]);
  fcntl(sw->pipe[0], F_SETFL, O_NONBLOCK);

  // assert(g_io_add_watch(channel, G_IO_IN|G_IO_HUP, (GIOFunc)PipeCB, (gpointer)sw));
  assert(g_io_add_watch(channel, G_IO_IN, (GIOFunc)PipeCB, (gpointer)sw));
  return sw->pipe[1];
}

static int PipeCB(GIOChannel *channel, GIOCondition condition, gpointer data)
{
  //  char* buf;
  gsize nread;


  //  fprintf(stderr, "pipe callback with condition %d\n", condition);

  StatusWindow *sw;
  sw = (StatusWindow *)data;

  //  g_io_channel_read_to_end(channel, &buf, &nread, 0);
  //  fwrite(buf, 1, nread, stderr);
  //  free(buf);

  char buf[256];

  while(g_io_channel_read_chars(channel, buf, 256, &nread, 0) && (nread > 0))
    {
      fwrite(buf, 1, nread, stderr);
      /* StatusWindowWrite(sw,buf); */
    }
  return TRUE;
}
