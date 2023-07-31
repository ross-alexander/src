/* ----------------------------------------------------------------------
   --
   -- dvdrip
   --
   -- 2023-04-02: ralexand
   --
   ---------------------------------------------------------------------- */

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <lua/5.4/lua.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <glibmm/ustring.h>

#include <dvdread/ifo_read.h>
#include <dvdread/nav_read.h>

#include "dvdrip.h"

static double frames_per_s[4] = {-1.0, 25.00, -1.0, 29.97};

struct audiostream
{
  const char *langcode;
  const char *language;
  const char *format;
  const char *frequency;
  const char *quantization;
  int channels;
  int ap_mode;
  const char *content;
  int streamid;
};

struct chapter
{
  float length;
  playback_time_t playback_time;
  int startcell;
};

struct cell
{
  float length;
  playback_time_t playback_time;
};

struct subtitle
{
  const char *langcode;
  const char *language;
  const char *content;
  int streamid;
};

struct dvd_title_t
{
  int enabled;
  int num;
  //  struct dvd_info_t *dvd_info;
  struct
  {
    float length;
    playback_time_t playback_time;
    char *vts_id;
  } general;
  struct
  {
    int vts;
    int ttn;
    float fps;
    const char *format;
    const char *aspect;
    const char *width;
    const char *height;
    const char *df;
  } parameter;
  int angle_count; // no real angle detail is available... but hey.
  int audiostream_count;
  struct audiostream *audiostreams;
  int chapter_count_reported; // This value is sometimes wrong
  int chapter_count; //This value is real
  struct chapter* chapters;
  int cell_count;
  struct cell *cells;
  int subtitle_count;
  struct subtitle *subtitles;
  int *palette;
};

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


const char *quantization[4] = {"16bit", "20bit", "24bit", "drc"};
const char *video_format[2] = {"NTSC", "PAL"};
const char *aspect_ratio[4] = {"4/3", "16/9", "\"?:?\"", "16/9"};
const char *video_width[4]  = {"720", "704", "352", "352"};
const char *video_height[4] = {"480", "576", "???", "576"};
const char *permitted_df[4] = {"P&S + Letter", "Pan&Scan", "Letterbox", "?"};
const char *audio_format[7] = {"ac3", "?", "mpeg1", "mpeg2", "lpcm ", "sdds ", "dts"};
const int   audio_id[7]     = {0x80, 0, 0xC0, 0xC0, 0xA0, 0, 0x88};
const char *sample_freq[2]  = {"48000", "48000"};
const char *audio_type[5]   = {"Undefined", "Normal", "Impaired", "Comments1", "Comments2"};
const char *subp_type[16]   = {"Undefined", "Normal", "Large", "Children", "reserved", "Normal_CC", "Large_CC", "Children_CC",
			 "reserved", "Forced", "reserved", "reserved", "reserved", "Director",
			 "Large_Director", "Children_Director"};

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
-- lsdvd_read_dvd
--
---------------------------------------------------------------------- */

void lsdvd_read_dvd(const char *dvd_device)
{
  dvd_reader_t *dvd;
  ifo_handle_t *ifo_zero, **ifo;
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

  ifo = new ifo_handle_t*[ifo_zero->vts_atrt->nr_of_vtss + 1];
  
  for (unsigned int i = 1; i <= ifo_zero->vts_atrt->nr_of_vtss; i++)
    {
      ifo[i] = ifoOpen(dvd, i);
      if (!ifo[i])
	{
	  fprintf( stderr, "Can't open ifo %d!\n", i);
	  exit(4);
	}
    }

  std::vector<dvd_title_t*> titles;
  
  int nr_titles = ifo_zero->tt_srpt->nr_of_srpts;
  vmgi_mat_t *vmgi_mat = ifo_zero->vmgi_mat;

  for (unsigned int j = 0; j < nr_titles; j++)
    {     
      if (!ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vtsi_mat)
	continue;

      dvd_title_t* title_ptr = new dvd_title_t;
      dvd_title_t &t = *title_ptr;

      t.num = j + 1;
      
      vtsi_mat     = ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vtsi_mat;
      vts_pgcit    = ifo[ifo_zero->tt_srpt->title[j].title_set_nr]->vts_pgcit;
      video_attr   = &vtsi_mat->vts_video_attr;
      vts_ttn      = ifo_zero->tt_srpt->title[j].vts_ttn;
      vmgi_mat     = ifo_zero->vmgi_mat;
      title_set_nr = ifo_zero->tt_srpt->title[j].title_set_nr;
      pgc          = vts_pgcit->pgci_srp[ifo[title_set_nr]->vts_ptt_srpt->title[vts_ttn - 1].ptt[0].pgcn - 1].pgc;
      
      t.general.length = dvdtime2msec(&pgc->playback_time)/1000.0;

      printf("%f\n", t.general.length);
      t.general.playback_time = {0,0,0,0};
      converttime(&t.general.playback_time, &pgc->playback_time);

      t.general.vts_id = vtsi_mat->vts_identifier;
      
      t.chapter_count_reported = ifo_zero->tt_srpt->title[j].nr_of_ptts;
      t.cell_count = pgc->nr_of_cells;
      t.audiostream_count = vtsi_mat->nr_of_vts_audio_streams;
      t.subtitle_count = vtsi_mat->nr_of_vts_subp_streams;  
      
      t.parameter.vts = ifo_zero->tt_srpt->title[j].title_set_nr;
      t.parameter.ttn = ifo_zero->tt_srpt->title[j].vts_ttn;
      t.parameter.fps = frames_per_s[(pgc->playback_time.frame_u & 0xc0) >> 6];
      t.parameter.format = video_format[video_attr->video_format];
      t.parameter.aspect = aspect_ratio[video_attr->display_aspect_ratio];		  
      t.parameter.width = video_width[video_attr->picture_size];
      t.parameter.height = video_height[video_attr->video_format];
      t.parameter.df = permitted_df[video_attr->permitted_df];
      
      t.palette = new int[16];
      for (int i = 1; i < 16; i++)
	t.palette[i] = pgc->palette[i];
      
      // ANGLES
      t.angle_count = ifo_zero->tt_srpt->title[j].nr_of_angles;
      
      // AUDIO
      
      t.audiostreams = new audiostream[t.audiostream_count];
      
      for (int i = 0; i < t.audiostream_count; i++)
	{
	  audio_attr = &vtsi_mat->vts_audio_attr[i];
	  sprintf(lang_code, "%c%c", audio_attr->lang_code>>8, audio_attr->lang_code & 0xff);
	  if (!lang_code[0])
	    {
	      lang_code[0] = 'x'; lang_code[1] = 'x';
	    }
	  
	  t.audiostreams[i].langcode = strdup(lang_code);
	  t.audiostreams[i].language = strdup(lang_name(lang_code));
	  t.audiostreams[i].format = audio_format[audio_attr->audio_format];
	  t.audiostreams[i].frequency = sample_freq[audio_attr->sample_frequency];
	  t.audiostreams[i].quantization = quantization[audio_attr->quantization];
	  t.audiostreams[i].channels = audio_attr->channels+1;
	  t.audiostreams[i].ap_mode = audio_attr->application_mode;
	  t.audiostreams[i].content = audio_type[audio_attr->lang_extension];
	  t.audiostreams[i].streamid = audio_id[audio_attr->audio_format] + i;
	}
      // CHAPTERS
      
      cell = 0;
      t.chapter_count = pgc->nr_of_programs;
      t.chapters = new chapter[t.chapter_count];
      
      int ms;
      for (int i = 0; i < pgc->nr_of_programs; i++)
	{	   
	  ms = 0;
	  int next = pgc->program_map[i+1];   
	  if (i == pgc->nr_of_programs - 1) next = pgc->nr_of_cells + 1;
	  
	  while (cell < next - 1)
	    {
	      ms = ms + dvdtime2msec(&pgc->cell_playback[cell].playback_time);
	      t.chapters[i].playback_time = {0, 0, 0, 0};
	      converttime(&t.chapters[i].playback_time, &pgc->cell_playback[cell].playback_time);
	      cell++;
	    }
	  t.chapters[i].startcell = pgc->program_map[i];
	  t.chapters[i].length = ms * 0.001;			
	}
  
      // CELLS
      t.cells = new struct cell[t.cell_count];
      
      for (int i = 0; i < pgc->nr_of_cells; i++)
	{
	  t.cells[i].length = dvdtime2msec(&pgc->cell_playback[i].playback_time)/1000.0;
	  t.cells[i].playback_time = {0, 0, 0, 0};
	  converttime(&t.cells[i].playback_time, &pgc->cell_playback[i].playback_time);
	}
  // SUBTITLES
      t.subtitles = new subtitle[t.subtitle_count];
      for (int i = 0; i < vtsi_mat->nr_of_vts_subp_streams; i++)
	{
	  subp_attr = &vtsi_mat->vts_subp_attr[i];
	  sprintf(lang_code, "%c%c", subp_attr->lang_code>>8, subp_attr->lang_code & 0xff);
	  if (!lang_code[0])
	    {
	      lang_code[0] = 'x'; lang_code[1] = 'x';
	    }
	  t.subtitles[i].langcode = strdup(lang_code);
	  t.subtitles[i].language = strdup(lang_name(lang_code));
	  t.subtitles[i].content = subp_type[subp_attr->lang_extension];
	  t.subtitles[i].streamid = 0x20 + i;				
	}
      titles.push_back(title_ptr);
    } // for each title
  ifoClose(ifo_zero);
  DVDClose(dvd);

  /* Convert into text */
  
  Glib::ustring s;  
  for (auto t : titles)
    {
      playback_time_t *pbt = &t->general.playback_time;

      
      s += Glib::ustring::sprintf("Title: %02d, Length: %02d:%02d:%02d.%03d ", t->num, pbt->hour, pbt->minute, pbt->second, pbt->usec);
      s += Glib::ustring::sprintf("Chapters: %02d, Cells: %02d, ", t->chapter_count_reported, t->cell_count);
      s += Glib::ustring::sprintf("Audio streams: %02d, Subpictures: %02d", t->audiostream_count, t->subtitle_count);
      s += Glib::ustring::sprintf("\n"); 
      
      if (t->parameter.format != NULL)
	{
	  s += Glib::ustring::sprintf("\tVTS: %02d, TTN: %02d, ", t->parameter.vts, t->parameter.ttn);
	  s += Glib::ustring::sprintf("FPS: %.2f, ", t->parameter.fps);
	  s += Glib::ustring::sprintf("Format: %s, Aspect ratio: %s, ", t->parameter.format, t->parameter.aspect);
	  s += Glib::ustring::sprintf("Width: %s, Height: %s, ", t->parameter.width, t->parameter.height);
	  s += Glib::ustring::sprintf("DF: %s\n", t->parameter.df);
	}
      
      // PALETTE
      if (t->palette != NULL)
	{
	  s += Glib::ustring::sprintf("\tPalette: ");
	  for (int i=0; i < 16; i++)
	    {
	      s += Glib::ustring::sprintf("%06x ", t->palette[i]);
	    }
	  s += Glib::ustring::sprintf("\n");
	}
      
      // ANGLES
      if (t->angle_count)
	{
	  s += Glib::ustring::sprintf("\tNumber of Angles: %d\n", t->angle_count);
	}
      
      // AUDIO
      if (t->audiostreams != NULL)
	{
	  for (unsigned int i = 0; i < t->audiostream_count; i++)
	    {
	      struct audiostream *as = &(t->audiostreams[i]);
	      s += Glib::ustring::sprintf("\tAudio: %d, Language: %s - %s, ", i+1, as->langcode, as->language);
	      s += Glib::ustring::sprintf("Format: %s, ", t->audiostreams[i].format);
	      s += Glib::ustring::sprintf("Frequency: %s, ", t->audiostreams[i].frequency);
	      s += Glib::ustring::sprintf("Quantization: %s, ", t->audiostreams[i].quantization);
	      s += Glib::ustring::sprintf("Channels: %d, AP: %d, ", t->audiostreams[i].channels, t->audiostreams[i].ap_mode);
	      s += Glib::ustring::sprintf("Content: %s, ", t->audiostreams[i].content);
	      s += Glib::ustring::sprintf("Stream id: 0x%x", t->audiostreams[i].streamid);
	      s += Glib::ustring::sprintf("\n");
	    }
	}
      
      // CHAPTERS
      if (t->chapters != NULL)
	{
	  for (unsigned int i = 0;  i <t->chapter_count; i++)
	    {
	      playback_time_t *pbt = &t->chapters[i].playback_time;
	      s += Glib::ustring::sprintf("\tChapter: %02d, Length: %02d:%02d:%02d.%03d, Start Cell: %02d\n", i+1,
					  pbt->hour, pbt->minute, pbt->second, pbt->usec, t->chapters[i].startcell);
	    }
	}
      
      // CELLS
      if (t->cells != NULL)
	{
	  for (int i=0; i<t->cell_count; i++)   
	    {
	      s += Glib::ustring::sprintf("\tCell: %02d, Length: %02d:%02d:%02d.%03d\n", i+1, 
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
	      s += Glib::ustring::sprintf("\tSubtitle: %02d, Language: %s - %s, ", i+1,
					  t->subtitles[i].langcode,
					  t->subtitles[i].language);
	      s += Glib::ustring::sprintf("Content: %s, ", t->subtitles[i].content);
	      s += Glib::ustring::sprintf("Stream id: 0x%x, ", t->subtitles[i].streamid);
	      s += Glib::ustring::sprintf("\n");
	    }
	}
    }
  std::cout << s;

  //  return dvd_info;
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

  lsdvd_read_dvd(dvdrip.device.c_str());
  exit(1);

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
