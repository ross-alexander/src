import discord
import argparse
from discord.ext import commands

#ffmpeg_options = {
#    'options': '-vn'
#}

class music_cog(commands.Cog):
    def __init__(self, bot, db):
        self.bot = bot
        self.song_parent = db["files_dir"]
        print("Music files will be loaded from {}.".format(self.song_parent))

        self.song_database = {}
        
        base_dir = db["files_dir"]
        for song in db["songs"]:
            path = base_dir + "/" + song['filename']
            self.song_database[song['song_keys'][0]] = path
        
        #all the music related stuff
        self.is_playing = False
        self.is_paused = False
        self.vc = None
        
        # 2d array containing [song, channel]
        self.music_queue = []

    def search_db(self, item):
        if item in self.song_database:
            return self.song_database[item]
        else:
            return None

    @commands.command(name="play", aliases=["p","playing"], help="Plays a selected song from youtube")
    async def play(self, ctx, *args):
        query = " ".join(args)
        try:
            voice_channel = ctx.author.voice.channel
        except:
            await ctx.send("```You need to connect to a voice channel first!```")
            return
        if self.is_paused:
            self.vc.resume()
        else:
            song = self.search_db(query)
            if type(song) == type(None):
                await ctx.send("```Could not download the song. Incorrect format try another keyword. This could be due to playlist or a livestream format.```")
            else:
                await ctx.send(f"Found {song}")
                self.vc = await voice_channel.connect()
                if self.vc == None:
                    await ctx.send("```Could not connect to the voice channel```")
                    return
                self.vc.play(discord.FFmpegPCMAudio(song))


    @commands.command(name="queue", aliases=["q"], help="Displays the current songs in queue")
    async def queue(self, ctx):
        retval = ""
        for i in range(0, len(self.music_queue)):
            retval += f"#{i+1} -" + self.music_queue[i][0]['title'] + "\n"

        if retval != "":
            await ctx.send(f"```queue:\n{retval}```")
        else:
            await ctx.send("```No music in queue```")
