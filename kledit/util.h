// util.h - utility functions

#ifndef _UTIL_H
#define _UTIL_H

#include <max.h>
#include <fstream.h>
#include <string.h>
#include "resource.h"

// Various maximum lengths
#define MAX_FNAME_SIZE 512
#define MAX_NAME_SIZE 128

#define MAX_SPLIT_POLYS_PER_NODE	40
#define MAX_POLYS_PER_LEAF			200

#define MAX_DEGREE					16

#ifdef ABS
#undef ABS
#endif
#define ABS(a) ((a)>0.0f?(a):-(a))

#define SQR(a) ((a)*(a))

#define COPLANAR_TOLERANCE	(double)(0.0001)
#define LEAF_CUTOFF			150

// Get a string
TCHAR *GetString(int id);

// Americans - can they spell?
typedef Color Colour;

// Copy not more than length-1 bytes from src to buffer, zero terminating on overflow
void safeStrcpy (char *buffer, const char *src, int length);

// Get a triangle object from a node
TriObject *GetTriObjectFromNode(Interface *maxInterface, INode *node, int &deleteIt);

// Clamps a value between a and c
template <class T>
inline T RANGE (T a, T b, T c)
{
	return (b < a)?a : (b > c)?c: b;
}

// Swap two values
template <class T>
inline void SWAP (T &a, T &b)
{
	T temp;
	temp = a; a = b; b = temp;
}

typedef unsigned long		Uint32;
typedef signed long			Sint32;
typedef unsigned short		Uint16;
typedef signed short		Sint16;
typedef unsigned char		Uint8;
typedef signed char			Sint8;

#endif