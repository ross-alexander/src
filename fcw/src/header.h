// 	===============================================================
// 	Header.CPY - Header InfoBlock
// 	===============================================================
// 	Copyright 1999 Evolution Computing
// 	All rights reserved
// 	Written by Mike Riddle 30-July-1999
// 	===============================================================

#ifndef _HEADER_H_
#define _HEADER_H_ 


#define IB_HDR			0
#define IB_HDR_VER		4
#define DB_VER			0

#pragma pack(1)

typedef struct {
  int	ERLen;
  char	EType;
  char	IType;
} InfoBlock;

typedef struct
{
  int	ERLen;
  char	EType;
  char	IType;
  short	Creator;
  int	AllocLen;
  char	HdrVersion;
  char	DBVer;
  short	DwgFlags;
  short	FStyle;
  short	Color;
  short	Layer;
  short	LStyle;
  short	WPlane;
  short	DStyle;
  short	BColor;
  short	GridID;
  GPOINT3 Low;		// lowest x,y,z extents coordinate
  GPOINT3 Hi;   	// highest x,y,z extents coordinate
  char	XtValid;
  char	XtGrow;
  char	PThick;
  char	AngFmt;
  float	UnitV;
  char	blank1[1];
  char	DecP;
#ifdef X
  unsigned char   FRnd        As Byte
    ReservedF   As Byte
    TrackType   As Long
    TSpec       As String * 22
    LastGroupID As Long
    ArwStyle    As Long
    ArwLen      As Single
    ArwHgt      As Single
    CurWidth    As Single
    LastTag     As Currency
    WallWidth   As Single
    WallLimit   As Single
    ReservedV   As Currency
    ReservedA   As String * 128
#endif
} HDR;


typedef struct
{
  int	ERLen;
  char	EType;
  char	IType;
  short	Creator;
  int	AllocLen;
  short	IBVer;
  int   WData[256];
} CMAP;

typedef struct {
  unsigned int	ERLen;
  unsigned char	EType;
  unsigned char	EFlags;
  unsigned char	EFlags2;
  unsigned char	EColor;
  unsigned char	EColor2;
  unsigned char	EThick;
  short		EWPlane;
  short		ELayer;
  unsigned short ELStyle;
  short		GroupID;
  unsigned short EFStyle;
  float	LWidth;
  int	Tag;
} Common;

typedef struct {
  unsigned int	ERLen;
  unsigned char	EType;
  unsigned char	EFlags;
  unsigned char	EFlags2;
  unsigned char	EColor;
  unsigned char	EColor2;
  unsigned char	EThick;
  short		EWPlane;
  short		ELayer;
  unsigned short ELStyle;
  short		GroupID;
  unsigned short EFStyle;
  float	LWidth;
  int	Tag;
  GPOINT2	Point;
} Point2;

typedef struct {
  unsigned int	ERLen;
  unsigned char	EType;
  unsigned char	EFlags;
  unsigned char	EFlags2;
  unsigned char	EColor;
  unsigned char	EColor2;
  unsigned char	EThick;
  short		EWPlane;
  short		ELayer;
  unsigned short ELStyle;
  short		GroupID;
  unsigned short EFStyle;
  float	LWidth;
  int	Tag;
  GLINE2	Line;
} Line2;

typedef struct
{
  unsigned int	ERLen;		// 0x00 00
  unsigned char	EType;		// 0x04 04
  unsigned char	EFlags;		// 0x05 05
  unsigned char	EFlags2;       	// 0x06 06
  unsigned char	EColor;		// 0x07 07
  unsigned char	EColor2;       	// 0x08 08
  unsigned char	EThick;		// 0x09 09
  short	EWPlane;       	// 0x0A 10
  short	ELayer;		// 0x0C 12
  unsigned short ELStyle;       	// 0x0E 14
  short	GroupID;       	// 0x10 16
  unsigned short	EFStyle;	// 0x12 18
  float	LWidth;		// 0x14 20
  int	Tag;		// 0x18 24

  unsigned char	SmType; // 0x1C 28
  unsigned char	SRes;   // 0x1D 29
  float	SParm;          // 0x1E 30
  float	EParm;          // 0x22 34
  unsigned short Count; // 0x26 38
  unsigned char	Flags;  // 0x28 40
  unsigned char	unused; // 0x29 41
  GPOINT2 Nodes[];      // 0x2A 42
} Path2;

typedef struct {
  unsigned int	ERLen;		// 0x00 00
  unsigned char	EType;		// 0x04 04
  unsigned char	EFlags;		// 0x05 05
  unsigned char	EFlags2;       	// 0x06 06
  unsigned char	EColor;		// 0x07 07
  unsigned char	EColor2;       	// 0x08 08
  unsigned char	EThick;		// 0x09 09
  short	EWPlane;       	// 0x0A 10
  unsigned short	ELayer;		// 0x0C 12
  short ELStyle;       	// 0x0E 14
  short	GroupID;       	// 0x10 16
  unsigned short	EFStyle;	// 0x12 18
  float	LWidth;		// 0x14 20
  int	Tag;		// 0x18 24

  GCIR2 Circle;
} Cir2;

typedef struct {
  unsigned int	ERLen;		// 0x00 00
  unsigned char	EType;		// 0x04 04
  unsigned char	EFlags;		// 0x05 05
  unsigned char	EFlags2;       	// 0x06 06
  unsigned char	EColor;		// 0x07 07
  unsigned char	EColor2;       	// 0x08 08
  unsigned char	EThick;		// 0x09 09
  short	EWPlane;       	// 0x0A 10
  unsigned short	ELayer;		// 0x0C 12
  short ELStyle;       	// 0x0E 14
  short	GroupID;       	// 0x10 16
  unsigned short	EFStyle;	// 0x12 18
  float	LWidth;		// 0x14 20
  int	Tag;		// 0x18 24

  GARC2 Arc;
} Arc2;

#define NL_CLS  0x80  // 1 = close node list (polygon)
#define NL_SLD  0x40  // 1 = interior is solid (filled)

typedef struct {
  unsigned int	ERLen;		// 0x00 00
  unsigned char	EType;		// 0x04 04
  unsigned char	EFlags;		// 0x05 05
  unsigned char	EFlags2;       	// 0x06 06
  unsigned char	EColor;		// 0x07 07
  unsigned char	EColor2;       	// 0x08 08
  unsigned char	EThick;		// 0x09 09
  short	EWPlane;       	// 0x0A 10
  unsigned short	ELayer;		// 0x0C 12
  short ELStyle;       	// 0x0E 14
  short	GroupID;       	// 0x10 16
  unsigned short	EFStyle;	// 0x12 18
  float	LWidth;		// 0x14 20
  int	Tag;		// 0x18 24

  GELP2 Circle;
} Elp2;


typedef struct {
  unsigned int	ERLen;		// 0x00 00
  unsigned char	EType;		// 0x04 04
  unsigned char	EFlags;		// 0x05 05
  unsigned char	EFlags2;       	// 0x06 06
  unsigned char	EColor;		// 0x07 07
  unsigned char	EColor2;       	// 0x08 08
  unsigned char	EThick;		// 0x09 09
  short	EWPlane;       	// 0x0A 10
  unsigned short	ELayer;		// 0x0C 12
  short ELStyle;       	// 0x0E 14
  short	GroupID;       	// 0x10 16
  unsigned short	EFStyle;	// 0x12 18
  float	LWidth;		// 0x14 20
  int	Tag;		// 0x18 24

  GELA2 Circle;
} Ela2;


typedef struct
{
  unsigned int	ERLen;		// 0x00 00
  unsigned char	EType;		// 0x04 04
  unsigned char	EFlags;		// 0x05 05
  unsigned char	EFlags2;       	// 0x06 06
  unsigned char	EColor;		// 0x07 07
  unsigned char	EColor2;       	// 0x08 08
  unsigned char	EThick;		// 0x09 09
  short	EWPlane;       	// 0x0A 10
  unsigned short	ELayer;		// 0x0C 12
  short ELStyle;       	// 0x0E 14
  short	GroupID;       	// 0x10 16
  unsigned short	EFStyle;	// 0x12 18
  float	LWidth;		// 0x14 20
  int	Tag;		// 0x18 24

  GPOINT3 Low;		// lowest x,y,z extents coordinate
  GPOINT3 Hi;   	// highest x,y,z extents coordinate

  unsigned int Flags;		// options/status flags
  char SName[32];	// ANSIZ symbol name
}SYMDEF;	

typedef struct
{
  int	ERLen;		// 0x00 00
  char	EType;		// 0x04 04
  char	EFlags;		// 0x05 05
  char	EFlags2;       	// 0x06 06
  unsigned char	EColor;		// 0x07 07
  unsigned char	EColor2;       	// 0x08 08
  char	EThick;		// 0x09 09
  short	EWPlane;       	// 0x0A 10
  short	ELayer;		// 0x0C 12
  short ELStyle;       	// 0x0E 14
  short	GroupID;       	// 0x10 16
  short	EFStyle;	// 0x12 18
  float	LWidth;		// 0x14 20
  int	Tag;		// 0x18 24
  GPOINT3 Low;		// lowest x,y,z extents coordinate
  GPOINT3 Hi;   	// highest x,y,z extents coordinate
 
  char SName[32];	// ANSIZ symbol name
  unsigned long DefAdr;   // run-time adrs of definition
  SYMTMAT TMat;		// SymDef -> World Coordinates TMAT
} SYMREF;

// 	Values for SWFlags are:

#define SW_Grid				1
#define SW_Snap				2
#define SW_CSnap			4
#define SW_Ortho			8
#define SW_Attach		0x10
#define SW_Unlock		0x20
#define SW_Depth		0x40
#define SW_SheetAH		0x80

// 	===============================================================

#endif
