#include <string.h>
#include <gst/gst.h>


/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GstElement *pipeline;
  GstElement *source;
  GstElement *typefinder;
  GstElement *demux;
  GstElement *decoder;
  GstElement *convert;
  GstElement *sink;
  
} CustomData;


static void
pad_added_handler (GstElement *element,
              GstPad     *pad,
              CustomData    *data)
{
  GstPad *sinkpad;

  /* We can now link this pad with the vorbis-decoder sink pad */
  g_print ("Dynamic pad created, linking demuxer/decoder\n");

  /* Get the sink pad of next element */
  sinkpad = gst_element_get_static_pad (data->decoder, "sink");

  /*link the source and sink pads */
  gst_pad_link (pad,sinkpad);

  gst_object_unref (sinkpad);
  gst_element_set_state (data->decoder, GST_STATE_PLAYING);
}

static void
cb_typefound (GstElement *typefind,
	      guint       probability,
	      GstCaps    *caps,
	      CustomData    *data)
{
  
  gchar *type;
  
  type = gst_caps_to_string (caps);
  g_print ("Media type %s found, probability %d%%  : %d\n", type, probability,strcmp(type,"video/webm"));
 
 
  if(!strcmp(type,"video/webm"))
  {
    g_print("Recognized video/webm video stream \n");
    data->demux=gst_element_factory_make ("matroskademux", "demux");
    data->decoder=gst_element_factory_make ("vp8dec", "decoder");
    
    gst_bin_add(GST_BIN (data->pipeline),data->demux);
    gst_bin_add(GST_BIN (data->pipeline),data->decoder);
    
    if (gst_element_link (data->typefinder, data->demux) !=TRUE || gst_element_link_many (data->decoder, data->convert,NULL) !=TRUE)
      {
	g_printerr ("Elements could not be linked.\n");
	gst_object_unref (data->pipeline); 
	return ;
      }
    /* Connect to the pad-added signal */
    g_signal_connect (data->demux, "pad-added", G_CALLBACK (pad_added_handler), data);
    gst_element_set_state (data->demux, GST_STATE_PLAYING);
  }
  g_free (type);
}

   
int main(int argc, char *argv[]) {
  CustomData data;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  gboolean terminate = FALSE;
   
  /* Initialize GStreamer */
  gst_init (&argc, &argv);

 /* Create the elements */
  data.source = gst_element_make_from_uri (GST_URI_SRC,"file:/home/ralexand/source/gstreamer/big-buck-bunny_trailer.webm" ,"source", NULL);

  g_assert(data.source != NULL);
  
  /* create the typefind element */
  data.typefinder = gst_element_factory_make ("typefind", "typefind");
  g_assert (data.typefinder != NULL);

  g_print("GStreamer Source Type %s \n",GST_ELEMENT_NAME (data.source));

  data.sink = gst_element_factory_make ("autovideosink" ,"sink");
  data.convert = gst_element_factory_make ("videoconvert" ,"filter");


/* Create the empty pipeline */
  data.pipeline = gst_pipeline_new ("test-pipeline");
  if (!data.pipeline) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

/* Adding Elements to List */
  gst_bin_add_many (GST_BIN (data.pipeline),data.source, data.typefinder, data.convert, data.sink,NULL);
  
  if (gst_element_link (data.source, data.typefinder) != TRUE || gst_element_link (data.convert, data.sink) != TRUE)
    {
      g_printerr ("Elements could not be linked.\n");
      gst_object_unref (data.pipeline);
      return -1;
    }

/* Connect to the pad-added signal */
  g_signal_connect (data.typefinder, "have-type", G_CALLBACK (cb_typefound), &data);


/* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE)
    {
      g_printerr ("Unable to set the pipeline to the playing state.\n");
      gst_object_unref (data.pipeline);
      return -1;
    }



 /* Wait until error or EOS */
  bus = gst_element_get_bus (data.pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
   
/* Parse message */
if (msg != NULL) {
  GError *err;
  gchar *debug_info;
   
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_ERROR:
      gst_message_parse_error (msg, &err, &debug_info);
      g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
      g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
      g_clear_error (&err);
      g_free (debug_info);
      break;
    case GST_MESSAGE_EOS:
      g_print ("End-Of-Stream reached.\n");
      break;
    default:
      /* We should not reach here because we only asked for ERRORs and EOS */
      g_printerr ("Unexpected message received.\n");
      break;
  }
  gst_message_unref (msg);
}
   
  /* Free resources */
  if (msg != NULL)
    gst_message_unref (msg);
  gst_object_unref (bus);
  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}
