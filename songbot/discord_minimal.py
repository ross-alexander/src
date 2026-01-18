#!/usr/bin/python3.12

# ----------------------------------------------------------------------
#
# discord_minimal.py
#
# 2024-07-16: Minimal install that plays a single mp3 if user is
# in a voice channel
#
# ----------------------------------------------------------------------

# This example requires the 'message_content' intent.

import discord
import argparse
import json

intents = discord.Intents.default()
intents.message_content = True

client = discord.Client(intents=intents)

@client.event
async def on_ready():
    print(f'We have logged in as {client.user}')
    

@client.event
async def on_message(message):
    if message.author == client.user:
        return

    if message.content.startswith('$hello'):
        if message.author.voice is None:
            await message.channel.send('Hello!')
        else:
            print(message.content)
            vc = await message.author.voice.channel.connect()
            vc.play(discord.FFmpegPCMAudio("/locker/media/misc/gta3_intro.mp3"))


# ----------------------------------------------------------------------
#
# M A I N
#
# ----------------------------------------------------------------------

parser = argparse.ArgumentParser()

token_group = parser.add_mutually_exclusive_group(required=True)
token_group.add_argument("--token_file", action="store", help="json file which has discord bot token")
token_group.add_argument("--token", action="store", help="bot token")

# parser.add_argument("--database", "-d", action="store", help="json file which has song file path and info", required=True)

args = parser.parse_args()

print("Initializing...")
if not (args.token_file is None):
    with open(args.token_file, "r") as f:
        tmp = json.load(f)
        token = tmp["token"]
elif not (args.token is None):
    token = args.token

# with open(args.database, "r") as f:
#    db = json.load(f)

print("Token loaded.")

client.run(token)
