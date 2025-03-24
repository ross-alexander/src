#define TC_STATS 4

/* --------------------
Time record
-------------------- */

typedef struct playback_time_t {
  unsigned int hour;
  unsigned int minute;
  unsigned int second;
  unsigned int usec;
} playback_time_t;

/* --------------------
dvd structure
-------------------- */

struct dvd_title_t
{
  int enabled;
  int num;
  struct dvd_info_t *dvd_info;
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
    char *format;
    char *aspect;
    char *width;
    char *height;
    char *df;
  } parameter;
  int angle_count; // no real angle detail is available... but hey.
  int audiostream_count;
  struct audiostream
  {
    char *langcode;
    char *language;
    char *format;
    char *frequency;
    char *quantization;
    int channels;
    int ap_mode;
    char *content;
    int streamid;
  } *audiostreams;
  int chapter_count_reported; // This value is sometimes wrong
  int chapter_count; //This value is real
  struct
  {
    float length;
    playback_time_t playback_time;
    int startcell;
  } *chapters;
  int cell_count;
  struct
  {
    float length;
    playback_time_t playback_time;
  } *cells;
  int subtitle_count;
  struct
  {
    char *langcode;
    char *language;
    char *content;
    int streamid;
  } *subtitles;
  int *palette;
};

typedef struct dvd_title_t dvd_title_t;

struct dvd_info_t {
  struct {
    char *device;
    char *disc_title;
    char *vmg_id;
    char *provider_id;
  } discinfo;
  int title_count;
  dvd_title_t *titles;
  int longest_track;
};

typedef struct dvd_info_t dvd_info_t;

#define DVD_DEFAULT_DEVICE "/dev/sr0";
