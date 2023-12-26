#!/bin/sh

gst-launch playbin2 uri=file://$PWD/$1 video-sink="vdpauvideopostprocess ! vdpausink"
