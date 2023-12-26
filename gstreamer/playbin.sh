#!/bin/sh


# gst-launch  filesrc  location=fwme.vob  \!  mpegpsdemux name=demuxer demuxer. \! queue \! fakesink demuxer. \! queue \! a52dec \! audioconvert \! audioresample \! pulsesink

# gst-launch  filesrc  location=fwme.vob  \!  mpegpsdemux name=demuxer demuxer. \! queue \! mpeg2dec \! xvimagesink demuxer. \! queue \! a52dec \! audioconvert \! audioresample \! pulsesink

# gst-launch filesrc location=fwme.vob ! decodebin name=decoder \
#    decoder. ! ffmpegcolorspace ! xvimagesink \
#    decoder. ! audioconvert ! audioresample ! pulsesink

gst-launch-1.0 playbin uri=file:///home/ralexand/src/gstreamer/bladerunner-intro.mp4
