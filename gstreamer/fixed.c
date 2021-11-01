#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <gst/gst.h>

/* ----------------------------------------------------------------------
--
-- simple fixed pipeline for ogg decoding, working with gstreamer 1.0
--
---------------------------------------------------------------------- */


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
      g_print("Stream status changed\n");
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
      g_main_loop_quit(loop);
      break;
      
    case GST_MESSAGE_ERROR:
      {
	gchar  *debug;
	GError *error;
	
	gst_message_parse_error(msg, &error, &debug);
	g_free(debug);
	
	g_printerr("Error: %s\n", error->message);
	g_error_free(error);
	g_main_loop_quit (loop);
	break;
      }
    default:
      break;
    }
  return TRUE;
}

static void on_pad_added (GstElement *element, GstPad *pad, gpointer data)
{
  struct decoders *decoders = (struct decoders*)data;
  GstCaps *caps = gst_pad_get_current_caps(pad);
  GstElement *pipe = (GstElement*)data;

  GstStructure *s = gst_caps_get_structure(caps, 0);
  const char *name;

  name = gst_structure_get_name(s);

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created %s\n", name);


  GstElement *bin = gst_bin_new(0);
  GstElement * queue	= gst_element_factory_make("queue", "queue");
  GstElement * decode	= gst_element_factory_make("vorbisdec", "decode");
  GstElement * convert	= gst_element_factory_make("audioconvert", "convert");
  GstElement * sink	= gst_element_factory_make("alsasink", "alsasink");
  gst_bin_add_many(GST_BIN(bin), queue, decode, convert, sink, 0);
  gst_element_link_many (queue, decode, convert, sink, NULL);
  gst_bin_add_many(GST_BIN(pipe), bin, 0);

  assert(bin);
  assert(queue);
  assert(decode);
  assert(convert);
  assert(sink);

  GstPad *audiopad = gst_element_get_static_pad(queue, "sink");

  assert(audiopad);

  GstPad *ghost = gst_ghost_pad_new ("sink", audiopad);
  assert(ghost);

  gst_element_add_pad(bin, ghost); 
  GstPadLinkReturn res = gst_pad_link(pad, ghost);
  g_print ("res : %d\n", res);
  if (res == 0)
    {
      gst_element_set_state(bin, GST_STATE_PAUSED);
      g_print("Pad-added: linked\n");
    }
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
    {
      fprintf(stdout, "%s: [filename.ogg]\n", argv[0]);
      exit(0);
    }

  GMainLoop * loop	= g_main_loop_new(0, 0);
  GstElement * pipe	= gst_pipeline_new("av-player");
  GstElement * source	= gst_element_factory_make("filesrc", "file-source");

  GstElement * fakesink	= gst_element_factory_make("fakesink", "fakesink");
  GstElement * demux	= gst_element_factory_make("oggdemux", "demux");

  assert(pipe);
  assert(source);
  assert(demux);

  gst_bin_add_many(GST_BIN(pipe), source, demux, 0);
  //  gst_bin_add_many(GST_BIN(pipe), source, fakesink, 0);

  gst_element_link_many(source, demux, 0);
  //  gst_element_link_many(demux, fakesink, 0);

  g_signal_connect (demux, "pad-added", G_CALLBACK (on_pad_added), pipe);

  g_object_set(G_OBJECT(source), "location", src, 0);

  GstBus * bus = gst_pipeline_get_bus(GST_PIPELINE(pipe));
  gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);

  GstStateChangeReturn state = gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_PLAYING);
  g_main_loop_run(loop);
  exit(0);
}
