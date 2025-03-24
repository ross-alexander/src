#include "dvd.h"

int tc_dvd_read(char*, int, int, int, unsigned char*, int, FILE*);
int dvd_gtk(int argc, char *argv[], const char *device);
dvd_info_t* lsdvd_read_dvd(const char *device);
char *title_print(dvd_title_t*);
GString* title_gprint(dvd_title_t*);
GString* dvd_gprint_title(struct dvd_info_t *dvd_info, int track);
int get_title_name(const char* dvd_device, char* title);
int eject(const char *);
