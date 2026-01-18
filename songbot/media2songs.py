#!/usr/bin/python3

# ----------------------------------------------------------------------
#
# 2021-11-02: Ross Alexander
#
# ----------------------------------------------------------------------

import json
import os
import sys
import argparse

# --------------------
# Parse arguments with argparse
# --------------------

parser = argparse.ArgumentParser()
parser.add_argument('--in', dest='in_path', help='JSON file from m3u-meta')
parser.add_argument('--out', dest='out_path', help='Database from song_bot.py')

args = parser.parse_args()

if (not args.in_path):
    print("%s: --in <json file>" % sys.argv[0], file=sys.stderr)
    exit(1)

if (not args.out_path):
    print("%s: --out <json file>" % sys.argv[0], file=sys.stderr)
    exit(1)

# --------------------
# Open file and decode JSON
# --------------------

f = open(args.in_path, 'r', encoding='utf-8')
media = json.load(f)

if len(media) < 1:
    print("%s: json must be list with a minimum of one member" % (sys.argv[0]))
    exit(1)

media0 = media[0]

result = {
    'files_dir': media0['dir'],
    'songs': []
}

# --------------------
# Loop of m3u entries
# --------------------

for m3u in media0['m3u']:
    for file in m3u['files']:

# Handle case where dir is null
        
        if (m3u['dir'] != None):
            filename = os.path.join(m3u['dir'], file['file'])
        else:
            filename = file['file']

        print(filename)
        
        t = {
            'filename':filename,
            'song_keys': [ file['tags']['title'].casefold() ],
            'artist_keys': [ file['tags']['artist'].casefold() ],
            'album_keys': [ file['tags']['album'].casefold() ],
        }
        result['songs'].append(t)

# --------------------
# Write out and force utf-8
# --------------------

stream = open(args.out_path, 'w', encoding='utf-8')
json.dump(result, stream, indent=4, ensure_ascii=False)
