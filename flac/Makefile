.SUFFIXES: .ogg .fla

%.ogg	: %.fla
	ffmpeg  -i $< -acodec libopus -ab 192k -ac 2 -y $@
