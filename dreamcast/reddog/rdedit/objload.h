#ifndef _OBJLOAD_H
#define _OBJLOAD_H

#include <max.h>
#include <stdio.h>

/*
 * Routines for saving and loading mesh files
 */

int ObjSave (FILE *output, IScene *scene, Interface *i);
int ObjLoad (char *infile, Mesh &mesh, int Mode);
extern void BoxMake(Mesh &mesh,float minX,float minY,float minZ,float maxX,float maxY, float maxZ);

#endif