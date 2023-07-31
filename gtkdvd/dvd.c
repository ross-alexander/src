#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dvdread/ifo_read.h>
#include <dvdread/nav_read.h>
#include <dvdnav/dvdnav.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#include <errno.h>
#include <glib.h>

#include "common.h"

typedef enum {
    TC_LOG_ERR = 0, /* critical error condition */
    TC_LOG_WARN,    /* non-critical error condition */
    TC_LOG_INFO,    /* informative highlighted message */
    TC_LOG_MSG,     /* regular message */

    TC_LOG_EXTRA,   /* must always be the last */
    /*
     * on this special log level is guaranteed that message will be logged
     * verbatim: no tag, no colours, anything
     */
} TCLogLevel;

#define tc_log_error(tag, format, args...) tc_log(TC_LOG_ERR, tag, format , ## args)
#define tc_log_info(tag, format, args...) tc_log(TC_LOG_INFO, tag, format , ## args)
#define tc_log_warn(tag, format, args...) tc_log(TC_LOG_WARN, tag, format , ## args)
#define tc_log_msg(tag, format, args...) tc_log(TC_LOG_MSG, tag, format , ## args)

void tc_log(int level, char *tag, char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  fprintf(stderr, "%s: ", tag);
  vfprintf(stderr, format, ap);
  va_end(ap);
  fprintf(stderr, "\n");
}

/* --------------------
Language lists
-------------------- */

static struct {
  const char code[3];
  const char name[20];
}

language[] = {
	{ "  ", "Not Specified" }, { "aa", "Afar" },
	{ "ab", "Abkhazian" }, { "af", "Afrikaans" },
	{ "am", "Amharic" }, { "ar", "Arabic" },
	{ "as", "Assamese" },	{ "ay", "Aymara" },
	{ "az", "Azerbaijani" }, { "ba", "Bashkir" },
	{ "be", "Byelorussian" }, { "bg", "Bulgarian" },
	{ "bh", "Bihari" }, { "bi", "Bislama" },
	{ "bn", "Bengali; Bangla" }, { "bo", "Tibetan" },
	{ "br", "Breton" },
	{ "ca", "Catalan" }, { "co", "Corsican" },
	{ "cs", "Czech" }, { "cy", "Welsh" },
	{ "da", "Dansk" }, { "de", "Deutsch" }, { "dz", "Bhutani" },
	{ "el", "Greek" }, { "en", "English" },
	{ "eo", "Esperanto" }, { "es", "Espanol" },
	{ "et", "Estonian" }, { "eu", "Basque" }, { "fa", "Persian" },
	{ "fi", "Suomi" }, { "fj", "Fiji" }, { "fo", "Faroese" },
	{ "fr", "Francais" }, { "fy", "Frisian" },
	{ "ga", "Gaelic" }, { "gd", "Scots Gaelic" }, { "gl", "Galician" },
	{ "gn", "Guarani" }, { "gu", "Gujarati" },
	{ "ha", "Hausa" }, { "he", "Hebrew" }, { "hi", "Hindi" },
	{ "hr", "Hrvatski" }, { "hu", "Magyar" }, { "hy", "Armenian" },
	{ "ia", "Interlingua" }, { "id", "Indonesian" },
	{ "ie", "Interlingue" }, { "ik", "Inupiak" }, { "in", "Indonesian" },
	{ "is", "Islenska" }, { "it", "Italiano" },
	{ "iu", "Inuktitut" }, { "iw", "Hebrew" },
	{ "ja", "Japanese" }, { "ji", "Yiddish" }, { "jw", "Javanese" },
	{ "ka", "Georgian" }, { "kk", "Kazakh" }, { "kl", "Greenlandic" },
	{ "km", "Cambodian" }, { "kn", "Kannada" }, { "ko", "Korean" },
	{ "ks", "Kashmiri" }, { "ku", "Kurdish" }, { "ky", "Kirghiz" },
	{ "la", "Latin" }, { "ln", "Lingala" }, { "lo", "Laothian" },
	{ "lt", "Lithuanian" }, { "lv", "Latvian, Lettish" },
	{ "mg", "Malagasy" }, { "mi", "Maori" }, { "mk", "Macedonian" },
	{ "ml", "Malayalam" }, { "mn", "Mongolian" }, { "mo", "Moldavian" },
	{ "mr", "Marathi" }, { "ms", "Malay" }, { "mt", "Maltese" },
	{ "my", "Burmese" },
	{ "na", "Nauru" }, { "ne", "Nepali" },
	{ "nl", "Nederlands" }, { "no", "Norsk" }, { "oc", "Occitan" },
	{ "om", "Oromo" }, { "or", "Oriya" },
	{ "pa", "Punjabi" }, { "pl", "Polish" }, { "ps", "Pashto, Pushto" },
	{ "pt", "Portugues" }, { "qu", "Quechua" },
	{ "rm", "Rhaeto-Romance" }, { "rn", "Kirundi" }, { "ro", "Romanian"  },
	{ "ru", "Russian" }, { "rw", "Kinyarwanda" },
	{ "sa", "Sanskrit" }, { "sd", "Sindhi" }, { "sg", "Sangho" },
	{ "sh", "Serbo-Croatian" }, { "si", "Sinhalese" }, { "sk", "Slovak" },
	{ "sl", "Slovenian" }, { "sm", "Samoan" },
 	{ "sn", "Shona"  }, { "so", "Somali" }, { "sq", "Albanian" },
	{ "sr", "Serbian" }, { "ss", "Siswati" },
	{ "st", "Sesotho" }, { "su", "Sundanese" },
	{ "sv", "Svenska" }, { "sw", "Swahili" },
	{ "ta", "Tamil" }, { "te", "Telugu" }, { "tg", "Tajik" },
	{ "th", "Thai" }, { "ti", "Tigrinya" }, { "tk", "Turkmen" },
	{ "tl", "Tagalog" }, { "tn", "Setswana" }, { "to", "Tonga" },
	{ "tr", "Turkish" }, { "ts", "Tsonga" },
	{ "tt", "Tatar" }, { "tw", "Twi" },
	{ "ug", "Uighur" }, { "uk", "Ukrainian" },
	{ "ur", "Urdu" }, { "uz", "Uzbek" },
	{ "vi", "Vietnamese" }, { "vo", "Volapuk" },
	{ "wo", "Wolof" }, { "xh", "Xhosa" },
	{ "yi", "Yiddish" }, { "yo", "Yoruba" },
	{ "za", "Zhuang" }, { "zh", "Chinese" },
	{ "zu", "Zulu" },
	{ "xx", "Unknown" },
	{ "\0", "Unknown" } };


char *quantization[4] = {"16bit", "20bit", "24bit", "drc"};
char *video_format[2] = {"NTSC", "PAL"};
char *aspect_ratio[4] = {"4/3", "16/9", "\"?:?\"", "16/9"};
char *video_width[4]  = {"720", "704", "352", "352"};
char *video_height[4] = {"480", "576", "???", "576"};
char *permitted_df[4] = {"P&S + Letter", "Pan&Scan", "Letterbox", "?"};
char *audio_format[7] = {"ac3", "?", "mpeg1", "mpeg2", "lpcm ", "sdds ", "dts"};
int   audio_id[7]     = {0x80, 0, 0xC0, 0xC0, 0xA0, 0, 0x88};
char *sample_freq[2]  = {"48000", "48000"};
char *audio_type[5]   = {"Undefined", "Normal", "Impaired", "Comments1", "Comments2"};
char *subp_type[16]   = {"Undefined", "Normal", "Large", "Children", "reserved", "Normal_CC", "Large_CC", "Children_CC",
			 "reserved", "Forced", "reserved", "reserved", "reserved", "Director",
			 "Large_Director", "Children_Director"};

double frames_per_s[4] = {-1.0, 25.00, -1.0, 29.97};

/* ----------------------------------------------------------------------
--
-- dvdtime2msec
--
-- Convert DVD frames to miliseconds
--
---------------------------------------------------------------------- */

int dvdtime2msec(dvd_time_t *dt)
{
  double fps = frames_per_s[(dt->frame_u & 0xc0) >> 6];
  long   ms;
  ms  = (((dt->hour &   0xf0) >> 3) * 5 + (dt->hour   & 0x0f)) * 3600000;
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
   -- lang_name
   --
   ---------------------------------------------------------------------- */

const char* lang_name(const char* code)
{
  int k = 0;
  while (memcmp(language[k].code, code, 2) && language[k].name[0] ) { k++; }
  return language[k].name;
}

/* ----------------------------------------------------------------------
--
-- title_gprint
--
---------------------------------------------------------------------- */

#define gsap(x, ...) g_string_append_printf(x, __VA_ARGS__)

GString* title_gprint(dvd_title_t *t)
{
  GString *s = g_string_new(0);
  playback_time_t *pbt = &t->general.playback_time;
  
  gsap(s, "Title: %02d, Length: %02d:%02d:%02d.%03d ", t->num, pbt->hour, pbt->minute, pbt->second, pbt->usec);
  gsap(s, "Chapters: %02d, Cells: %02d, ", t->chapter_count_reported, t->cell_count);
  gsap(s, "Audio streams: %02d, Subpictures: %02d", t->audiostream_count, t->subtitle_count);
  gsap(s, "\n"); 

  if (t->parameter.format != NULL)
    {
      gsap(s, "\tVTS: %02d, TTN: %02d, ", t->parameter.vts, t->parameter.ttn);
      gsap(s, "FPS: %.2f, ", t->parameter.fps);
      gsap(s, "Format: %s, Aspect ratio: %s, ", t->parameter.format, t->parameter.aspect);
      gsap(s, "Width: %s, Height: %s, ", t->parameter.width, t->parameter.height);
      gsap(s, "DF: %s\n", t->parameter.df);
    }
  
  // PALETTE
  if (t->palette != NULL)
    {
      gsap(s, "\tPalette: ");
      for (int i=0; i < 16; i++)
	{
	  gsap(s, "%06x ", t->palette[i]);
	}
      gsap(s, "\n");
    }
  
  // ANGLES
  if (t->angle_count)
    {
      gsap(s, "\tNumber of Angles: %d\n", t->angle_count);
    }

  // AUDIO
  if (t->audiostreams != NULL)
    {
      for (unsigned int i = 0; i < t->audiostream_count; i++)
	{
	  struct audiostream *as = &(t->audiostreams[i]);
	  gsap(s, "\tAudio: %d, Language: %s - %s, ", i+1, as->langcode, as->language);
	  gsap(s, "Format: %s, ", t->audiostreams[i].format);
	  gsap(s, "Frequency: %s, ", t->audiostreams[i].frequency);
	  gsap(s, "Quantization: %s, ", t->audiostreams[i].quantization);
	  gsap(s, "Channels: %d, AP: %d, ", t->audiostreams[i].channels, t->audiostreams[i].ap_mode);
	  gsap(s, "Content: %s, ", t->audiostreams[i].content);
	  gsap(s, "Stream id: 0x%x", t->audiostreams[i].streamid);
	  gsap(s, "\n");
	}
    }
  
  // CHAPTERS
  if (t->chapters != NULL)
    {
      for (unsigned int i = 0;  i <t->chapter_count; i++)
	{
	  playback_time_t *pbt = &t->chapters[i].playback_time;
	  gsap(s, "\tChapter: %02d, Length: %02d:%02d:%02d.%03d, Start Cell: %02d\n", i+1,
	       pbt->hour, pbt->minute, pbt->second, pbt->usec, t->chapters[i].startcell);
	}
    }
	  
  // CELLS
  if (t->cells != NULL)
    {
      for (int i=0; i<t->cell_count; i++)   
	{
	  gsap(s, "\tCell: %02d, Length: %02d:%02d:%02d.%03d\n", i+1, 
				 t->cells[i].playback_time.hour,
				 t->cells[i].playback_time.minute,
				 t->cells[i].playback_time.second,
				 t->cells[i].playback_time.usec);
	}
    }
  // SUBTITLES
  if (t->subtitles != NULL)
    {
      for (int i=0; i<t->subtitle_count; i++)
	{
	  gsap(s, "\tSubtitle: %02d, Language: %s - %s, ", i+1,
				 t->subtitles[i].langcode,
				 t->subtitles[i].language);
	  gsap(s, "Content: %s, ", t->subtitles[i].content);
	  gsap(s, "Stream id: 0x%x, ", t->subtitles[i].streamid);
	  gsap(s, "\n");
	}
    }
  return s;
}

/* ----------------------------------------------------------------------
   --
   -- dvd_gprint_title
   --
   ---------------------------------------------------------------------- */

GString* dvd_gprint_title(struct dvd_info_t *dvd_info, int track)
{
  return title_gprint(&dvd_info->titles[track]);
}

char* title_print(dvd_title_t *title)
{
  GString *s = title_gprint(title);
  return g_string_free(s, 0);
}

int get_title_name(const char* dvd_device, char* title)
{
  FILE *filehandle = 0;
  int  i;

  if (! (filehandle = fopen(dvd_device, "r")))
    {
      fprintf(stderr, "Couldn't open %s for title\n", dvd_device);
      strcpy(title, "unknown");
      return -1;
  }
  
  if ( fseek(filehandle, 32808, SEEK_SET ))
    {
      fclose(filehandle);
      fprintf(stderr, "Couldn't seek in %s for title\n", dvd_device);
      strcpy(title, "unknown");
      return -1;
  }

  if ( 32 != (i = fread(title, 1, 32, filehandle)))
    {
      fclose(filehandle);
      fprintf(stderr, "Couldn't read enough bytes for title.\n");
      strcpy(title, "unknown");
      return -1;
    }
  fclose (filehandle);
  
  title[32] = '\0';
  while(i-- > 2)
    if(title[i] == ' ') title[i] = '\0';
  return 0;
}


/* ----------------------------------------------------------------------
--
-- lsdvd_read_dvd
--
---------------------------------------------------------------------- */

dvd_info_t* lsdvd_read_dvd(const char *dvd_device)
{
  dvd_reader_t *dvd;
  ifo_handle_t *ifo_zero, **ifo;
  int opt_t = 0;
  int opt_v = 1;
  int opt_P = 1;
  int opt_n = 1;
  int opt_a = 1;
  int opt_c = 1;
  int opt_d = 1;
  int opt_s = 1;

  vtsi_mat_t *vtsi_mat;
  pgcit_t *vts_pgcit;
  audio_attr_t *audio_attr;
  video_attr_t *video_attr;
  subp_attr_t *subp_attr;
  pgc_t *pgc;
  int cell, vts_ttn, title_set_nr;
  int max_length = 0, max_track = 0;
  char lang_code[3];

  dvd = DVDOpen(dvd_device);
  if(!dvd)
    {
      fprintf(stderr, "Can't open disc %s!\n", dvd_device);
      exit(2);
    }
  ifo_zero = ifoOpen(dvd, 0);
  if (!ifo_zero)
    {
      fprintf( stderr, "Can't open main ifo!\n");
      exit(3);
  }

  ifo = (ifo_handle_t **)malloc((ifo_zero->vts_atrt->nr_of_vtss + 1) * sizeof(ifo_handle_t *));
  
  for (unsigned int i = 1; i <= ifo_zero->vts_atrt->nr_of_vtss; i++)
    {
      ifo[i] = ifoOpen(dvd, i);
      if (!ifo[i])
	{
	  fprintf( stderr, "Can't open ifo %d!\n", i);
	  exit(4);
	}
    }

  /* hack */
  char title[32];
  int has_title = get_title_name(dvd_device, title);

  int titles = ifo_zero->tt_srpt->nr_of_srpts;
  vmgi_mat_t *vmgi_mat = ifo_zero->vmgi_mat;
  dvd_info_t *dvd_info = calloc(sizeof(dvd_info_t), 1);
  
  dvd_info->discinfo.device = strdup(dvd_device);
  dvd_info->discinfo.disc_title = has_title ? "unknown" : strdup(title);
  dvd_info->discinfo.vmg_id = strdup(vmgi_mat->vmg_identifier);
  dvd_info->discinfo.provider_id = strdup(vmgi_mat->provider_identifier);
  dvd_info->title_count = titles;
  dvd_info->titles = calloc(titles, sizeof(*dvd_info->titles));

  for (unsigned int j = 0; j < titles; j++)
    {     
      if ((opt_t == j+1) || (opt_t == 0))
	{
	  // GENERAL
	  if (ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vtsi_mat)
	    {
	      dvd_info->titles[j].enabled = 1;
	      dvd_title_t *t = &dvd_info->titles[j];
	      t->num = j+1;

	      vtsi_mat     = ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vtsi_mat;
	      vts_pgcit    = ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vts_pgcit;
	      video_attr   = &vtsi_mat->vts_video_attr;
	      vts_ttn      = ifo_zero->tt_srpt->title[j].vts_ttn;
	      vmgi_mat     = ifo_zero->vmgi_mat;
	      title_set_nr = ifo_zero->tt_srpt->title[j].title_set_nr;
	      pgc = vts_pgcit->pgci_srp[ifo[title_set_nr]->vts_ptt_srpt->title[vts_ttn - 1].ptt[0].pgcn - 1].pgc;
	      
	      dvd_info->titles[j].general.length = dvdtime2msec(&pgc->playback_time)/1000.0;
	      converttime(&dvd_info->titles[j].general.playback_time, &pgc->playback_time);
	      dvd_info->titles[j].general.vts_id = vtsi_mat->vts_identifier;
	      
	      if (dvdtime2msec(&pgc->playback_time) > max_length)
		{
		  max_length = dvdtime2msec(&pgc->playback_time);
		  max_track = j+1;
		}
	      
	      dvd_info->titles[j].chapter_count_reported = ifo_zero->tt_srpt->title[j].nr_of_ptts;
	      dvd_info->titles[j].cell_count = pgc->nr_of_cells;
	      dvd_info->titles[j].audiostream_count = vtsi_mat->nr_of_vts_audio_streams;
	      dvd_info->titles[j].subtitle_count = vtsi_mat->nr_of_vts_subp_streams;  
	      
	      if(opt_v)
		{
		  dvd_info->titles[j].parameter.vts = ifo_zero->tt_srpt->title[j].title_set_nr;
		  dvd_info->titles[j].parameter.ttn = ifo_zero->tt_srpt->title[j].vts_ttn;
		  dvd_info->titles[j].parameter.fps = frames_per_s[(pgc->playback_time.frame_u & 0xc0) >> 6];
		  dvd_info->titles[j].parameter.format = video_format[video_attr->video_format];
		  dvd_info->titles[j].parameter.aspect = aspect_ratio[video_attr->display_aspect_ratio];		  
		  dvd_info->titles[j].parameter.width = video_width[video_attr->picture_size];
		  dvd_info->titles[j].parameter.height = video_height[video_attr->video_format];
		  dvd_info->titles[j].parameter.df = permitted_df[video_attr->permitted_df];
		}
	      // PALETTE
	      if (opt_P)
		{
		  dvd_info->titles[j].palette = malloc(16 * sizeof(int));
		  for (int i=1; i < 16; i++)
		    {
		      dvd_info->titles[j].palette[i] = pgc->palette[i];
		    }
		}
	      else
		{
		  dvd_info->titles[j].palette = NULL;
		}
	      // ANGLES
	      if (opt_n)
		{
		  dvd_info->titles[j].angle_count = ifo_zero->tt_srpt->title[j].nr_of_angles;
		}
	      else
		{
		  dvd_info->titles[j].angle_count = 0;
		}
	      // AUDIO
	      if (opt_a)
		{
		  dvd_info->titles[j].audiostreams = calloc(dvd_info->titles[j].audiostream_count, sizeof(*dvd_info->titles[j].audiostreams));
		  
		  for (int i=0; i<dvd_info->titles[j].audiostream_count; i++)
		    {
		      audio_attr = &vtsi_mat->vts_audio_attr[i];
		      sprintf(lang_code, "%c%c", audio_attr->lang_code>>8, audio_attr->lang_code & 0xff);
		      if (!lang_code[0])
			{
			  lang_code[0] = 'x'; lang_code[1] = 'x';
			}
		      
		      dvd_info->titles[j].audiostreams[i].langcode = strdup(lang_code);
		      dvd_info->titles[j].audiostreams[i].language = strdup(lang_name(lang_code));
		      dvd_info->titles[j].audiostreams[i].format = audio_format[audio_attr->audio_format];
		      dvd_info->titles[j].audiostreams[i].frequency = sample_freq[audio_attr->sample_frequency];
		      dvd_info->titles[j].audiostreams[i].quantization = quantization[audio_attr->quantization];
		      dvd_info->titles[j].audiostreams[i].channels = audio_attr->channels+1;
		      dvd_info->titles[j].audiostreams[i].ap_mode = audio_attr->application_mode;
		      dvd_info->titles[j].audiostreams[i].content = audio_type[audio_attr->lang_extension];
		      dvd_info->titles[j].audiostreams[i].streamid = audio_id[audio_attr->audio_format] + i;
		    }
		}
	      else
		{
		  dvd_info->titles[j].audiostreams = NULL;
		}	      
	      // CHAPTERS
	      cell = 0;
	      if (opt_c)
		{
		  dvd_info->titles[j].chapter_count = pgc->nr_of_programs;
		  dvd_info->titles[j].chapters = calloc(dvd_info->titles[j].chapter_count, sizeof(*dvd_info->titles[j].chapters));
		  
		  int ms;
		  for (int i=0; i<pgc->nr_of_programs; i++)
		    {	   
		      ms=0;
		      int next = pgc->program_map[i+1];   
		      if (i == pgc->nr_of_programs - 1) next = pgc->nr_of_cells + 1;
		      
		      while (cell < next - 1)
			{
			  ms = ms + dvdtime2msec(&pgc->cell_playback[cell].playback_time);
			  converttime(&dvd_info->titles[j].chapters[i].playback_time, &pgc->cell_playback[cell].playback_time);
			  cell++;
			}
		      dvd_info->titles[j].chapters[i].startcell = pgc->program_map[i];
		      dvd_info->titles[j].chapters[i].length = ms * 0.001;			
		    }
		}
	      // CELLS
	      dvd_info->titles[j].cells = calloc(dvd_info->titles[j].cell_count, sizeof(*dvd_info->titles[j].cells));
	      if (opt_d)
		{
		  for (int i=0; i<pgc->nr_of_cells; i++)
		    {
		      dvd_info->titles[j].cells[i].length = dvdtime2msec(&pgc->cell_playback[i].playback_time)/1000.0;
		      converttime(&dvd_info->titles[j].cells[i].playback_time, &pgc->cell_playback[i].playback_time);
		    }
		}
	      else
		{
		  dvd_info->titles[j].cells = NULL;
		}
	      // SUBTITLES
	      dvd_info->titles[j].subtitles = calloc(dvd_info->titles[j].subtitle_count, sizeof(*dvd_info->titles[j].subtitles));
	      if (opt_s)
		{
		  for (int i=0; i<vtsi_mat->nr_of_vts_subp_streams; i++)
		    {
		      subp_attr = &vtsi_mat->vts_subp_attr[i];
		      sprintf(lang_code, "%c%c", subp_attr->lang_code>>8, subp_attr->lang_code & 0xff);
		      if (!lang_code[0])
			{
			  lang_code[0] = 'x'; lang_code[1] = 'x';
			}
		      dvd_info->titles[j].subtitles[i].langcode = strdup(lang_code);
		      dvd_info->titles[j].subtitles[i].language = strdup(lang_name(lang_code));
		      dvd_info->titles[j].subtitles[i].content = subp_type[subp_attr->lang_extension];
		      dvd_info->titles[j].subtitles[i].streamid = 0x20 + i;				
		    }
		}
	      else
		{
		  dvd_info->titles[j].subtitles = NULL;
		}
	    } // if vtsi_mat
	} // if not -t
    } // for each title
  ifoClose(ifo_zero);
  DVDClose(dvd);
  return dvd_info;
}

/* ----------------------------------------------------------------------
--
-- tc_dvd_read
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

int tc_dvd_read(char *dvd_device, int arg_title, int arg_chapter, int arg_angle, unsigned char *data, int verbose, FILE *stream)
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

  dvd_reader_t *dvd = DVDOpen(dvd_device);

  chapid  = arg_chapter - 1;
  titleid = arg_title - 1;
  angle   = arg_angle - 1;
  
  /**
   * Load the video manager to find out the information about the titles on
   * this disc.
   */
  
  vmg_file = ifoOpen(dvd, 0);
  if(!vmg_file)
    {
      tc_log_error(__FILE__, "Can't open VMG info.");
      return -1;
  }
  tt_srpt = vmg_file->tt_srpt;

  /**
   * Make sure our title number is valid.
   */
  if( titleid < 0 || titleid >= tt_srpt->nr_of_srpts )
    {
      tc_log_error(__FILE__, "Invalid title %d.", titleid + 1);
      ifoClose( vmg_file );
      return -1;
    }

    /**
     * Make sure the chapter number is valid for this title.
     */

  if(chapid < 0 || chapid >= tt_srpt->title[titleid].nr_of_ptts)
    {
      tc_log_error(__FILE__, "Invalid chapter %d.", chapid + 1);
      ifoClose( vmg_file );
      return -1;
    }

  /**
   * Make sure the angle number is valid for this title.
   */
  if(angle < 0 || angle >= tt_srpt->title[titleid].nr_of_angles)
    {
      tc_log_error(__FILE__, "Invalid angle %d.", angle + 1);
      ifoClose( vmg_file );
      return -1;
    }
  
  /**
   * Load the VTS information for the title set our title is in.
   */

  vts_file = ifoOpen(dvd, tt_srpt->title[titleid].title_set_nr);
  if(!vts_file)
    {
      tc_log_error(__FILE__, "Can't open the title %d info file.", tt_srpt->title[titleid].title_set_nr);
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
  if (chapid+1 == tt_srpt->title[titleid].nr_of_ptts)
    {
      last_cell = cur_pgc->nr_of_cells;
    }
  else
    {
      last_cell = cur_pgc->program_map[ (vts_ptt_srpt->title[ttn - 1].ptt[chapid+1].pgn) - 1 ] - 1;
    }

  /**
   * We've got enough info, time to open the title set data.
   */
  
  title = DVDOpenFile(dvd, tt_srpt->title[titleid].title_set_nr, DVD_READ_TITLE_VOBS);

  if(!title)
    {
      tc_log_error(__FILE__, "Can't open title VOBS (VTS_%02d_1.VOB).", tt_srpt->title[ titleid ].title_set_nr);
      ifoClose(vts_file);
      ifoClose(vmg_file);
      return -1;
    }
  
  /**
   * Playback the cells for our chapter.
   */
  
  next_cell = start_cell;
    
  for(cur_cell = start_cell; next_cell < last_cell;)
    {
      cur_cell = next_cell;
      
      /* Check if we're entering an angle block. */
      if(cur_pgc->cell_playback[ cur_cell ].block_type == BLOCK_TYPE_ANGLE_BLOCK)
	{
	  cur_cell += angle;
	  for(int i = 0; ;++i)
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
      
      for(cur_pack = cur_pgc->cell_playback[cur_cell].first_sector; cur_pack < cur_pgc->cell_playback[cur_cell].last_sector;)
	{
	  dsi_t dsi_pack;
	  unsigned int next_vobu, next_ilvu_start, cur_output_size;
	  
      /**
       * Read NAV packet.
       */
	nav_retry:
	  len = DVDReadBlocks(title, (int)cur_pack, 1, data);
	  if( len != 1 )
	    {
	      tc_log_error(__FILE__, "Read failed for block %d", cur_pack);
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
	  navRead_DSI( &dsi_pack, &(data[ DSI_START_BYTE ]));

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
	  len = DVDReadBlocks(title, (int) cur_pack, cur_output_size, data);
	  if(len != (int) cur_output_size)
	    {
	      tc_log_error(__FILE__, "Read failed for %d blocks at %d", cur_output_size, cur_pack );
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
	  
	  if(verbose & TC_STATS)
	    tc_log_msg(__FILE__, "%d %d", cur_pack, cur_output_size);
	  cur_pack = next_vobu;
	}
    }
  ifoClose(vts_file);
  ifoClose(vmg_file);
  DVDCloseFile(title);
  DVDClose(dvd);
  return 0;
}


