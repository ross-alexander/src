#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <gio/gio.h>

int main()
{
  char *file = "drawing.svg";
  struct stat statbuf;
  stat(file, &statbuf);
  printf("Size = %d\n", statbuf.st_size);
  int size = statbuf.st_size;
  char *data = calloc(sizeof(char), size);
  FILE *stream = fopen(file, "rb");
  fread(data, sizeof(char), size, stream);
  fclose(stream);

  char *mimetype = g_content_type_guess(file, data, size, NULL);
  printf("Got mimetype %s\n", mimetype);

  mimetype = g_content_type_guess(NULL, data, size, NULL);
  printf("Got mimetype %s\n", mimetype);
}
