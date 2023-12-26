gst-launch-1.0 filesrc location=///locker/movies/FireWalkWithMe.vob ! mpegpsdemux name=demuxer \
demuxer. ! queue ! avdec_ac3 ! audioconvert ! audioresample ! pulsesink \
demuxer. ! queue ! avdec_mpeg2video ! autovideoconvert ! xvimagesink
