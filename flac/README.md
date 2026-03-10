There are a number of scripts but the two important ones are `reflac.pl` and `m3u-meta.pl`

# REFLAC

The script recursively scans a directory looking for m3u files.  If
the destination is missing or older than the original then it uses
`ffmpeg` and `parallel` to convert the flac files into the destination
format.

opus: ./reflac.pl --format=opus --source=/locker/media/flac --target=/locker/media/opus
vorbis: ./reflac.pl --format=vorbis --source=/locker/media/flac --target=/locker/media/vorbis
aac: ./reflac.pl --format=aac --source=/locker/media/flac --target=/locker/media/m4a

# M3U-META

    ./m3u-meta.pl -i /locker/media/flac -o flac.js
    ./m3u-meta.pl -i /locker/media/m4a -o m4a.js
    ./m3u-meta.pl -i /locker/media/vorbis -o vorbis.js
    ./m3u-meta.pl -i /locker/media/opus -o opus.js
