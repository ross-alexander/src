/* ----------------------------------------------------------------------
   --
   -- ip2sites
   --
   -- Takes a sites file (tab delimited IPv4 prefix + description)
   -- and builds a LC Trie from it with the description as the value.
   --
   --
   ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <getopt.h>

#include <map>
#include <vector>
#include <string>

#include "qsort.h"
#include "trie.h"

/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{

  /* Use getopt_long to process command line arguments */
  
  const char *sites_file = 0;
  const char *extract_file = 0;
  int c;
  while (1)
    {
      static struct option long_options[] = {
	{"sites", required_argument, 0, 's'},
	{"extract", required_argument, 0, 'e'},
	{0, 0, 0, 0}
      };
      int option_index = 0;

      c = getopt_long(argc, argv, "s:e:", long_options, &option_index);

      if (c == -1) break;

      switch (c)
	{
	case 's':
	  sites_file = optarg;
	  break;
	case 'e':
	  extract_file = optarg;
	  break;
	}
    }

  /* Check command line arguments had been given */

  if (sites_file == 0)
    {
      fprintf(stderr, "-s sites required.\n");
      exit(1);
    }
  if (extract_file == 0)
    {
      fprintf(stderr, "-e extract required.\n");
      exit(1);
    }
  
  std::vector<entry_t> etable;

  /* Open file with checking result */
  
  FILE *stream = fopen(sites_file, "r");

  if (stream == NULL)
    {
      fprintf(stderr, "Cannot open %s: %s\n", sites_file, strerror(errno));
      exit(1);
    }

  /* Use strtok to split fields */
  
  int line_len = 1024;
  char *line = (char*)calloc(line_len, sizeof(char));
  int eindex = 0;
  int nfields = 2;
  while (fgets(line, line_len, stream))
    {
      const char* delim = "\t\n";
      char *next_token, *token;
      char* bits[nfields];
      token = strtok_r(line, delim, &next_token);
      unsigned int count = 0;
      while (token && (count < nfields))
	{
	  bits[count++] = token;
	  token = strtok_r(NULL, delim, &next_token);
	}

      char *addr_field = bits[0];
      char *comment_field = bits[1];

      /* Split prefix into address and length */
      
      delim = "/";
      char *addr[2] = {NULL, NULL};
      count = 0;
      token = strtok_r(addr_field, delim, &next_token);
      while (token && (count < 2))
	{
	  addr[count++] = token;
	  token = strtok_r(NULL, delim, &next_token);
	}
      entry_t entry = new(entryrec);

      /* Use ntohl as inet_addr returns address in Network (big) endian */
      
      entry->data = (word)ntohl(inet_addr(addr[0]));
      entry->len = strtoul(addr[1], NULL, 10);
      entry->nexthop = strdup(comment_field);
      etable.push_back(entry);
    }
  fclose(stream);

  /* Build trie */
  
  routtable_t table = buildrouttable(etable.data(), etable.size(), 0.5, 16, 0);

  /* --------------------
     Read in extract file
     -------------------- */
  
  stream = fopen(extract_file, "r");
  if (stream == NULL)
    {
      fprintf(stderr, "Cannot open %s: %s\n", extract_file, strerror(errno));
      exit(1);
    }
  nfields = 6;
  while (fgets(line, line_len, stream))
    {
      const char* delim = "\t\n";
      char *next_token, *token;
      char* bits[nfields];
      token = strtok_r(line, delim, &next_token);
      unsigned int count = 0;
      while (token && (count < nfields))
	{
	  bits[count++] = token;
	  token = strtok_r(NULL, delim, &next_token);
	}
      
      /* Format is from jlog with the smoothwall.lua script */
      
      word addr = ntohl(inet_addr(bits[0]));
      nexthop_t nh = find(addr, table);
      if (!nh) nh = (void*)"****";
      printf("%-60s\t%-12s\t%s\n", nh, bits[0], bits[1]);
    }
  fclose(stream);
}
