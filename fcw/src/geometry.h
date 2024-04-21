// 	===============================================================
// 	GEOMETRY.CPY - Real Geometry Structures
// 	===============================================================
// 	Copyright 1999 Evolution Computing
// 	All rights reserved
// 	Written by Mike Riddle 20-July-1999
// 	===============================================================
//  Converted to C-Style 28-Feb-2000
//

#ifndef _GEOMTRY_H_
#define _GEOMTRY_H_

        
typedef struct 
{
	float x;
	float y;
} GPOINT2;

typedef struct
{
	float x;
	float y;
	float z;
} GPOINT3;

typedef struct
{
	GPOINT2 p1;
	GPOINT2 p2;
} GLINE2;

typedef struct
{
	GPOINT2 Low;
	GPOINT2 High;
} EXTENTS2;

typedef struct
{
	GPOINT3 p1;
	GPOINT3 p2;
} GLINE3;

typedef struct 
{
	GPOINT2 Center;
	float Radius;
} GCIR2;

typedef struct
{
	GCIR2 Circle;
	float SAng;
	float AngW;
} GARC2;

typedef struct
{
	GCIR2 Circle;
	float Ecc;
	float Incl;
} GELP2;

typedef struct
{
	GELP2 Elp;
	float SAng;
	float AngW;
} GELA2;

// 	===============================================================

typedef struct    	// SYMBOL TMat (last always [0 0 0 1])
{
	float m11;    		// first column
	float m21;
	float m31;
	float m41;
	float m12;    		// second column
	float m22;
	float m32;
	float m42;
	float m13;		    // third column
	float m23;
	float m33;
	float m43;
}SYMTMAT;

#endif
