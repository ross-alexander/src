/* ----------------------------------------------------------------------
   --
   -- dvdrip
   --
   -- 2023-04-02: ralexand
   --
   ---------------------------------------------------------------------- */

#include <iostream>
#include <string>
#include <vector>
#include <lua/5.4/lua.hpp>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <dvdread/ifo_read.h>
#include <dvdread/nav_read.h>

#include "dvdrip.h"

static double frames_per_s[4] = {-1.0, 25.00, -1.0, 29.97};


/* ----------------------------------------------------------------------
--
-- dvdtime2msec
--
-- Convert DVD frames to miliseconds
--
---------------------------------------------------------------------- */

unsigned int dvdtime2msec(dvd_time_t *dt)
{
  double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];
  unsigned int ms;
  ms  = (((dt->hour   & 0xf0) >> 3) * 5 + (dt->hour   & 0x0f)) * 3600000;
  ms += (((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f)) * 60000;
  ms += (((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f)) * 1000;
  
  if(fps > 0)
    ms += ((dt->frame_u & 0x30) >> 3) * 5 + (dt->frame_u & 0x0f) * 1000.0 / fps;
  return ms;
}

/* ----------------------------------------------------------------------
--
-- converttime
--

 * This is used to add up sets of times in the struct. it's not elegant at all
 * but a quick way to easily add up 4 times at once. tracking the times in usec's 
 * constantly is easier, but without using math.h, it sucks to actually DO anything with it
 * also it ***has*** to be better to return the playback_time, not just mess with it like this

--
---------------------------------------------------------------------- */

void converttime(playback_time_t *pt, dvd_time_t *dt)
{
  double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];
  
  pt->usec = pt->usec + ((dt->frame_u & 0x30) >> 3) * 5 + (dt->frame_u & 0x0f) * 1000.0 / fps;
  pt->second = pt->second + ((dt->second & 0xf0) >> 3) * 5 + (dt->second & 0x0f);
  pt->minute = pt->minute + ((dt->minute & 0xf0) >> 3) * 5 + (dt->minute & 0x0f);
  pt->hour = pt->hour + ((dt->hour &   0xf0) >> 3) * 5 + (dt->hour   & 0x0f);
  
  if ( pt->usec >= 1000 ) { pt->usec -= 1000; pt->second++; }
  if ( pt->second >= 60 ) { pt->second -= 60; pt->minute++; }
  if ( pt->minute > 59 ) { pt->minute -= 60; pt->hour++; }
}

/* ----------------------------------------------------------------------
   --
   -- dvdrip_ifo
   --
   ---------------------------------------------------------------------- */

title_v dvdrip_ifo(dvdrip_t &dvdrip)
{
  ifo_handle_t *ifo_zero, **ifo;

  title_v titles;
  
  dvd_reader_t *reader = DVDOpen(dvdrip.device.c_str());
  if(!reader)
    {
      fprintf(stderr, "Can't open disc %s!\n", dvdrip.device.c_str());
      exit(2);
    }
  ifo_zero = ifoOpen(reader, 0);
  if (!ifo_zero)
    {
      fprintf(stderr, "Can't open main ifo!\n");
      exit(3);
  }

  /* Get number of Video Title Sets */

  unsigned int nr_of_vtss = ifo_zero->vts_atrt->nr_of_vtss;
  ifo = new ifo_handle_t*[nr_of_vtss + 1];

  for (unsigned int i = 1; i <= nr_of_vtss; i++)
    {
      ifo[i] = ifoOpen(reader, i);
      if (!ifo[i])
	{
	  fprintf( stderr, "Can't open ifo %d!\n", i);
	  exit(4);
	}
    }

  unsigned int nr_titles = ifo_zero->tt_srpt->nr_of_srpts;

  vtsi_mat_t *vtsi_mat;
  vmgi_mat_t *vmgi_mat;
  pgcit_t *vts_pgcit;
  audio_attr_t *audio_attr;
  video_attr_t *video_attr;
  subp_attr_t *subp_attr;
  pgc_t *pgc;
  int cell, vts_ttn, title_set_nr;

  /* Loop over titles */
  
  for (unsigned int j = 0; j < nr_titles; j++)
    {
      unsigned int title_set_nr = ifo_zero->tt_srpt->title[j].title_set_nr;

      if (!(vtsi_mat = ifo[title_set_nr]->vtsi_mat))
	break;

      vtsi_mat     = ifo[title_set_nr]->vtsi_mat;
      vts_pgcit    = ifo[title_set_nr]->vts_pgcit;
      video_attr   = &vtsi_mat->vts_video_attr;
      vts_ttn      = ifo_zero->tt_srpt->title[j].vts_ttn;
      vmgi_mat     = ifo_zero->vmgi_mat;
      pgc = vts_pgcit->pgci_srp[ifo[title_set_nr]->vts_ptt_srpt->title[vts_ttn - 1].ptt[0].pgcn - 1].pgc;

      unsigned int duration = dvdtime2msec(&pgc->playback_time)/1000.0;

      title_t *title = new title_t;
      title->title_nr = j;
      title->nr_chapters = pgc->nr_of_programs;
      title->duration = duration;
      title->title_set_nr = title_set_nr;
      title->vts_ttn = vts_ttn;
      titles.push_back(title);
    }
  ifoClose(ifo_zero);
  DVDClose(reader);
  return titles;
}  

/* ----------------------------------------------------------------------
   --
   -- dvdrip_read_chapter
   --
   -- This was copied from an old version of transcode
   --
   ---------------------------------------------------------------------- */

/**
 * Returns true if the pack is a NAV pack.  This check is clearly insufficient,
 * and sometimes we incorrectly think that valid other packs are NAV packs.  I
 * need to make this stronger.
 */

static int is_nav_pack(unsigned char *buffer)
{
  return buffer[41] == 0xbf && buffer[1027] == 0xbf;
}

int dvdrip_read_chapter(const char *dvd_device, unsigned int arg_title, unsigned int arg_chapter, unsigned int arg_angle, unsigned char *data, int verbose, FILE *stream)
{
  int pgc_id, len, start_cell, cur_cell, last_cell, next_cell;
  unsigned int cur_pack;
  int ttn, pgn;
  int lockretries;
  
  dvd_file_t *title;
  ifo_handle_t *vmg_file;
  tt_srpt_t *tt_srpt;
  ifo_handle_t *vts_file;
  vts_ptt_srpt_t *vts_ptt_srpt;
  pgc_t *cur_pgc;
  int titleid, angle, chapid;

  dvd_reader_t *reader = DVDOpen(dvd_device);

  chapid  = arg_chapter - 1;
  titleid = arg_title - 1;
  angle   = arg_angle - 1;
  
  /**
   * Load the video manager to find out the information about the titles on
   * this disc.
   */
  
  vmg_file = ifoOpen(reader, 0);
  if(!vmg_file)
    {
      fprintf(stderr, "Can't open VMG info.");
      return -1;
    }
  tt_srpt = vmg_file->tt_srpt;

  /**
   * Make sure our title number is valid.
   */
  if((titleid < 0) || (titleid >= tt_srpt->nr_of_srpts))
    {
      fprintf(stderr, "Invalid title %d\n", titleid + 1);
      ifoClose(vmg_file);
      return -1;
    }

    /**
     * Make sure the chapter number is valid for this title.
     */

  if((chapid < 0) || (chapid >= tt_srpt->title[titleid].nr_of_ptts))
    {
      fprintf(stderr, "Invalid chapter %d.", chapid + 1);
      ifoClose(vmg_file);
      return -1;
    }

  /**
   * Make sure the angle number is valid for this title.
   */
  if(angle < 0 || angle >= tt_srpt->title[titleid].nr_of_angles)
    {
      fprintf(stderr, "Invalid angle %d.", angle + 1);
      ifoClose(vmg_file);
      return -1;
    }
  
  /**
   * Load the VTS information for the title set our title is in.
   */

  unsigned int title_set_nr = tt_srpt->title[titleid].title_set_nr; 
  
  vts_file = ifoOpen(reader, title_set_nr);
  if(!vts_file)
    {
      fprintf(stderr, "Can't open the title %d info file.", title_set_nr);
      ifoClose(vmg_file);
      return -1;
    }

  /**
   * Determine which program chain we want to watch.  This is based on the
   * chapter number.
   */
  
  ttn = tt_srpt->title[titleid].vts_ttn;
  vts_ptt_srpt = vts_file->vts_ptt_srpt;
  pgc_id = vts_ptt_srpt->title[ttn - 1].ptt[chapid].pgcn;
  pgn = vts_ptt_srpt->title[ttn - 1].ptt[ chapid ].pgn;
  cur_pgc = vts_file->vts_pgcit->pgci_srp[pgc_id - 1].pgc;
  start_cell = cur_pgc->program_map[pgn - 1] - 1;
  
  //ThOe
  if (chapid + 1 == tt_srpt->title[titleid].nr_of_ptts)
    {
      last_cell = cur_pgc->nr_of_cells;
    }
  else
    {
      last_cell = cur_pgc->program_map[(vts_ptt_srpt->title[ttn - 1].ptt[chapid+1].pgn) - 1] - 1;
    }

  /**
   * We've got enough info, time to open the title set data.
   */
  
  title = DVDOpenFile(reader, title_set_nr, DVD_READ_TITLE_VOBS);

  if(!title)
    {
      fprintf(stderr, "Can't open title VOBS (VTS_%02d_1.VOB).", title_set_nr);
      ifoClose(vts_file);
      ifoClose(vmg_file);
      return -1;
    }
  
  /**
   * Playback the cells for our chapter.
   */
  
  next_cell = start_cell;
    
  for(cur_cell = start_cell; next_cell < last_cell; )
    {
      cur_cell = next_cell;
      
      /* Check if we're entering an angle block. */
      if(cur_pgc->cell_playback[ cur_cell ].block_type == BLOCK_TYPE_ANGLE_BLOCK)
	{
	  cur_cell += angle;
	  for(int i = 0; ; ++i)
	    {
	      if(cur_pgc->cell_playback[cur_cell + i].block_mode == BLOCK_MODE_LAST_CELL)
		{
		  next_cell = cur_cell + i + 1;
		  break;
		}
	    }
        }
      else
	{
	  next_cell = cur_cell + 1;
        }
      
      /**
       * We loop until we're out of this cell.
       */
      
      for(cur_pack = cur_pgc->cell_playback[cur_cell].first_sector; cur_pack < cur_pgc->cell_playback[cur_cell].last_sector; )
	{
	  dsi_t dsi_pack;
	  unsigned int next_vobu, next_ilvu_start, cur_output_size;
	  
      /**
       * Read NAV packet.
       */
	nav_retry:
	  len = DVDReadBlocks(title, (int)cur_pack, 1, data);
	  if(len != 1)
	    {
	      fprintf(stderr, "Read failed for block %d", cur_pack);
	      ifoClose(vts_file);
	      ifoClose(vmg_file);
	      DVDCloseFile(title);
	      return -1;
	    }

	  //assert( is_nav_pack( data ) );
	  if(!is_nav_pack(data))
	    {
	      cur_pack++;
	      goto nav_retry;
	    }
	  
      /**
       * Parse the contained dsi packet.
       */
	  navRead_DSI(&dsi_pack, &(data[DSI_START_BYTE]));

	  if(!( cur_pack == dsi_pack.dsi_gi.nv_pck_lbn))
	    {
	      cur_output_size = 0;
	      dsi_pack.vobu_sri.next_vobu = SRI_END_OF_CELL;
	    }
	  /**
	   * Determine where we go next.  These values are the ones we mostly
	   * care about.
	   */

	  next_ilvu_start = cur_pack + dsi_pack.sml_agli.data[angle].address;
	  cur_output_size = dsi_pack.dsi_gi.vobu_ea;

	  /**
	   * If we're not at the end of this cell, we can determine the next
	   * VOBU to display using the VOBU_SRI information section of the
	   * DSI.  Using this value correctly follows the current angle,
	   * avoiding the doubled scenes in The Matrix, and makes our life
	   * really happy.
	   *
	   * Otherwise, we set our next address past the end of this cell to
	   * force the code above to go to the next cell in the program.
	   */

	  if(dsi_pack.vobu_sri.next_vobu != SRI_END_OF_CELL)
	    {
	      next_vobu = cur_pack + ( dsi_pack.vobu_sri.next_vobu & 0x7fffffff );
	    }
	  else 
	    {
	      next_vobu = cur_pack + cur_output_size + 1;
	    }
	  assert(cur_output_size < 1024);
	  cur_pack++;

	  /**
	   * Read in and output cursize packs.
	   */
	  len = DVDReadBlocks(title, (int)cur_pack, cur_output_size, data);
	  if(len != (int)cur_output_size)
	    {
	      fprintf(stderr, "Read failed for %d blocks at %d", cur_output_size, cur_pack);
	      ifoClose(vts_file);
	      ifoClose(vmg_file);
	      DVDCloseFile(title);
	      return -1;
	    }
	  
	  if (fwrite(data, cur_output_size, DVD_VIDEO_LB_LEN, stream) <= 0)
	    {
	      fprintf(stderr, "Error writing: (%d) %s\n", errno, strerror(errno));
	      exit(1);
	    }
	  cur_pack = next_vobu;
	}
    }
  ifoClose(vts_file);
  ifoClose(vmg_file);
  DVDCloseFile(title);
  DVDClose(reader);
  return 0;
}

/* ----------------------------------------------------------------------
--
-- info_read_title
--
---------------------------------------------------------------------- */
 
int dvdrip_read_title(dvdrip_t* dvdrip, title_t* title, const char *dest)
{
  printf("Device = %s\n", dvdrip->device.c_str());
  int n_chapters = title->nr_chapters;
  
  FILE *stream;
  unsigned char *readdata = (unsigned char*)malloc(1024 * DVD_VIDEO_LB_LEN);

  if ((stream = fopen(dest, "wb")) == 0)
    return 0;

  /* read_chapter assumes titles and chapters start with 1 */
  
  for (int j = 1; j <= n_chapters; j++)
    {
      dvdrip_read_chapter(dvdrip->device.c_str(), title->title_nr + 1, j, 1, readdata, 1, stream);
      printf("Writing chapter %d of title %d to %s\n", j, title->title_nr, dest);
    }
  fclose(stream);
  return 1;
}


/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char* argv[])
{
  dvdrip_t dvdrip;
  int option_index = 0;
  char *long_arg;
  int c;
  static struct option long_options[] = {
    {"file",      required_argument, 0,  'f' },
    {"separator", required_argument, 0,  's' },
    {"index",     required_argument, 0,  'i' },
    {"device",    required_argument, 0,  'd' },
    {"path",      required_argument, 0,  'p' },
    {"lua",       required_argument, 0,  'l' },
    {0,           0,                 0,   0  }
  };
  while(1)
    {
      if ((c = getopt_long(argc, argv, "", long_options, &option_index)) == -1)
	break;
      switch(c)
	{
	case 'f':
	  dvdrip.file = std::string(optarg);
	  break;
	case 'l':
	  dvdrip.lua = std::string(optarg);
	  break;
	case 's':
	  dvdrip.separator = std::string(optarg);
	  break;
	case 'p':
	  dvdrip.path = std::string(optarg);
	  break;
	case 'i':
	  dvdrip.index = strtol(optarg, 0, 10);
	  break;
	}
    }

  /* Set default device to /dev/sr0 */
  
  if (dvdrip.device.empty())
    dvdrip.device = "/dev/sr0";


  /* --------------------
     Load lua script
     -------------------- */

  lua_State *L = dvdrip_lua_init(&dvdrip);
  
  /* Get information about titles */
  
  title_v titles = dvdrip_ifo(dvdrip);

  for (auto title: titles)
    {
      printf("Title %d: Set: %d TTN: %d Duration: %d Chapters: %d\n", title->title_nr, title->title_set_nr, title->vts_ttn, title->duration, title->nr_chapters);
    }

  dvdrip_lua_titles(L, &dvdrip, titles);
  exit(0);
}
