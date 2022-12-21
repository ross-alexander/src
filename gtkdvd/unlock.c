/*
* cd-dvd-lock.c
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* The initial developer of the original code is Deyan T. Chepishev
* http://www.poweradded.net/
*
* (C) 2009             Deyan T. Chepishev
*/

/*
*
* The CD/DVD device is expected to be /dev/cdrom. This is
* usually a symlink to the real device. If you dont want to create a symlink
* just edit the path in the code and point it to your CD/DVD device.
*
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <getopt.h>

void usage( char a[] );

int main(int argc, char *argv[])
{

//The location of cd/dvd device. Edit this if necessary

  char *device = "/dev/sr0";
  int fd, lock = -1, eject = -1;
  int ch;

// Set unbuffered output
  setvbuf(stdout, NULL, _IONBF, 0);

  while((ch = getopt(argc, argv, "d:")) != EOF)
    switch(ch)
      {
      case 'd':
     	device = optarg;
	break;
      }

  if((argc - optind) < 1)
    {
      usage(argv[0]);
      exit(1);
    }
  
  for (int i = optind; i < argc; i++)
    {
      if (strcasecmp(argv[i], "lock") == 0)
	lock = 1;
      if(strcasecmp(argv[1], "unlock") == 0)
	lock = 0;
      if(strcasecmp(argv[1], "eject") == 0)
	eject = 1;
    }      
  if(lock < 0)
    {
      usage(argv[0]);
      exit(1);
    }

  if ((fd=open(device,O_RDWR|O_NONBLOCK)) == -1)
    {
      printf("Error opening device: \"%s\", ",device);
      perror("");
      exit(1);
    }
  if (ioctl(fd, CDROM_LOCKDOOR, lock) != 0)
    {
      printf("Error locking device: \"%s\", ",device);
      perror("");
      exit(1);
    }
  if (eject)
    ioctl(fd, CDROMEJECT, eject);
}

void usage( char a[] )
{
  printf("\n\nUsage %s [lock|unlock]\n\n", a);
}
