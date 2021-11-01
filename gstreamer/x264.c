#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xfixes.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_renderutil.h>

#include <gst/gst.h>
#include <gst/video/videooverlay.h>

#include "common.h"

struct decoders_t {
  GMainLoop *loop;
  GstElement *pipe;
  GstElement *typefind;
  GstElement *queue;
  GstElement *src;
};


static gboolean idle_exit_loop (gpointer data)
{
  g_main_loop_quit ((GMainLoop *) data);
  
  /* once */
  return FALSE;
}


/* ----------------------------------------------------------------------
--
-- bus_call
--
---------------------------------------------------------------------- */

gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *)data;

    g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME(msg));
  
  switch (GST_MESSAGE_TYPE(msg)) {
  case GST_MESSAGE_STREAM_STATUS:
    break;

  case GST_MESSAGE_STATE_CHANGED:
    {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);
      g_print ("%s state set to %s\n", gst_object_get_name(GST_MESSAGE_SRC(msg)), gst_element_state_get_name (new_state));
      break;
    }
  case GST_MESSAGE_EOS:
    g_print ("End of stream\n");
    g_main_loop_quit (loop);
    break;
    
  case GST_MESSAGE_ERROR: {
    gchar  *debug;
    GError *error;
    
    gst_message_parse_error (msg, &error, &debug);
    g_free (debug);
    
    g_printerr ("Error: %s\n", error->message);
    g_error_free (error);
    
    g_main_loop_quit (loop);
    break;
  }
  default:
    break;
  }
  return TRUE;
}

/* ----------------------------------------------------------------------
--
-- on_pad_added
--
---------------------------------------------------------------------- */

static void on_pad_added (GstElement *element, GstPad *pad, gpointer data)
{
  struct decoders_t *decoders = (struct decoders_t*)data;
  GstCaps *caps = gst_pad_query_caps(pad, 0);
  GstElement *pipe = decoders->pipe;

  GstStructure *s;
  const char *name;
  for (int i = 0; i < gst_caps_get_size(caps); i++)
    {
      s = gst_caps_get_structure(caps, i);
      printf("** %s %d\n", gst_structure_get_name(s), gst_structure_n_fields(s));
      for (int j = 0; j < gst_structure_n_fields(s); j++)
	{
	  printf("++ %s\n", gst_structure_nth_field_name(s, j));
	}
    }
  s = gst_caps_get_structure(caps, 0);
  name = gst_structure_get_name(s);
  
  /* We can now link this pad with the vorbis-decoder sink pad */

  g_print ("Dynamic pad created, linking demuxer/decoder %s\n", name);

  GstPad *sinkpad;

  if (strncmp(name, "video", 5) == 0)
    {
      const char *type = name + 6;
      printf("video %s found\n", type);
      GstElement *decode = 0;
      if (strcmp(type, "mpeg") == 0)
	{
	  int version;
	  gst_structure_get_int(s, "mpegversion", &version);
	  printf("mpegversion=%d\n", version);
	  if (version == 2)
	    decode = gst_element_factory_make("avdec_mpeg2video", "decode");
	  if (version == 4)
	    decode = gst_element_factory_make("avdev_mpeg4", "decode");
	}
      if (strcmp(type, "x-h264") == 0)
	{
	  decode = gst_element_factory_make("vaapih264dec", "decode");
	  //	  decode = gst_element_factory_make("avdec_h264", "decode");
	}
      if (decode)
	{
	  GstElement* bin      = gst_bin_new("video-bin");
	  GstElement* queue    = gst_element_factory_make("queue", "video-queue");
	  GstElement* convert  = gst_element_factory_make ("videoconvert",  "video-converter");
	  GstElement* sink     = gst_element_factory_make ("xvimagesink", "video-output");
	  g_object_set (G_OBJECT(sink), "force-aspect-ratio", 1, NULL);
	  
	  gst_bin_add_many(GST_BIN(bin), queue, decode, convert, sink, 0);
	  gst_element_link_many (queue, decode, convert, sink, 0);
	  gst_bin_add(GST_BIN(pipe), bin);
	  
	  GstPad *videopad = gst_element_get_static_pad (queue, "sink");
	  GstPad *ghost = gst_ghost_pad_new ("sink", videopad);
	  gst_object_unref (videopad);
	  gst_element_add_pad (bin, ghost); 
	  GstPadLinkReturn res = gst_pad_link(pad, ghost);
	  g_print ("res : %d\n", res);
	  
	  //      sinkpad = gst_element_get_static_pad(vbin, "sink");
	  //      gst_pad_link (pad, sinkpad);
	  //      gst_object_unref (sinkpad);
	  
	  gst_element_set_state(bin, GST_STATE_PAUSED);
	  g_print("Linked video\n");
	}
    }
  
  /* --------------------
     Audio
     -------------------- */
  
  if (strncmp(name, "audio", strlen("audio")) == 0)
    {
      const char *type = name + 6;
      printf("audio %s found\n", type);
      GstElement *bin = gst_bin_new("audio-bin");
      GstElement *queue = gst_element_factory_make ("queue", "audio-queue");
      GstElement *decode;
      if (strcmp(type, "x-vorbis") == 0)
	{
	  decode = gst_element_factory_make("vorbisdec", "decode");
	}
      if (strcmp(type, "mpeg") == 0)
	{
	  int version;
	  gst_structure_get_int(s, "mpegversion", &version);
	  printf("mpegversion=%d\n", version);
	  decode = gst_element_factory_make("avdec_aac", "decode");
	}
      if (strcmp(type, "x-ac3") == 0)
	{
	  decode = gst_element_factory_make("avdec_ac3", "decode");
	}
      if (decode)
	{
	  GstElement *convert = gst_element_factory_make("audioconvert", "convert");
	  GstElement *sink = gst_element_factory_make("alsasink", "alsasink");
	  
	  assert(convert);
	  assert(sink);
	  gst_bin_add_many(GST_BIN(bin), queue, decode, convert, sink, 0);
	  gst_element_link_many (queue, decode, convert, sink, 0);
	  gst_bin_add(GST_BIN(pipe), bin);
	  GstPad *audiopad = gst_element_get_static_pad (queue, "sink");
	  GstPad *ghost = gst_ghost_pad_new ("sink", audiopad);
	  gst_object_unref (audiopad);
	  gst_element_add_pad(bin, ghost); 
	  GstPadLinkReturn res = gst_pad_link(pad, ghost);
	  g_print ("res : %d\n", res);
	  
	  
	  //      sinkpad = gst_element_get_static_pad(vbin, "sink");
	  //      gst_pad_link (pad, sinkpad);
	  //      gst_object_unref (sinkpad);
	  gst_element_set_state(bin, GST_STATE_PAUSED);
	  g_print("Linked audio\n");
	}
    }
}

/* ----------------------------------------------------------------------
--
-- cb_typefound
--
---------------------------------------------------------------------- */

static void cb_typefound (GstElement *typefind,
			  guint       probability,
			  GstCaps    *caps,
			  gpointer    data)
{
  struct decoders_t *decoders =  (struct decoders_t*)data;
  assert(typefind == decoders->typefind);

  gchar* type = gst_caps_to_string(caps);
  g_print ("Media type %s found, probability %d%%\n", type, probability);

  GstPad *typefind_src = gst_element_get_static_pad(typefind, "src");
  GstPad *typefind_sink = gst_element_get_static_pad(typefind, "sink");

  GST_PAD_STREAM_LOCK(typefind_sink);
  gst_element_set_state (decoders->pipe, GST_STATE_PAUSED);
  
  GstElement *demux = 0;
  if (strcmp(type, "audio/ogg") == 0)
    demux = gst_element_factory_make("oggdemux", "demux");
  if (strcmp(type, "audio/x-m4a") == 0)
    demux = gst_element_factory_make("qtdemux", "demux");
  if (strcmp(type, "video/webm") == 0)
    demux = gst_element_factory_make("webmmux", "demux");
  if (strncmp(type, "video/quicktime", strlen("video/quicktime")) == 0)
    demux = gst_element_factory_make("qtdemux", "demux");

  if (demux)
    {
      GstPad *sink_pad = gst_element_get_static_pad(demux, "sink");
      gst_bin_add(GST_BIN(decoders->pipe), demux);
      g_signal_connect (demux, "pad-added", G_CALLBACK(on_pad_added), decoders);
      gst_pad_link(typefind_src, sink_pad);
      printf("Demux added\n");
    }
  else
    {
      printf("No demux found\n");
      g_idle_add (idle_exit_loop, decoders->loop);
    }
  g_free(type);
  GST_PAD_STREAM_UNLOCK(typefind_sink);
  gst_element_set_state (decoders->pipe, GST_STATE_PLAYING);
  gst_element_set_state (decoders->typefind, GST_STATE_PLAYING);
  gst_element_set_state (demux, GST_STATE_PLAYING);
  return;
  
  GST_PAD_STREAM_LOCK(typefind_sink);

  if(strncmp(type, "video/quicktime", strlen("video/quicktime")) == 0)
    {
      GstElement *demux = gst_element_factory_make("qtdemux", "demux");
      GstPad *sink_pad = gst_element_get_static_pad(demux, "sink");
      gst_bin_add(GST_BIN(decoders->pipe), demux);

      gst_pad_link(typefind_src, sink_pad);

#ifdef UseGhost
      GstPad *ghost = gst_ghost_pad_new_no_target("src", GST_PAD_SRC);

      gst_ghost_pad_set_target(GST_GHOST_PAD(ghost), src_pad);

      GstPadLinkReturn res = gst_pad_link(GST_PAD(ghost), sink_pad);
      g_print ("res : %d\n", res);
#endif

      g_signal_connect(demux, "pad-added", G_CALLBACK (on_pad_added), &decoders);
      printf("Demuxer added\n");
    }

  if(strncmp(type, "audio/ogg", strlen("audio/ogg")) == 0)
    {
      GstElement *demux = gst_element_factory_make("oggdemux", "demux");
      gst_bin_add_many(GST_BIN(decoders->pipe), demux, 0);

      GstPad *demux_sink = gst_element_get_static_pad(demux, "sink");

      g_signal_connect (demux, "pad-added", G_CALLBACK (on_pad_added), decoders);

      gst_pad_link(typefind_src, demux_sink);
      gst_object_unref(demux_sink);
    }
  
  g_free (type);
  GST_PAD_STREAM_UNLOCK(typefind_sink);
  gst_object_unref(typefind_src);

  /* since we connect to a signal in the pipeline thread context, we need
   * to set an idle handler to exit the main loop in the mainloop context.
   * Normally, your app should not need to worry about such things. */
}



/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
 /* --------------------
     Start GStreamer details
     -------------------- */

  GMainLoop *loop;
  GstElement *pipe, *source, *demuxer;
  GstBus *bus;

  int screenNum;
  int vdpau = 0;

  xcb_connection_t *connection = xcb_connect(NULL, &screenNum);

  gst_init (&argc, &argv);

  char *src = 0;

  if (argc == 1)
    src = "BRFC-001.mkv";
  else
    src = argv[1];

  /* --------------------
     Create basic elements
     -------------------- */

  loop      = g_main_loop_new (NULL, FALSE);
  pipe	    = gst_pipeline_new ("av-player");
  source    = gst_element_factory_make ("filesrc", "file-source");

  GstElement *typefind = gst_element_factory_make("typefind", "typefind");
  GstElement *fakesink = gst_element_factory_make("fakesink", "fakesink");

  //  GstElement *queue = gst_element_factory_make("queue", "queue");

  //  demuxer   = gst_element_factory_make ("qtdemux", "qt-demuxer");
  // demuxer   = gst_element_factory_make ("mpegpsdemux", "demuxer");

  assert(pipe);
  assert(source);
  assert(typefind);

  gst_bin_add_many(GST_BIN(pipe), source, typefind, 0); // fakesink, 0);
  gst_element_link_many(source, typefind, 0);

  struct decoders_t decoders;
  decoders.loop = loop;
  decoders.pipe = pipe;
  decoders.src = source;
  decoders.typefind = typefind;
  //  decoders.queue = queue;

  g_signal_connect(typefind, "have-type", G_CALLBACK (cb_typefound), &decoders);
  g_object_set (G_OBJECT(source), "location", src, NULL);

  bus = gst_pipeline_get_bus(GST_PIPELINE(pipe));
  gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window_gst, connection, 0);
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  GstStateChangeReturn gessresult0 = gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_PLAYING);

  g_main_loop_run(loop);
  g_print("Loop terminates\n");
}
