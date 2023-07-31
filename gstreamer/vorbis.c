#include <gst/gst.h>
#include <glib.h>
#include <assert.h>

/* ----------------------------------------------------------------------
--
-- Fixed pipeline for vorbis/ogg audio
--
-- 2023-03-26:
--   Rename from hello to vorbis and merge fixed

-- 2021-11-01:
--
-- Fixed format pipeline based on vorbis stream inside an
-- ogg container.
--
---------------------------------------------------------------------- */


static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *)data;
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

static void fixed_on_pad_added (GstElement *element, GstPad *pad, gpointer data)
{
  GstPad *sinkpad;
  GstElement *decoder = (GstElement *) data;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");
  
  sinkpad = gst_element_get_static_pad (decoder, "sink");
  gst_pad_link (pad, sinkpad);
  gst_object_unref (sinkpad);
}

/* ----------------------------------------------------------------------
--
-- fixed_pipeline
--
---------------------------------------------------------------------- */

int fixed_pipeline(const char *path)
{
  GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink;
  GstBus *bus;
  GMainLoop *loop;

  /* Create gstreamer elements */
  
  pipeline = gst_pipeline_new ("audio-player");
  source   = gst_element_factory_make ("filesrc",       "file-source");
  demuxer  = gst_element_factory_make ("oggdemux",      "demuxer");
  decoder  = gst_element_factory_make ("vorbisdec",     "bin-decoder");
  conv     = gst_element_factory_make ("audioconvert",  "converter");
  sink     = gst_element_factory_make ("autoaudiosink", "output");

  assert(source);
  assert(demuxer);
  assert(decoder);
  assert(conv);
  assert(sink);

  if (!pipeline || !source || !demuxer || !decoder || !conv || !sink)
    {
      g_printerr ("One element could not be created. Exiting.\n");
      return -1;
    }

  /* Set up the pipeline */

  /* we set the input filename to the source element */
  g_object_set (G_OBJECT(source), "location", path, NULL);
  
  /* we add a message handler */

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  /* file-source | ogg-demuxer | vorbis-decoder | converter | alsa-output */

  gst_bin_add_many (GST_BIN(pipeline), source, demuxer, decoder, conv, sink, NULL);

  /* we link the elements together */
  /* file-source -> ogg-demuxer ~> vorbis-decoder -> converter -> alsa-output */

  gst_element_link (source, demuxer);
  gst_element_link_many (decoder, conv, sink, NULL);
  g_signal_connect (demuxer, "pad-added", G_CALLBACK (fixed_on_pad_added), decoder);

  /* note that the demuxer will be linked to the decoder dynamically.
     The reason is that Ogg may contain various streams (for example
     audio and video). The source pad(s) will be created at run time,
     by the demuxer when it detects the amount and nature of streams.
     Therefore we connect a callback function which will be executed
     when the "pad-added" is emitted.*/

  /* Set the pipeline to "playing" state*/

  g_print ("Now playing: %s\n", path);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* Iterate */
  loop = g_main_loop_new (0, 0);

  g_print ("Running...\n");
  g_main_loop_run (loop);

  /* Out of the main loop, clean up nicely */

  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
}
  

/* ----------------------------------------------------------------------
--
-- fake_pipeline
--
---------------------------------------------------------------------- */

static void fake_on_pad_added (GstElement *element, GstPad *pad, gpointer data)
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
  GstElement * sink	= gst_element_factory_make("autoaudiosink", "audiosink");
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



int fake_pipeline(const char *path)
{
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

  g_signal_connect (demux, "pad-added", G_CALLBACK (fake_on_pad_added), pipe);

  g_object_set(G_OBJECT(source), "location", path, 0);

  GstBus * bus = gst_pipeline_get_bus(GST_PIPELINE(pipe));
  gst_bus_add_watch(bus, bus_call, loop);
  gst_object_unref(bus);

  GstStateChangeReturn state = gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_PLAYING);
  g_main_loop_run(loop);
}


/* ----------------------------------------------------------------------
--
-- main
--
---------------------------------------------------------------------- */

int main (int argc, char *argv[])
{
  
  /* Initialisation */
  gst_init (&argc, &argv);

  /* Check input arguments */
  if (argc != 2)
    {
      g_printerr ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
      return -1;
    }

  fake_pipeline(argv[1]);

  return 0;
}
