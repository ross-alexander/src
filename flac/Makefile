.SUFFIXES: .ogg .fla

%.ogg	: %.fla
	ffmpeg  -i $< -acodec libopus -ab 192k -ac 2 -y $@

all: flac-scan

flac-scan: flac-scan.go
	go build $<
