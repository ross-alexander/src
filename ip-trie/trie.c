#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <glib.h>

#define TABLE_MAX 1024

int loglevel = 0;

void log_printf(int level, char* s, ...)
{
  va_list ap;
  va_start(ap, s);
  if (level <= loglevel)
    vprintf(s, ap);
}

uint32_t str2word(const char *s)
{
  struct in_addr a;
  inet_aton(s, &a);
  return a.s_addr;
}

/* **********************************************************************
*
*  Prefix first search code
*
********************************************************************** */
typedef unsigned int word;
typedef unsigned long longword;

typedef struct {
  longword addr, mask, nextHop; /* 7/7/93 - maybe should be changed */
  word metric;
  int interface;
  int learnedIntf;
  int flags;
  long pktSend;
  long ttl;
} tableRec;

static unsigned int tableSize;
static tableRec *rTable;


#define NODE_BRANCH 1
#define NODE_LEAF 2
#define NODE_EMPTY 4

typedef struct node {
  int type;
  struct node *left, *right;
  longword key;
  //  word index;
  word len;
  void *value;
} node;

node *prefixRoot;

int BitCheck(word key, word index)
{
  return (key & (1 << (31 - index))) ? 1 : 0;
}

node *Empty(void)
{
  node *nd;
  nd = (node*)calloc(1, sizeof(node));
  nd->type = NODE_EMPTY;
  nd->left = nd->right = 0;
  nd->value = 0;
  return nd;
}

char* int2binstr(longword key, longword len)
{
  char *res = calloc(sizeof(char), len + 2);
  for (int i = 0; i < len; i++)
    res[i] = key & (1 << (31 - i)) ? '1' : '0';
  return res;
}

/* ----------------------------------------------------------------------
   --
   -- Count
   --
   ---------------------------------------------------------------------- */

int Count(int type, node *nd)
{
  register int i;
  switch(nd->type)
    {
    case NODE_EMPTY:
      return (type & 0x01) ? 1 : 0;
      break;
    case NODE_LEAF:
      return (type & 0x02) ? 1 : 0;
      break;
    case NODE_BRANCH:
      i = Count(type, nd->left) + Count(type, nd->right);
      return (type & 0x04) ? i + 1 : i; 
    }
  return 0;
} 

/* --------------------------------------------------
   Dump

   Outputs all leaf nodes to the stdout
-------------------------------------------------- */

void Dump(node *self, longword depth)
{
  for (int i = 0; i < depth; i++)
    printf("  ");
  if (self->type == NODE_EMPTY)
    {
      printf("|\n");
      return;
    }
  char *s = int2binstr(self->key, self->len);
  printf("%-20s %s\n", (char*)self->value, s);
  free(s);
  
  if (self->type == NODE_BRANCH)
    {
      printf("←"); Dump(self->left, depth + 1);
      printf("→"); Dump(self->right, depth + 1);
    }
}

#ifdef Aggr

/* --------------------------------------------------
   Aggregate
-------------------------------------------------- */
void Aggr(word n, longword v, node *nd)
{
  longword i, l, r;
  int count = 0;

  switch (nd->type)
    {
    case NODE_LEAF:
      return;
    case NODE_EMPTY:
      return;

    case NODE_BRANCH:
      Aggr(n + 1, v << 1, nd->l);
      Aggr(n + 1, v << 1 | 0x01, nd->r);

      l = nd->l->type;
      r = nd->r->type;

      if ((l == NODE_EMPTY) && (r == NODE_EMPTY))
	{
	  nd->type = NODE_EMPTY;
	  free(nd->l);
	  free(nd->r);
	  return;
	}
      if ((l == NODE_LEAF) && (r == NODE_EMPTY))
	{
	  nd->type = NODE_LEAF;
	  nd->i = nd->l->i;
	  nd->n = 0;
	  nd->v = v << 32 - n;
	  free(nd->r);
	  free(nd->l);
	}
      if ((l == NODE_EMPTY) && (r == NODE_LEAF))
	{
	  nd->type = NODE_LEAF;
	  nd->i = nd->r->i;
	  nd->n = 0;
	  nd->v = v << 32 - n;
	  free(nd->r);
	  free(nd->l);
	}
      if ((l == NODE_LEAF) && (r == NODE_LEAF))
	if (nd->l->i == nd->r->i)
	  {
	    nd->type = NODE_LEAF;
	    nd->i = nd->l->i;
	    nd->n = 0;
	    nd->v = v << 32 - n;
	    free(nd->l);
	    free(nd->r);
	  }
    }
}

#endif

/* --------------------------------------------------
   Insert
-------------------------------------------------- */

void Insert(node *self, longword key, word len, word index, void* value)
{
  log_printf(2, "Insert %02ld %08lX/%d : ", index, key, len);

  switch(self->type)
    {
    case NODE_EMPTY:
      /* if (index < len) */
      /* 	{ */
      /* 	  self->type = NODE_BRANCH; */
      /* 	  self->left = Empty(); */
      /* 	  self->right = Empty(); */
      /* 	  self->value = value; */
      /* 	  self->len = len; */
      /* 	  self->key = key; */
      /* 	  Insert(self, key, len, index, value); */
      /* 	} */
      /* else */
	{
	  log_printf(1, "empty -> leaf(%s)\n", value);
	  self->type = NODE_LEAF;
	  self->key = key;
	  self->len = len;
	  self->value = value;
	}
      break;
    case NODE_BRANCH:
      if (index < len)
	{
	  int bit = BitCheck(key, index);
	  log_printf(2, "%s\n", bit ? "branch right" : "branch left");
	  Insert(bit ? self->right : self->left, key, len, index + 1, value);
        }
      else
	{
	  log_printf(2, "branch value %s\n", value);
	  self->key = key;
	  self->len = len;
	  self->value = value;
	}
      break;

    case NODE_LEAF:
      self->type = NODE_BRANCH;
      self->left = Empty();
      self->right = Empty();
      void *oldvalue = self->value;
      //      self->value = value;
      log_printf(2, "leaf %s\n", oldvalue);
      
      Insert(self, self->key, self->len, index, oldvalue);
      Insert(self, key, len, index, value);
      break;
    }
}

/* ----------------------------------------------------------------------
--
-- Search
--
---------------------------------------------------------------------- */

void* Search(node *self, word key, word index)
{
  void *ret = nullptr;
  switch(self->type)
    {
    case NODE_EMPTY:
      log_printf(1, "Empty\n");
      ret = nullptr;
      break;
    case NODE_LEAF:
      log_printf(1, "Leaf %ld %08lX\n", index, self->key);
      ret = self->value;
      break;
    case NODE_BRANCH:
      {
	int bit = BitCheck(key, index);
	log_printf(1, "Branch %s %d\n", bit ? "R" : "L", bit);
	node *branch = bit ? self->right : self->left;
	void *res = Search(branch, key, index + 1);
	if ((res == 0) && (self->value != 0))
	  res = self->value;
	ret = res;
	break;
      }
    }
  return ret;
}

/* ----------------------------------------------------------------------
   --
   -- main
   --
   ---------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  tableSize = 0;
  rTable = malloc(sizeof(tableRec) * TABLE_MAX);
  prefixRoot = Empty();

  FILE* stream;
  const char *path = nullptr;

  if (argc > 1)
    {
      path = argv[1];
      stream = fopen(path, "r");
      if (!stream)
	{
	  fprintf(stderr, "Failed to open %s\n", path);
	  exit(1);
	}
      
      char line[1024];
      while(fgets(line, sizeof(line), stream))
	{
	  gchar **part = g_strsplit_set(line, ",\n", 0);
	  gchar **quad = g_strsplit(part[2], ".", 0);
	  word ip = 0;
	  for (int i = 0; quad[i]; i++)
	    {
	      int j = strtoul(quad[i], 0, 10) << (8 * (3-i));
	      ip |= j;
	    }
	  //      printf("%s/%s %08x\n", part[2], part[3], ip);
	  int plen = strtoul(part[3], 0, 10);
	  Insert(prefixRoot, ip, plen, 0, g_strdup_printf("%s/%s", part[2], part[3]));
	}
      fclose(stream);
      Dump(prefixRoot, 0);
    }
  else
    {
      loglevel = 3;
      //      Insert(prefixRoot, 0x00000000, 0, 0, "0.0.0.0/0");

      Insert(prefixRoot, 0xac1d0000, 16, 0, "172.29.0.0/16");
      Dump(prefixRoot, 0);
      printf("--------------------\n");
      //  Insert(prefixRoot, 0xac1d0800, 21, 0, "172.29.8.0/21");
      //  Dump(0, prefixRoot);
    }

  char *s;
  printf("++ %s\n", (s = Search(prefixRoot, str2word("1.1.1.1"), 0)) ? s : "null");
  //  printf("++ %s\n", (s = Search(prefixRoot, 0xac1d0708, 0)) ? s : "null");
  //  printf("++ %s\n", (s = Search(prefixRoot, 0xac1d0a08, 0)) ? s : "null");
  //  printf("++ %s\n", (s = Search(prefixRoot, 0xac1dfff0, 0)) ? s : "null");

  //  printf("++ total = %d leaves = %d branches & leaves = %d\n",
  //	 Count(7, prefixRoot), Count(2, prefixRoot), Count(6, prefixRoot));
}
