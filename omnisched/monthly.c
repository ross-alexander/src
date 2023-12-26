#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

int main(int argc, char *argv[])
{
  int holiday = 0, hour, min, offset = 0, ch, mon, year = 2001;
  struct tm stm;
  time_t tick;
  char tmfmtstr[256], *tstr = "22:00";
  char *output = "omni";
  struct tm months[12];

  while ((ch = getopt(argc, argv, "fhvsy:t:o:")) != EOF)
    switch(ch)
      {
      case 'v':
	printf("%s: [-y year] [-f friday] [-s sunday] -t [HH:MM]\n", argv[0]);
	exit(0);
      case 'h':
	holiday = 1;
	break;
      case 'y':
	year = strtod(optarg, NULL);
	break;
      case 'f':
	offset = -1;
	break;
      case 's':
	  offset = +1;
	break;
      case 't':
	tstr = optarg;
	break;
      case 'o':
	output = optarg;
	break;
      }

  time(&tick);
  localtime_r(&tick, &stm);
  if (year < 100)
    {
      stm.tm_year = stm.tm_year - (stm.tm_year % 100) + year;
    }
  else
    {
      stm.tm_year = year - 1900;
    }
  strftime(tmfmtstr, 256, "%Y", &stm);
  if (stm.tm_year < 2)
    {
      fprintf(stderr, "Illegal year %s (must be later than 1901).\n", tmfmtstr);
      exit(1);
    }

  sscanf(tstr, "%u:%u", &hour, &min);
  if (hour < 0 || hour > 23)
    {
      fprintf(stderr, "Illegal hour %02u.\n", hour);
      exit(1);
    }
  if (min < 0 || min > 59)
    {
      fprintf(stderr, "Illegal minute %02u.\n", min);
      exit(1);
    }

  min = (min / 15) * 15;

  for (mon = 0; mon < 12; mon++)
    {

      /* --------------------
	 Get start of the month
	 -------------------- */

      stm.tm_mday = 1;
      stm.tm_mon = mon;
      stm.tm_hour = 0;
      stm.tm_min = 0;
      stm.tm_sec = 0;

      tick = mktime(&stm);

      /* --------------------
	 wday is number of days since Sunday
	 -------------------- */

      tick += (13 - stm.tm_wday) * (60 * 60 * 24);
      tick += offset * (60 * 60 * 24);
      localtime_r(&tick, &stm);
      stm.tm_hour = hour;
      stm.tm_min = min;
      tick = mktime(&stm);
      localtime_r(&tick, &months[mon]);
    }

  if (!strcmp("omni", output))
    {
      if (holiday)
	printf("-holidays on\n\n");
      printf("-full\n");
      printf("\t-only %s\n", tmfmtstr);
      for(mon = 0; mon < 12; mon++)
	{
	  strftime(tmfmtstr, 256, "\t\t-day %d -month %b\n", &months[mon]);
	  printf("%s", tmfmtstr);
	}
      strftime(tmfmtstr, 256, "\t-at %H:%M\n", &stm);
      printf("%s", tmfmtstr);
    }

  if (!strcmp("html", output))
    {
      printf("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
      printf("<html>\n");
      printf("<head>\n");
      printf("<title>Monthly (second Saturday of the month) for %d</title>\n", year);
      printf("<link rel=\"stylesheet\" type=\"text/css\" href=\"monthly.css\"/>\n");
      printf("</head>\n");
      printf("<body>\n");
      printf("<h1>Monthly (second Saturday of the month) for %d</h1>\n", year);
      printf("<table>\n");
      for (mon = 0; mon < 12; mon++)
	{
	  strftime(tmfmtstr, 256, "<tr><td>%B</td><td>%A %d</td></tr>\n", &months[mon]);
	  printf("%s", tmfmtstr);
	}
      printf("</table></body></html>\n");
    }

  return 0;
}
