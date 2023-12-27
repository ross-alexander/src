#ifndef _common_
#define _common_

#include "queue.h"
#include <time.h>

#define MAX_INTF 6
#define MAX_PCOLS 6
#define ENET_LEN 6

#define MAX_ADDR_LEN 10

#define TRUE 1
#define true TRUE
#define FALSE 0
#define false FALSE

typedef unsigned long longword;		/* 32 bits */
typedef unsigned short word;		/* 16 bits */
typedef unsigned char byte;		/*  8 bits */

#define htons(x) intel16(x)
#define htonl(x) intel(x)
#define ntohs(x) intel16(x)
#define ntohl(x) intel(x)
#define checksum(p, len) inchksum(p, len)

/* ----------------------------------------------------------------------
-  packet driver defines
---------------------------------------------------------------------- */
#define devicePktDrvr 1

/* ----------------------------------------------------------------------
-  functionality defines
---------------------------------------------------------------------- */
#define funcUsable 0x01
#define funcUnicast 0x02
#define funcBroadcast 0x04
#define funcMulticast 0x08

/* ----------------------------------------------------------------------
-  Hardware type as defined by rfc1340 (Assigned numbers)
_  page 69 under "Address resolution protocols"
---------------------------------------------------------------------- */
#define hrdEthernet 1
#define hrdExprEthernet 2
#define hrdAmateurRadio 3
#define hrdProNET 4
#define hrdChaos 5
#define hrdIEEE802 6
#define hrdARCNET 7
#define hrdHyperchannel 8
#define hrdLanstar 9
#define hrdAutonet 10
#define hrdLocalTalk 11
#define hrdLocalNet 12
#define hrdUltraLink 13
#define hrdSDMS 14
#define hrdFrameRelay 15
#define hrdATM 16

/* ----------------------------------------------------------------------
-  Ethernet type field values as defined by rfc1340 (Assigned numbers) page ??.
---------------------------------------------------------------------- */
#define IP_ETHER_TYPE 0x0800
#define ARP_ETHER_TYPE 0x0806

/* ----------------------------------------------------------------------
-  packet driver type defintions
---------------------------------------------------------------------- */
typedef struct {
  int intf, index, flags;
  word type;
  word handle;
  int addrLen;
  byte addr[MAX_ADDR_LEN];
  byte bcastAddr[MAX_ADDR_LEN];
  queue rcvQ;
  long pktRcv, pktSent;
  } protoRec;

typedef struct {
  word vector;
  word PDNumber;
  int number;
  int functionality;
  int class;
  int version;
  int type;
  int usable;
  int nProto;
  int intfType, hardwareType, hdrLen, tlrLen, maxPktLen, functions;
  } intfRec;

typedef struct {
  int nIntf;
  intfRec intf[MAX_INTF];
  protoRec pcol[MAX_INTF][MAX_PCOLS];
} PDRec;

typedef struct {
  queueNode q;
  time_t stamp;
  struct {
    int length;
    int intf;
  } pd;
  struct {
    int retry;
    time_t time;
    int addrLen;
    byte addr[MAX_ADDR_LEN];
  } arp;
} bufferRec;

typedef struct {
  int addrLen;
  byte ipAddr[4];
  byte subnet[4];
} ipRec;

/* ----------------------------------------------------------------------
-  assembly prototypes
---------------------------------------------------------------------- */ 
extern unsigned long intel(unsigned long val);
extern unsigned int intel16(unsigned int val);
extern word inchksum(void *ptr, int len);

/* ----------------------------------------------------------------------
-  packet driver prototypes
---------------------------------------------------------------------- */ 
extern PDRec *PDInit();
extern protoRec *PDAddProtocol(int port, word type);
extern void PDRemoveProtocol(protoRec *protocol);
extern bufferRec *PDCheckQueue(protoRec *prol);
extern void PDSendPacket(protoRec *pcol, bufferRec *buffer, int rtn);
extern bufferRec *GetBuffer();
extern void ReturnBuffer();

/* ----------------------------------------------------------------------
-  ARP prototypes
---------------------------------------------------------------------- */
/*
extern void ARPInit(pubPorts *public);
extern int ARPAddAddress(int port, ipRecord *ipStuff);
extern int ARPRemoveAddress(int port);
extern int ARPResolve(int port, word size, byte *buffer);
extern void ARPShutdown();
extern void ARPIdle();
*/
/* ----------------------------------------------------------------------
-	IP prototypes
---------------------------------------------------------------------- */
extern void IPInit(PDRec *param);
extern void IPShutdown();
/*
extern void IPIdle();
extern void IPSend(int port, byte *buffer);
*/
/* ----------------------------------------------------------------------
-	ICMP prototypes
---------------------------------------------------------------------- */
/*
extern int ICMPHandle(ipHeader *packet);
*/
#endif
