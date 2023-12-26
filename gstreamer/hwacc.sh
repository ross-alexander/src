ffmpeg -vaapi_device /dev/dri/renderD128 -i input.mpg -vf 'format=nv12,hwupload' -c:v h264_vaapi -y output.mp4
