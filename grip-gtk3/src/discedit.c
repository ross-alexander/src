/* discedit.c
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
#include <glib/gi18n.h>
#include "grip.h"
#include "cdplay.h"
#include "dialog.h"
#include "grip_id3.h"
#include "common.h"
#include "discedit.h"

static void SaveDiscInfo(GtkWidget *widget,gpointer data);
static void TitleEditChanged(GtkWidget *widget,gpointer data);
static void ArtistEditChanged(GtkWidget *widget,gpointer data);
static void YearEditChanged(GtkWidget *widget,gpointer data);
static void EditNextTrack(GtkWidget *widget,gpointer data);
static void ID3GenreChanged(GtkWidget *widget,gpointer data);
static void SeparateFields(char *buf,char *field1,char *field2,char *sep);
static void SplitTitleArtist(GtkWidget *widget,gpointer data);
static void SubmitEntryCB(GtkWidget *widget,gpointer data);
static void GetDiscDBGenre(GripInfo *ginfo);
static void DiscDBGenreChanged(GtkWidget *widget,gpointer data);

/* ----------------------------------------------------------------------
--
-- MakeEditBox
--
---------------------------------------------------------------------- */

GtkWidget *MakeEditBox(GripInfo *ginfo)
{
  GripGUI *uinfo;
  GtkWidget *vbox,*hbox;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *frame;
  GtkWidget *check;
  GtkWidget *entry;
  GtkAdjustment *adj;
  ID3Genre *id3_genre;
  gint id3_genre_count;
  int len;
  int dub_size;
  PangoLayout *layout;

  uinfo=&(ginfo->gui_info);

  frame = gtk_frame_new(NULL);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

  label = gtk_label_new(_("Disc title"));

  /* This should be the longest string in the track edit section */

  layout = gtk_widget_create_pango_layout(GTK_WIDGET(label), _("Track name"));
  pango_layout_get_size(layout,&len,NULL);
  len/=PANGO_SCALE;
  g_object_unref(layout);

  layout = gtk_widget_create_pango_layout(GTK_WIDGET(label), _("W"));
  pango_layout_get_size(layout,&dub_size,NULL);
  dub_size/=PANGO_SCALE;
  g_object_unref(layout);

  gtk_widget_set_size_request(label, len , 0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);


  uinfo->title_edit_entry=gtk_entry_new();
  g_signal_connect(G_OBJECT(uinfo->title_edit_entry),"changed", G_CALLBACK(TitleEditChanged),(gpointer)ginfo);
  gtk_editable_set_position(GTK_EDITABLE(uinfo->title_edit_entry),0);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->title_edit_entry,TRUE,TRUE,0);
  gtk_widget_show(uinfo->title_edit_entry);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);


  hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

  label=gtk_label_new(_("Disc artist"));
  gtk_widget_set_size_request(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  uinfo->artist_edit_entry=gtk_entry_new();
  g_signal_connect(G_OBJECT(uinfo->artist_edit_entry),"changed", G_CALLBACK(ArtistEditChanged),(gpointer)ginfo);
  gtk_editable_set_position(GTK_EDITABLE(uinfo->artist_edit_entry),0);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->artist_edit_entry,TRUE,TRUE,0);
  gtk_widget_show(uinfo->artist_edit_entry);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  /* --------------------
     Genre combo box
     -------------------- */
  
  hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

  label=gtk_label_new(_("ID3 genre:"));
  gtk_widget_set_size_request(label, len, 0);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE,0);
  gtk_widget_show(label);

  //  uinfo->id3_genre_combo = gtk_combo_box_text_new();

  GtkListStore *genre_liststore = gtk_list_store_new(2, G_TYPE_INT, G_TYPE_STRING);
  
  for(id3_genre_count=0; (id3_genre=ID3GenreByNum(id3_genre_count)); id3_genre_count++)
    {
      gtk_list_store_insert_with_values(genre_liststore, NULL, -1, 0, id3_genre->num, 1, id3_genre->name, -1);
    }

  uinfo->id3_genre_combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(genre_liststore));
  g_object_unref(genre_liststore);

  GtkCellRenderer *column = gtk_cell_renderer_text_new();
  gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(uinfo->id3_genre_combo), column, TRUE);

    /* column does not need to be g_object_unref()ed because it
     * is GInitiallyUnowned and the floating reference has been
     * passed to combo by the gtk_cell_layout_pack_start() call. */

  gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(uinfo->id3_genre_combo), column,
				 "text", 1,
				 NULL);
  
  gtk_combo_box_set_active(GTK_COMBO_BOX(uinfo->id3_genre_combo), 1);
  g_signal_connect(G_OBJECT(uinfo->id3_genre_combo), "changed", G_CALLBACK(ID3GenreChanged), (gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->id3_genre_combo,TRUE,TRUE,0);
  gtk_widget_show(uinfo->id3_genre_combo);

  SetID3Genre(ginfo, ginfo->ddata.data_id3genre);
  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);


  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

  label = gtk_label_new(_("Disc year"));
  gtk_widget_set_size_request(label, len, 0);
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_widget_show(label);
  adj = gtk_adjustment_new(0,0,9999,1.0,5.0,0);

  uinfo->year_spin_button = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 0.5, 0);
  g_signal_connect(G_OBJECT(uinfo->year_spin_button),"value_changed", G_CALLBACK(YearEditChanged),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->year_spin_button,TRUE,TRUE,0);
  gtk_widget_show(uinfo->year_spin_button);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

  label=gtk_label_new(_("Track name"));
  gtk_widget_set_size_request(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  uinfo->track_edit_entry = gtk_entry_new();
  g_signal_connect(G_OBJECT(uinfo->track_edit_entry),"changed", G_CALLBACK(TrackEditChanged),(gpointer)ginfo);
  g_signal_connect(G_OBJECT(uinfo->track_edit_entry),"activate", G_CALLBACK(EditNextTrack),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->track_edit_entry,TRUE,TRUE,0);
  gtk_widget_show(uinfo->track_edit_entry);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  uinfo->multi_artist_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,3);

  label=gtk_label_new(_("Track artist"));
  gtk_widget_set_size_request(label,len,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);

  uinfo->track_artist_edit_entry = gtk_entry_new();
  g_signal_connect(G_OBJECT(uinfo->track_artist_edit_entry), "changed", G_CALLBACK(TrackEditChanged),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox),uinfo->track_artist_edit_entry, TRUE,TRUE,0);
  gtk_widget_show(uinfo->track_artist_edit_entry);

  gtk_box_pack_start(GTK_BOX(uinfo->multi_artist_box),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);
  label = gtk_label_new(_("Split:"));
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_widget_show(label);

  button = gtk_button_new_with_label(_("Title/Artist"));
  //  gtk_object_set_user_data(GTK_OBJECT(button),(gpointer)0);
  //  gtk_signal_connect(GTK_OBJECT(button),"clicked", GTK_SIGNAL_FUNC(SplitTitleArtist),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  button = gtk_button_new_with_label(_("Artist/Title"));
  //  gtk_object_set_user_data(GTK_OBJECT(button),(gpointer)1);
  //  gtk_signal_connect(GTK_OBJECT(button),"clicked", GTK_SIGNAL_FUNC(SplitTitleArtist),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  gtk_widget_show(button);

  entry = MakeStrEntry(&uinfo->split_chars_entry,ginfo->title_split_chars, _("Split chars"),5,TRUE);
  gtk_widget_set_size_request(uinfo->split_chars_entry, 5*dub_size,0);
  gtk_box_pack_end(GTK_BOX(hbox), entry, FALSE, FALSE, 0);
  gtk_widget_show(entry);

  gtk_box_pack_start(GTK_BOX(uinfo->multi_artist_box), hbox, FALSE, FALSE, 2);
  gtk_widget_show(hbox);

  gtk_box_pack_start(GTK_BOX(vbox),uinfo->multi_artist_box, FALSE, FALSE, 0);

  if(ginfo->ddata.data_multi_artist)
    gtk_widget_show(uinfo->multi_artist_box);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

  check = MakeCheckButton(&uinfo->multi_artist_button, &(ginfo->ddata.data_multi_artist), _("Multi-artist"));
  g_signal_connect(G_OBJECT(uinfo->multi_artist_button),"clicked", G_CALLBACK(UpdateMultiArtist),(gpointer)ginfo);
  gtk_box_pack_start(GTK_BOX(hbox),check,TRUE,TRUE,0);
  gtk_widget_show(check);
  
  button = ImageButton(GTK_WIDGET(uinfo->appwin), uinfo->cdscan_image);
  gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(ScanDisc),(gpointer)ginfo);
  gtk_widget_show(button);
  
  button = ImageButton(GTK_WIDGET(uinfo->appwin),uinfo->save_image);
  //  gtk_widget_set_style(button,uinfo->style_dark_grey);
  g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(SaveDiscInfo),(gpointer)ginfo);
  //  gtk_tooltips_set_tip(MakeToolTip(),button, _("Save disc info"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  button = ImageButton(GTK_WIDGET(uinfo->appwin),uinfo->mail_image);
  //  gtk_widget_set_style(button,uinfo->style_dark_grey);
  //  gtk_signal_connect(GTK_OBJECT(button),"clicked", GTK_SIGNAL_FUNC(SubmitEntryCB),(gpointer)ginfo);
  //  gtk_tooltips_set_tip(MakeToolTip(),button, _("Submit disc info"),NULL);
  gtk_box_pack_start(GTK_BOX(hbox),button,FALSE,FALSE,0);
  gtk_widget_show(button);

  gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
  gtk_widget_show(hbox);

  gtk_container_add(GTK_CONTAINER(frame),vbox);
  gtk_widget_show(vbox);

  return frame;
}


void UpdateMultiArtist(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  GripGUI *uinfo;

  ginfo=(GripInfo *)data;
  uinfo=&(ginfo->gui_info);

  if(!ginfo->ddata.data_multi_artist)
    {
      gtk_widget_hide(uinfo->multi_artist_box);
      UpdateGTK();
    }
  else
    {
      gtk_widget_show(uinfo->multi_artist_box);
    }
}


void ToggleTrackEdit(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  GripGUI *uinfo;

  ginfo=(GripInfo *)data;
  uinfo=&(ginfo->gui_info);

  if(uinfo->track_edit_visible)
    {
      gtk_window_resize(GTK_WINDOW(uinfo->appwin), uinfo->win_width, uinfo->win_height);
      gtk_widget_hide(uinfo->track_edit_box);
      UpdateGTK();
    }
  else
    {
      if(uinfo->minimized) MinMax(NULL, (gpointer)ginfo);
      gtk_widget_show(uinfo->track_edit_box);

      gtk_window_resize(GTK_WINDOW(uinfo->appwin), uinfo->win_width, uinfo->win_height_edit);
    }
  uinfo->track_edit_visible=!uinfo->track_edit_visible;
}

void SetTitle(GripInfo *ginfo,char *title)
{
  g_signal_handlers_block_by_func(G_OBJECT(ginfo->gui_info.title_edit_entry),
                                  TitleEditChanged,(gpointer)ginfo);

  gtk_entry_set_text(GTK_ENTRY(ginfo->gui_info.title_edit_entry),title);
  gtk_editable_set_position(GTK_EDITABLE(ginfo->gui_info.title_edit_entry), 0);

  strcpy(ginfo->ddata.data_title,title);
  gtk_label_set_text(GTK_LABEL(ginfo->gui_info.disc_name_label),title);

  g_signal_handlers_unblock_by_func(G_OBJECT(ginfo->gui_info.title_edit_entry),
                                    TitleEditChanged,(gpointer)ginfo);
}

void SetArtist(GripInfo *ginfo,char *artist)
{
  g_signal_handlers_block_by_func(G_OBJECT(ginfo->gui_info.artist_edit_entry),
                                  ArtistEditChanged,(gpointer)ginfo);

  gtk_entry_set_text(GTK_ENTRY(ginfo->gui_info.artist_edit_entry),artist);
  gtk_editable_set_position(GTK_EDITABLE(ginfo->gui_info.artist_edit_entry),0);

  strcpy(ginfo->ddata.data_artist,artist);
  gtk_label_set_text(GTK_LABEL(ginfo->gui_info.disc_artist_label),artist);

  g_signal_handlers_unblock_by_func(G_OBJECT(ginfo->gui_info.artist_edit_entry),
                                    ArtistEditChanged,(gpointer)ginfo);
}

void SetYear(GripInfo *ginfo,int year)
{
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(ginfo->gui_info.year_spin_button),
  			    (gfloat)year);
}

void SetID3Genre(GripInfo *ginfo, int id3_genre)
{
  /*
  GtkTreeModel* model = GTK_TREE_MODEL(gtk_combo_box_get_model(GTK_COMBO_BOX(ginfo->gui_info.id3_genre_combo)));
  GtkTreeIter iter;
  gtk_tree_model_get_iter_first(model, &iter); 
  do {
    int id;
    char* name;
    gtk_tree_model_get(model, &iter, 0, &id, -1);
    gtk_tree_model_get(model, &iter, 1, &name, -1);
    printf("%d - %s\n", id, name);
  } while(gtk_tree_model_iter_next(model, &iter));
  
  //  GtkWidget *item;
  //  item = GTK_WIDGET(g_list_nth(ginfo->gui_info.id3_genre_item_list, ID3GenrePos(id3_genre))->data);
  */
  gtk_combo_box_set_active(GTK_COMBO_BOX(ginfo->gui_info.id3_genre_combo), ID3GenrePos(id3_genre));
}

static void SaveDiscInfo(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  if(ginfo->have_disc)
    {
      if(DiscDBWriteDiscData(&(ginfo->disc), &(ginfo->ddata), NULL, TRUE, FALSE, "utf-8") < 0)
      grip_app_warning(ginfo->gui_info.appwin, _("Error saving disc data."));
  }
  else
    {
      grip_app_warning(ginfo->gui_info.appwin, _("No disc present."));
    }
}

static void TitleEditChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  strcpy(ginfo->ddata.data_title,
         gtk_entry_get_text(GTK_ENTRY(ginfo->gui_info.title_edit_entry)));

  gtk_label_set_text(GTK_LABEL(ginfo->gui_info.disc_name_label),
                ginfo->ddata.data_title);
}

static void ArtistEditChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  strcpy(ginfo->ddata.data_artist,
         gtk_entry_get_text(GTK_ENTRY(ginfo->gui_info.artist_edit_entry)));

  gtk_label_set_text(GTK_LABEL(ginfo->gui_info.disc_artist_label),
                ginfo->ddata.data_artist);
}

static void YearEditChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  ginfo->ddata.data_year=
    gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ginfo->gui_info.
  						     year_spin_button));
}

void TrackEditChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  char newname[256];
  GtkTreeIter iter;
  gint i;

  ginfo=(GripInfo *)data;

  strcpy(ginfo->ddata.data_track[CURRENT_TRACK].track_name,
         gtk_entry_get_text(GTK_ENTRY(ginfo->gui_info.track_edit_entry)));
  
  strcpy(ginfo->ddata.data_track[CURRENT_TRACK].track_artist,
         gtk_entry_get_text(GTK_ENTRY(ginfo->gui_info.track_artist_edit_entry)));
  
  if(*ginfo->ddata.data_track[CURRENT_TRACK].track_artist)
    g_snprintf(newname,256,"%02d  %s (%s)",CURRENT_TRACK+1,
               ginfo->ddata.data_track[CURRENT_TRACK].track_name,
               ginfo->ddata.data_track[CURRENT_TRACK].track_artist);
  else
    g_snprintf(newname,256,"%02d  %s",CURRENT_TRACK+1,
               ginfo->ddata.data_track[CURRENT_TRACK].track_name);

  gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ginfo->gui_info.track_list_store),
                                &iter);
  for(i=0;i<CURRENT_TRACK;i++)
    gtk_tree_model_iter_next(GTK_TREE_MODEL(ginfo->gui_info.track_list_store),
                             &iter);
  gtk_list_store_set(ginfo->gui_info.track_list_store,&iter,
			 TRACKLIST_TRACK_COL,newname,-1);
  /*  gtk_clist_set_text(GTK_CLIST(ginfo->gui_info.trackclist),
      CURRENT_TRACK,0,newname);*/
}

static void EditNextTrack(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  NextTrack(ginfo);
  /*  gtk_editable_select_region(GTK_EDITABLE(track_edit_entry),0,-1);*/
  gtk_widget_grab_focus(GTK_WIDGET(ginfo->gui_info.track_edit_entry));
}

static void ID3GenreChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo = (GripInfo*)data;
  GtkComboBox *combo = GTK_COMBO_BOX(ginfo->gui_info.id3_genre_combo);
  GtkTreeModel* model = GTK_TREE_MODEL(gtk_combo_box_get_model(combo));
  GtkTreeIter iter;
  gtk_combo_box_get_active_iter (combo, &iter);
  int id;
  char* name;
  gtk_tree_model_get(model, &iter, 0, &id, 1, &name, -1);
  printf("%d - %s\n", id, name);
  
  ginfo->ddata.data_id3genre = id;
  ginfo->ddata.data_genre=ID32DiscDB(ginfo->ddata.data_id3genre);
}

static void SeparateFields(char *buf,char *field1,char *field2,char *sep)
{
  char *tmp;
  char spare[80];

  tmp=strtok(buf,sep);

  if(!tmp) return;

  strncpy(spare,g_strstrip(tmp),80);

  tmp=strtok(NULL,"");

  if(tmp) {
    strncpy(field2,g_strstrip(tmp),80);
  }
  else *field2='\0';

  strcpy(field1,spare);
}

static void SplitTitleArtist(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  int track;
  int mode;

  ginfo=(GripInfo *)data;
  mode = (long)g_object_get_qdata(G_OBJECT(widget), g_quark_from_string("Mode"));

  for(track=0;track<ginfo->disc.num_tracks;track++)
    {
      if(mode==0)
	SeparateFields(ginfo->ddata.data_track[track].track_name,
		       ginfo->ddata.data_track[track].track_name,
		       ginfo->ddata.data_track[track].track_artist,
		       ginfo->title_split_chars);
      else 
	SeparateFields(ginfo->ddata.data_track[track].track_name,
		       ginfo->ddata.data_track[track].track_artist,
		       ginfo->ddata.data_track[track].track_name,
		       ginfo->title_split_chars);
  }
  UpdateTracks(ginfo);
}

static void SubmitEntryCB(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;
  int len;

  ginfo=(GripInfo *)data;

  if(!ginfo->have_disc)
    {
      grip_app_warning(ginfo->gui_info.appwin, _("Cannot submit. No disc is present."));
      return;
    }

  if(!ginfo->ddata.data_genre)
    {
      /*    gnome_app_warning((GnomeApp *)ginfo->gui_info.appwin,
	    _("Submission requires a genre other than 'unknown'."));*/
      // GetDiscDBGenre(ginfo);
      return;
    }

  if(!*ginfo->ddata.data_title)
    {
      grip_app_warning(ginfo->gui_info.appwin, _("You must enter a disc title."));
      return;
    }

  if(!*ginfo->ddata.data_artist)
    {
      grip_app_warning(ginfo->gui_info.appwin, _("You must enter a disc artist."));
      return;
    }

  len = strlen(ginfo->discdb_submit_email);

  if(!strncasecmp(ginfo->discdb_submit_email+(len-9),".cddb.com",9))
    grip_app_ok_cancel_modal(ginfo->gui_info.appwin,
			     _("You are about to submit this disc information\n"
			       "to a commercial CDDB server, which will then\n"
			       "own the data that you submit. These servers make\n"
			       "a profit out of your effort. We suggest that you\n"
			       "support free servers instead.\n\nContinue?"),
			     G_CALLBACK(SubmitEntry),(gpointer)ginfo);
  else
    grip_app_ok_cancel_modal
      (ginfo->gui_info.appwin,
       _("You are about to submit this\ndisc information via email.\n\n"
	 "Continue?"),G_CALLBACK(SubmitEntry),(gpointer)ginfo);
}


/* Make the user pick a DiscDB genre on submit*/
static void GetDiscDBGenre(GripInfo *ginfo)
{
  GtkWidget *dialog;
  GtkWidget *label;
  GtkWidget *submit_button;
  GtkWidget *cancel_button;
  GtkWidget *hbox;
  GtkWidget *genre_combo;
  GtkWidget *item;
  int genre;

  dialog=gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(dialog),_("Genre selection"));

  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)),5);

  label=gtk_label_new(_("Submission requires a genre other than 'unknown'\n"
		      "Please select a DiscDB genre below"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)),label,TRUE,TRUE,0);
  gtk_widget_show(label);

  genre_combo = gtk_combo_box_new();
  //  gtk_entry_set_editable(GTK_ENTRY(GTK_COMBO_BOX(genre_combo)),FALSE);

  g_object_set(G_OBJECT(genre_combo), "editable", FALSE, NULL);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 3);

  label=gtk_label_new(_("DiscDB genre"));
  gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
  gtk_widget_show(label);
#ifdef Gnome2

  for(genre=0;genre<12;genre++)
    {
      item = gtk_list_item_new_with_label(DiscDBGenre(genre));
      g_object_set_qdata(G_OBJECT(item), g_quark_from_string("Genre"), (gpointer)genre);
    gtk_signal_connect(GTK_OBJECT(item), "select",
		       GTK_SIGNAL_FUNC(DiscDBGenreChanged),(gpointer)ginfo);
    gtk_container_add(GTK_CONTAINER(genre_combo),item);
    gtk_widget_show(item);
  }

  gtk_box_pack_start(GTK_BOX(hbox),genre_combo,TRUE,TRUE,0);
  gtk_widget_show(genre_combo);

  gtk_box_pack_start(GTK_VBOX(GTK_DIALOG(dialog)),hbox,TRUE,TRUE,0);
  gtk_widget_show(hbox);

  submit_button=gtk_button_new_with_label(_("Submit"));

  gtk_signal_connect(GTK_OBJECT(submit_button),"clicked",
		     (gpointer)SubmitEntryCB,(gpointer)ginfo);
  gtk_signal_connect_object(GTK_OBJECT(submit_button),"clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    GTK_OBJECT(dialog));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)),submit_button,
		     TRUE,TRUE,0);
  gtk_widget_show(submit_button);

  cancel_button=gtk_button_new_with_label(_("Cancel"));

  gtk_signal_connect_object(GTK_OBJECT(cancel_button),"clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy),
			    GTK_OBJECT(dialog));
  gtk_box_pack_start(GTK_VBOX(GTK_DIALOG(dialog)),cancel_button,
		     TRUE,TRUE,0);
  gtk_widget_show(cancel_button);

  gtk_widget_show(dialog);

  gtk_grab_add(dialog);
#endif
}

/* Set the DiscDB genre when a combo item is selected */
static void DiscDBGenreChanged(GtkWidget *widget,gpointer data)
{
  GripInfo *ginfo;

  ginfo=(GripInfo *)data;

  ginfo->ddata.data_genre = (long)g_object_get_qdata(G_OBJECT(widget), g_quark_from_string("Genre"));
}
