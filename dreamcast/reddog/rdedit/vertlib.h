/*
 * VertLib.h
 * Speed up vertex library production
 */
#ifndef VERTLIB_H
#define VERTLIB_H

#include "ShadBox.h"

#define HASH_SIZE 1021

struct IntRep {
	PosAndCol	p;
	bool		coll;
};

class VertexLibrary
{
private:
	Tab<int> hashTable[HASH_SIZE]; // points into repArray
	Tab<IntRep> repArray;

	int size;

	unsigned int hash (PosAndCol &point) const;
public:
	VertexLibrary() {}
	~VertexLibrary() {}
	
	int Add (PosAndCol &point, bool collisionable = false); // Add a point

	void Prep ();	// Called after all points registered

	int Find (PosAndCol &point) const;
	void Output (FILE *f, Tab<IShadBox *> &shadowBox);

	int Size () const { return size * sizeof (PosAndCol); }
	int Count() const { return size; }
	PosAndCol *Addr(int i)
	{
		return &repArray[i].p;
	}
};

#endif
