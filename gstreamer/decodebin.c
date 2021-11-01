#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <gst/gst.h>

#include <xcb/xcb.h>


#include "common.h"

/* ----------------------------------------------------------------------
--
-- bus_call
--
---------------------------------------------------------------------- */

gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop*)data;

  switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_STREAM_STATUS:
      break;
      
    case GST_MESSAGE_STATE_CHANGED:
      break;
      
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
      
    case GST_MESSAGE_ERROR:
      {
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

static void on_pad_added(GstElement *element, GstPad *pad, gpointer data)
{
  GstCaps *caps = gst_pad_query_caps(pad, 0);

  assert(caps);

  const char *name[gst_caps_get_size(caps)];
  const char *structure[gst_caps_get_size(caps)];

  for (int i = 0; i < gst_caps_get_size(caps); i++)
    {
      GstStructure *s = gst_caps_get_structure(caps, i);
      name[i] = gst_structure_get_name(s);
      structure[i] = gst_structure_to_string(s);
      printf("Pad added: caps %s : %s\n", name[i], structure[i]);
    }

  GstStructure *s = gst_caps_get_structure(caps, 0);
  GstPad *sinkpad;
  GstElement *pipe = (GstElement*)data;
  GstElement *bin = 0;
  GstElement *first = 0;

  if ((strncmp(name[0], "video/x-raw-rgb", 15) == 0) || (strncmp(name[0], "video/x-raw-yuv", 15) == 0) || (strncmp(name[0], "video/x-raw", strlen("video/x-raw")) == 0))
    {
      bin	= gst_bin_new(0);
      GstElement *conv	= gst_element_factory_make ("videoconvert",  "video-converter");
      assert(conv);

      GstElement *sink	= gst_element_factory_make ("xvimagesink", "video-output");
      g_object_set(G_OBJECT(sink), "force-aspect-ratio", 1, NULL);
      gst_bin_add_many(GST_BIN(bin), conv, sink, 0);
      gst_element_link_many (conv, sink, 0);
      first = conv;
    }
  else if ((strncmp(name[0], "audio/x-raw-int", 15) == 0) || (strncmp(name[0], "audio/x-raw-float", 17) == 0) || (strncmp(name[0], "audio/x-raw", 11) == 0))
    {
      printf("Adding raw audio\n");
      bin	= gst_bin_new(0);
      GstElement *conv	= gst_element_factory_make ("audioconvert",  "audio-converter");
      GstElement *sink	= gst_element_factory_make ("autoaudiosink", "audio-output");
      gst_bin_add_many(GST_BIN(bin), conv, sink, 0);
      gst_element_link_many(conv, sink, 0);
      first = conv;
    }

  else if (strncmp(name[0], "video/x-h264", 12) == 0)
    {
      bin	= gst_bin_new(0);
      GstElement *decode	= gst_element_factory_make ("ffdec_h264",  "video-decode");
      GstElement *conv	= gst_element_factory_make ("ffmpegcolorspace",  "video-converter");
      GstElement *sink	= gst_element_factory_make ("xvimagesink", "video-output");
      g_object_set(G_OBJECT(sink), "force-aspect-ratio", 1, NULL);
      gst_bin_add_many(GST_BIN(bin), decode, conv, sink, 0);
      gst_element_link_many (decode, conv, sink, 0);
      first = decode;
    }
  else if (strncmp(name[0], "video/mpeg", 11) == 0)
    {
      bin	= gst_bin_new(0);
      GstElement *decode	= gst_element_factory_make ("ffdec_mpeg2video",  "video-decode");
      GstElement *conv	= gst_element_factory_make ("ffmpegcolorspace",  "video-converter");
      GstElement *sink	= gst_element_factory_make ("xvimagesink", "video-output");
      g_object_set(G_OBJECT(sink), "force-aspect-ratio", 1, NULL);
      gst_bin_add_many(GST_BIN(bin), decode, conv, sink, 0);
      gst_element_link_many (decode, conv, sink, 0);
      first = decode;
    }
  else if (strncmp(name[0], "video/x-theora", sizeof("video/x-theora")) == 0)
    {
      bin	= gst_bin_new(0);
      GstElement *decode	= gst_element_factory_make ("theoradec",  "video-decode");
      GstElement *conv	= gst_element_factory_make ("ffmpegcolorspace",  "video-converter");
      GstElement *sink	= gst_element_factory_make ("xvimagesink", "video-output");
      g_object_set(G_OBJECT(sink), "force-aspect-ratio", 1, NULL);
      gst_bin_add_many(GST_BIN(bin), decode, conv, sink, 0);
      gst_element_link_many (decode, conv, sink, 0);
      first = decode;
    }
  else if (strncmp(name[0], "audio/mpeg", 10) == 0)
    {
      int version;
      GstElement *decode = 0;
      gst_structure_get_int(s, "mpegversion", &version);
      bin	= gst_bin_new(0);
      printf("Pad added: audio/mpeg version=%d\n", version);
      if (version == 1)
	decode	= gst_element_factory_make ("ffdec_mp3",  "audio-decode");
      else
	decode	= gst_element_factory_make ("faad",  "audio-decode");
      GstElement *conv	= gst_element_factory_make ("audioconvert",  "audio-converter");
      GstElement *sink	= gst_element_factory_make ("autoaudiosink", "audio-output");
      gst_bin_add_many(GST_BIN(bin), decode, conv, sink, 0);
      gst_element_link_many(decode, conv, sink, 0);
      first = decode;
    }
  else if (strncmp(name[0], "audio/x-private1-ac3", 20) == 0)
    {
      int version;
      bin	= gst_bin_new(0);
      GstElement *decode = gst_element_factory_make ("a52dec",  "audio-decode");
      g_object_set(G_OBJECT(decode), "mode", 2, NULL);
      GstElement *conv	= gst_element_factory_make ("audioconvert",  "audio-converter");
      GstElement *sink	= gst_element_factory_make ("autoaudiosink", "audio-output");
      gst_bin_add_many(GST_BIN(bin), decode, conv, sink, 0);
      gst_element_link_many(decode, conv, sink, 0);
      first = decode;
    }
  else if (strncmp(name[0], "audio/x-vorbis", sizeof("audio/x-vorbis")) == 0)
    {
      int version;
      bin	= gst_bin_new(0);
      GstElement * decode = gst_element_factory_make ("vorbisdec",  "audio-decode");
      GstElement * conv	= gst_element_factory_make("audioconvert",  "audio-converter");
      GstElement * sink	= gst_element_factory_make("alsasink", "audio-output");
      gst_bin_add_many(GST_BIN(bin), decode, conv, sink, 0);
      gst_element_link_many(decode, conv, sink, 0);
      first = decode;
    }

  if (bin && first)
    {
      gst_bin_add(GST_BIN(pipe), bin);
      GstPad *sinkpad = gst_element_get_static_pad(first, "sink");
      assert(sinkpad);
      GstPad *ghost = gst_ghost_pad_new("sink", sinkpad);
      gst_element_add_pad(bin, ghost); 
      GstPadLinkReturn res = gst_pad_link(pad, ghost);
      if (res == 0)
	{
	  gst_element_set_state(bin, GST_STATE_PAUSED);
	  g_print("Pad-added: linked\n");
	}
    }
}

/* ----------------------------------------------------------------------
--
-- auto_cont
--
---------------------------------------------------------------------- */
int auto_cont(GstElement *bin, GstPad *pad, GstCaps *caps, gpointer user)
{
  GstStructure *s = gst_caps_get_structure(caps, 0);
  const char *name = gst_structure_get_name(s);
  printf("Autoplug-continue: caps %s\n", name);
  return 1;
}


/* ----------------------------------------------------------------------
--
-- auto_sel
--
---------------------------------------------------------------------- */
int auto_sel(GstElement *bin, GstPad *pad, GstCaps *caps, GstElementFactory *factory, gpointer user)
{
  GstStructure *s = gst_caps_get_structure(caps, 0);
  const char *name = gst_structure_get_name(s);
  const char *klass = gst_element_factory_get_klass(factory);
  printf("Autoplug-select: factory %s class %s caps %s\n", gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory)), klass, name);
  /*
  if (strncmp(klass, "Codec/Demuxer", 11) == 0)
    return 0;
  */
  return 0; // GST_AUTOPLUG_SELECT_TRY
}

/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  gst_init(&argc, &argv);

  char *src = 0;
  if (argc > 1)
    src = argv[1];
  else
    src = "BRFC-001.mp4";

  GMainLoop* loop	= g_main_loop_new(0, 0);
  GstElement *pipe	= gst_pipeline_new("av-player");

  GstElement *source	= gst_element_factory_make ("filesrc", "file-source");
  GstElement *decodebin	= gst_element_factory_make ("decodebin", 0);

  assert(decodebin);
  assert(source);

  g_object_set(G_OBJECT(source), "location", src, 0);
  g_signal_connect(decodebin, "pad-added", G_CALLBACK(on_pad_added), pipe);
  g_signal_connect(decodebin, "autoplug-continue", G_CALLBACK(auto_cont), pipe);
  g_signal_connect(decodebin, "autoplug-select", G_CALLBACK(auto_sel), pipe);

  gst_bin_add_many(GST_BIN(pipe), source, decodebin, 0);
  gst_element_link_many(source, decodebin, 0);

  GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipe));
  gst_bus_set_sync_handler (bus, (GstBusSyncHandler) create_window_gst, 0, 0);
  gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);

  GstStateChangeReturn state = gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_PLAYING);
  g_main_loop_run(loop);
}
