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
            vc = await message.author.voice.channel.connect()
            vc.play(discord.FFmpegPCMAudio("/locker/media/misc/gta3_intro.mp3"))

client.run('OTAyNjQyMzQ4Mzg1Nzk2MTI2.YXhZMg.QGtbE2Fnh2LAmiMaLzI7v9KIh5k')
