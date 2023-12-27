/* **********************************************************************
*
*  Internet Protocol (IP)
* 
*  version 0.0.1
*    8 January 1993							
*    IPInit, IPIdle completed.  The code is able to receive incoming packets
*    and decode them.
*
*  version 0.0.2
*    12 February 1993
*    IMCPHandle now works so the machine can be "pinged".  Can now recieve and
*    demultiplex packets and send them.
*
*  version 0.0.3
*    15 March 1993
*    Updated to handle multiply interfaces.
*
*   version 0.0.4
*    21 March 1993
*    Bug fixed done to handle multiply interfaces.
*
*  version 0.0.5
*    22 April 1993
*    Code updated to take into account the IP address the packet is being sent
*    to may not be the same as the address as the packet's final destination.
*    Done in conjunction to changes in ARPResolve.
*
*************************************************************************
*
*  Routing of packets and Routing Information Protocol
* 
*  version 0.0.1
*   16 March 1993
*   Get static routing working first.  Look at RIP packets but don't do
*   anything with them.
*
*  version 0.0.2
*    7 April 1993
*    Routing code merged with ip code to form a single ip.c  file.  This is
*    done because it is too hard to easily seperate the two two apart.
*
*  version 0.0.3
*    20 April 1993
*    Now recieves RIP packets via UDP but currently does not use them to
*    calculate routing tables.  The code does lookup up the table to see if
*    network is in the routing table and uses this information to check if the
*    address is local.
*
*  version 0.0.4
*    14 May 1993
*    Send out local routing information and forward packets.
*
*  version 0.0.5
*    13 August 1993
*    Add broadcast address to forward table.
*    Add prefix addressing (partially)
*
********************************************************************** */
#include <time.h>
#include <stdlib.h>
#include "common.h"
#include "ip.h"

#define UDProuted 520
#define TABLE_MAX 200
 
#define Asubnet 0xFF000000;  /* 255.000.000.000 */
#define Bsubnet 0xFFFF0000;  /* 255.255.000.000 */
#define Csubnet 0xFFFFFF00;  /* 255.255.255.000 */

#define flagEmpty     0x0001
#define flagOld       0x0002
#define flagDefault   0x0004
#define flagLocal     0x0008
#define flagStatic    0x0010
#define flagThisHost  0x0020
#define flagBroadcast 0x0040

#define FLAG_LOCAL    flagLocal
#define FLAG_HOST     flagThisHost
#define FLAG_BCAST    flagBroadcast

#define RIP_IP_FAMILY_ID 2

#define UPDATE_FREQ 30

typedef struct {
  word familyID;
  word filler1;
  byte ipAddr[4];
  byte filler2[8];
  longword metric;
} ripEntry;

typedef struct {
  byte command; 
  byte version;
  word filler1;
} ripHdr;

typedef struct {
  longword addr, mask, nextHop; /* 7/7/93 - maybe should be changed */
  word metric;
  int interface;
  int learnedIntf;
  int flags;
  long pktSend;
  long ttl;
} tableRec;

static tableRec *rTable;
static int tableSize;

static PDRec *pd;
static time_t nextUpdate;

/* **********************************************************************
*
*
********************************************************************** */
void DumpAddr(int length, byte *ptr)
{
  int i;
  length--;
  for (i = 0; i < length; i++)
    printf("%03d.", ptr[i]);
  printf("%03d  ", ptr[length]);
}

void RouteDump()
{
  int i;

  for (i = 0; i < tableSize; i++)
    {
      DumpAddr(4, (byte*)&rTable[i].addr);
      DumpAddr(4, (byte*)&rTable[i].mask);
      DumpAddr(4, (byte*)&rTable[i].nextHop);
      printf("%d ", rTable[i].metric);
      if (rTable[i].flags & flagLocal)
        printf("L");
      if (rTable[i].flags & flagThisHost)
        printf("H");
      if (rTable[i].flags & flagBroadcast)
        printf("B");
      printf("\n");
    }
}

void SpecialDump()
{
  int i, j, k, l;

  for (i = 0; i < tableSize; i++)
    {
      printf("32\t");
      DumpAddr(4, (byte*)&rTable[i].addr);
      printf("\t%d\n", rTable[i].interface);
    }
}

/* **********************************************************************
*
*  Prefix first search code
*
********************************************************************** */
#define NODE_BRANCH 0
#define NODE_LEAF 1
#define NODE_EMPTY 2

typedef struct node {
  struct node *l, *r;
  int type;
  longword v, n;
  word i;
} node;

node *prefixRoot;

node *Empty()
{
  node *nd;
  nd = (node*)malloc(sizeof(node));
  nd->type = NODE_EMPTY;
  return nd;
}

void int2bin(longword n, longword v)
{
  word i;
  printf("%3ld %08lX", n, v);
  return;
  
  for (i = n - 1; i >= 0; i--)
    putchar( v & (1 << i) ? '1' : '0');
}

/* Count */
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
      i = Count(type, nd->l) + Count(type, nd->r);
      return (type & 0x04) ? i + 1 : i; 
    }
} 

/* --------------------------------------------------
   Dump

   Outputs all leaf nodes to the stdout
-------------------------------------------------- */
void Dump(longword depth, longword v, node *nd)
{
  int i;
  switch (nd->type)
    {
    case NODE_BRANCH:

printf("!brl n = %3ld v = %08lX\n", depth, v << 1);
      Dump(depth + 1, v << 1, nd->l);

printf("!brr n = %3ld v = %08lX\n", depth, v << 1 | 1);
      Dump(depth + 1, v << 1 | 1, nd->r);

      break;

    case NODE_LEAF:
      printf("|%3d %3ld %08lX ",
             nd->i, nd->n, nd->v);
             
      int2bin(depth, v);
      printf(" + ");
      int2bin(nd->n, nd->v);
      printf(" *\n");
      break;
    }
}

/* --------------------------------------------------
   Aggrigate
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

/* --------------------------------------------------
   Insert
-------------------------------------------------- */
void Insert(node *nd, longword v, longword n, word intf)
{
printf("Insert %ld %08lX ", n, v);
switch(nd->type)
{ case NODE_EMPTY: printf("E\n"); break;
  case NODE_LEAF: printf("L %ld %08lX\n", nd->n, nd->v); break;
  case NODE_BRANCH: printf("B\n"); break;
}
  switch(nd->type)
    {
    case NODE_EMPTY:
      nd->type = NODE_LEAF;
      nd->n = n;
      nd->v = v;
      nd->i = intf;
      break;
    case NODE_BRANCH:
      if (n) Insert(v & (1 << n-1) ? nd->r : nd->l, v, n - 1, intf);
      if (n)
        {
          if(v & (1 << n-1))
            printf("left %ld %08lX\n", n, v);
          else
           printf("right %ld %08lX\n", n, v);
        }
      break;

    case NODE_LEAF:
      if (n)
	{
	  nd->type = NODE_BRANCH;
	  nd->l = Empty();
	  nd->r = Empty();

          if(nd->v & (1 << nd->n-1))
            printf("%ld %08lX %08lX _1_", nd->n-1, nd->v, 1 << (nd->n-1));
	  else
            printf("%ld %08lX %08lX _0_", nd->n-1, nd->v, 1 << (nd->n-1));
	  Insert(nd->v & (1 << nd->n-1) ? nd->r : nd->l, nd->v, nd->n-1, nd->i);

          if(v & (1 << n-1))
            printf("%ld %08lX %08lX _1_", n-1, v, 1 << (n-1));
	  else
            printf("%ld %08lX %08lX _0_", n-1, v, 1 << (n-1));
	  Insert(v & (1 << n-1) ? nd->r : nd->l, v, n - 1, intf);
	}
    }
}

void Prefix()
{
  longword v, n;
  int loop, i;

  for (loop = 0; loop < tableSize; loop++)
    {
      v = ntohl(rTable[loop].addr);
      n = 32;
      i = rTable[loop].interface;
      Insert(prefixRoot, v, n, i);

      Dump(0, 0, prefixRoot);
      printf("++ total = %d leaves = %d branches & leaves = %d\n",
        Count(7, prefixRoot), Count(2, prefixRoot), Count(6, prefixRoot));
    } 
}

/* **********************************************************************
*
*
********************************************************************** */
tableRec *RouteSearch(int length, byte *addr)
{
  int i;
  long test;
/* ---------------------------------------------
-  Currently length is ignored and addr is changed to a long
--------------------------------------------- */
  test = *(long*)addr;
  for (i = 0; i < tableSize; i++)
    {
      if((test & rTable[i].mask) == rTable[i].addr)
	return &rTable[i];
    }
  return (tableRec*)0;
}

int RouteAddEntry(tableRec *entry)
{
  memcpy(rTable + tableSize, entry, sizeof(tableRec));
  tableSize++;
  return 0;
}

/* **********************************************************************
*
*  RoutedHandle
*
*  Handles incoming routed and RIP packets and is suppose to build the routing
*  tables from them using the vector-distance algorithm.
*
*  Currently suffers from the problem that RIP does not include subnet masks
*  for each of its IP addresses so have to work backwards by eight bits at a
*  time trying to find the first non zero octet.
*
*  The input is a pointer to an IP packet.  The Routed information is
*  encapsulated in a UDP packet with port# 520.
*
********************************************************************** */
void RoutedHandle(ipHdr *ip, tableRec *rTblEntry)
{
  udpHdr *udp;
  ripHdr *rip;
  ripEntry *entry;

  long mask, addr;
  int nRoutes, i, j, k, result;
  tableRec *route, newRoute;

  udp = (udpHdr*)&ip[1];
  rip = (ripHdr*)&udp[1];
  entry = (ripEntry*)&rip[1];
  nRoutes = (ntohs(udp->length) - sizeof(ripHdr)) / sizeof(ripEntry);

  for (i = 0; i < nRoutes; i++)
    {
      route = RouteSearch(4, (byte*)entry[i].ipAddr);
      if(route)
	{
	  if(++entry[i].metric < route->metric)
	    {
	      route->metric = entry[i].metric;
	      route->interface = rTblEntry->interface;
	      memcpy(&route->nextHop, &ip->source, 4);
	    }
	}
      else
	{
	  memcpy(&newRoute.addr, entry[i].ipAddr, 4);
	  memcpy(&newRoute.nextHop, &ip->source, 4);
	  newRoute.interface = rTblEntry->interface;
	  newRoute.metric = entry[i].metric + 1;
	  newRoute.flags = 0;
	  mask = 0xFFFFFFFF;
	  for(j = 0; j < 4; j++)
	    {
	      if(entry[i].ipAddr[j])
		mask = mask << 8;
	      else
		break;
	    }
	  mask = ~mask;
	  newRoute.mask = mask;
	  RouteAddEntry(&newRoute);
	}
    }
}

/* **********************************************************************
*
*  Forward Packet
*
********************************************************************** */
void ForwardPacket(ipHdr *ip, tableRec *route)
{
  if (route->flags & flagLocal)
    ARPResolve(route->interface, ip->length, ip, 4, (byte*)&ip->destination, FALSE);
  else
    ARPResolve(route->interface, ip->length, ip, 4, (byte*)&route->nextHop, FALSE);
}

/* **********************************************************************
*
*  LocalPacket
*
********************************************************************** */
void LocalPacket(ipHdr *ip, tableRec *route)
{
  int result;

  switch(ip->proto)
    {
    case ipPrICMP:
      {
	result = ICMPHandle(ip);
/* ----------------------------------------------------------------------
-  if result == 1 then there is a packet to send, else ignore
---------------------------------------------------------------------- */
	if (result == 1)
	  {
	    ip->checksum = 0;
	    ip->checksum = ~checksum(ip, sizeof(ipHdr));
	    ForwardPacket(ip, route);
	  }
	break;
      }

    case ipPrTCP:
      {
	printf("TCP packet\n");
	break;
      }

    case ipPrUDP:
      {
	udpHdr *udp;
	int i;    
	udp = (udpHdr*)&ip[1];

/* ignore checksum */
/* 7/4/93 Add hack for routed packets */

	if(udp->destination == htons(UDProuted))
	  RoutedHandle(ip, route);
	break; 
      }
    default:
      break;
    }
}

/* **********************************************************************
*
*  RouteIdle
*
********************************************************************** */
void RouteIdle()
{
  register time_t checkClock;
  int i, size, count;
  register int j, k; 
  byte packet[1800]; /* should be changed */
  ipHdr *ip;
  udpHdr *udp;
  ripHdr *rip;
  ripEntry *entry;

  checkClock = time((time_t*)0);
  if ((checkClock - nextUpdate) > UPDATE_FREQ)
    {
      ip = (ipHdr*)packet;
      udp = (udpHdr*)(ip + 1);
      rip = (ripHdr*)(udp + 1);
      entry = (ripEntry*)(rip + 1);
      for(i = 0; i < pd->nIntf; i++)
	{
	  count = 0;
	  for(j = 0; j < tableSize; j++)
	    {
	      if((rTable[j].flags & (flagLocal | flagThisHost)) == flagLocal &&
		 rTable[j].interface != i)
		{
		  for (k = 0; k < sizeof(ripEntry); k++)
		    ((byte*)entry)[k] = 0x00;
		  entry->familyID = htons(RIP_IP_FAMILY_ID); /* ip family */
		  *((long*)entry->ipAddr) = rTable[j].addr;
		  entry->metric = htonl(rTable[j].metric);
		  entry++;
		  count++;
		}
	    }
	  rip->command = 2; /* ripResponse */
	  rip->version = 1; /* the only current version */
	  rip->filler1 = 0;

	  udp->source = udp->destination = htons(UDProuted);
	  udp->length = htons(count * sizeof(ripEntry) + sizeof(udpHdr) +
			      sizeof(ripHdr));
	  udp->checksum = 0;
/*
      udp->checksum = ~checksum(udp, ntohs(udp->length));
*/ 
	  ip->ver = 4;
	  ip->hdrlen = 5;
	  ip->tos = 0;
	  ip->length = htons(sizeof(ipHdr) + ntohs(udp->length));
	  ip->identification = 0;
	  ip->fragment = 0;
	  ip->ttl = 2;
	  ip->proto = ipPrUDP;
	  ip->checksum = 0;
	  ip->source = rTable[i].addr;
	  ip->destination = rTable[i+2].addr;
	  ip->checksum = ~checksum(ip, sizeof(ipHdr));
     	  ARPResolve(i, ip->length, ip, 4, (byte*)&ip->destination, TRUE);
	}
      nextUpdate = time((time_t*)0);
    }
}

/* **********************************************************************
*
*  RouteInit
*
********************************************************************** */
void RouteInit()
{
  tableSize = 0;
  rTable = malloc(sizeof(tableRec) * TABLE_MAX);
  nextUpdate = time((time_t*)0) - UPDATE_FREQ;
  prefixRoot = Empty();
}
void RouteShutdwon() {}

/* **********************************************************************
*
*  IPCallback
*
*  Called by ARP.  If there are then it checks the routing table to match the
*  destination address.  If the address is local to the host send it directly
*  to the destination otherwise forward the packet to the next hop.  If packet
*  is destined for this host then process the packet as a standard host.
*
********************************************************************** */
void IPCallback(int intf, int length, byte *pkt)
{
  ipHdr *ip;
  tableRec *route;

/* ----------------------------------------------------------------------
-  Calculate checksum, if not correct then trash packet	and continue.
-  Check routing table for destination address.
---------------------------------------------------------------------- */
  ip = (ipHdr*)pkt;

printf("%1d", intf);

  route = RouteSearch(4, (byte*)&ip->destination);
  if (route)
    {
      if(route->flags & flagLocal) /* host on directly connected */
	if (route->flags & (flagThisHost | flagBroadcast))
	  LocalPacket(ip, route);
	else
	  ForwardPacket(ip, route);
      else
	ForwardPacket(ip, route);
    }
}

/* **********************************************************************
*
*  IPInit
*
********************************************************************** */
#define N_INTF 2

longword ipAddrList[] = {
  0x82D82196, /* 130.216.33.150 */
  0x82d8F901  /* 130.216.249.1  */
};

void IPInit(PDRec *param)
{
  int i;
  tableRec local[MAX_INTF], host[MAX_INTF], bcast[MAX_INTF];
  
  pd = param;
  for (i = 0; i < N_INTF; i++)
    {
      local[i].mask = htonl(0xFFFFFF00);
      host[i].mask = bcast[i].mask = htonl(0xFFFFFFFF);
      local[i].metric = host[i].metric = bcast[i].metric = 1;

      local[i].flags = flagLocal;
      host[i].flags = flagLocal | flagThisHost;
      bcast[i].flags = flagLocal | flagBroadcast;

      host[i].addr = htonl(ipAddrList[i]) & host[i].mask;
      local[i].addr = htonl(ipAddrList[i]) & local[i].mask;
      bcast[i].addr = local[i].addr | ~local[i].mask;

      host[i].interface = local[i].interface = bcast[i].interface = i;
    }

  RouteInit();

  for (i = 0; i < N_INTF; i++)
    RouteAddEntry(&host[i]);
  for (i = 0; i < N_INTF; i++)
    RouteAddEntry(&bcast[i]);
  for (i = 0; i < N_INTF; i++)
    RouteAddEntry(&local[i]);

  for (i = 0; i < N_INTF; i++)
    ARPAddAddress(i, 4, (byte*)&host[i].addr, IPCallback);
}

/* **********************************************************************
*
*  IPShutdown
*
********************************************************************** */
void IPShutdown()
{
  ARPRemoveAddress(0);
  ARPRemoveAddress(1);

  printf("\n");
  RouteDump();
}
