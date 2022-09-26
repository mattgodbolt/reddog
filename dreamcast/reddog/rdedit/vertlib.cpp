/*
 * VertLib.cpp
 * Hash list of vertices
 */

#include "stdafx.h"
#include "VertLib.h"

unsigned int VertexLibrary::hash (PosAndCol &col) const
{
	union {
		float Float;
		unsigned int Int;
	} foo;
	unsigned int retVal = 0;

	foo.Float = col.p.x; retVal ^= foo.Int;
	foo.Float = col.p.y; retVal ^= foo.Int;
	foo.Float = col.p.z; retVal ^= foo.Int;
	retVal ^= col.col;

	return (retVal % HASH_SIZE);
}

int VertexLibrary::Add (PosAndCol &col, bool collisionable /*= false*/)
{
	unsigned int hVal = hash (col);
	for (int i = 0; i < hashTable[hVal].Count(); ++i) {
		if (repArray[hashTable[hVal][i]].p == col) {
			if (collisionable)
				repArray[hashTable[hVal][i]].coll = true;
			return hashTable[hVal][i];
		}
	}
	IntRep rep;
	rep.p = col;
	rep.coll = collisionable;
	int id = repArray.Append(1, &rep, repArray.Count());
	hashTable[hVal].Append (1, &id, hashTable[hVal].Count());
	return id;
}

int __cdecl comparer (const void *u, const void *v)
{
	const IntRep *U = (const IntRep *)u, *V = (const IntRep *)v;
	if (U->coll) {
		if (V->coll) {
			return 0;
		} else {
			return -1;
		}
	} else {
		if (V->coll) {
			return 1;
		} else {
			return 0;
		}
	}
}

void VertexLibrary::Prep ()
{
	int i;
	size = repArray.Count();
	// If we have more than 65k verts...
	if (repArray.Count() > 65535) {
		// Sort the repArray by collisionable-ness
		repArray.Sort (comparer);
		// Now to count the amount, and re-create the hash table
		for (i = 0; i < HASH_SIZE; ++i)
			hashTable[i].ZeroCount();
		for (i = 0; i < size; ++i) {
			unsigned int hVal = hash (repArray[i].p);
			hashTable[hVal].Append (1, &i, hashTable[hVal].Count());
		}
	}
}

int VertexLibrary::Find (PosAndCol &col) const
{
	if (repArray.Count() == 0)
		return -1;
	unsigned int hVal = hash (col);
	for (int i = 0; i < hashTable[hVal].Count(); ++i) {
		if (repArray[hashTable[hVal][i]].p == col)
			return hashTable[hVal][i];
	}
	return -1;
}

// Default ambient value here
#define DEFAULT_AMT 0.6f

void VertexLibrary::Output (FILE *f, Tab<IShadBox *> &shadowBox)
{
	for (int j = 0; j < repArray.Count(); ++j) {
		PosAndCol col = repArray[j].p;
		float amt = DEFAULT_AMT;
		float pri = 0.f;
		for (int box = 0; box < shadowBox.Count(); ++box) {
			if (shadowBox[box]->InBox(col.p)) {
				if (shadowBox[box]->Priority() > pri) {
					pri = shadowBox[box]->Priority();
					amt = shadowBox[box]->Value();
				}
			}
		}
		// Munge the vertex
		float r, g, b;
		r = ((col.col & 0xff0000)>>16) * (1.f / 256.f);
		g = ((col.col & 0x00ff00)>>8) * (1.f / 256.f);
		b = ((col.col & 0x0000ff)>>0) * (1.f / 256.f);
		r = r * (1.f - amt) + amt;
		g = g * (1.f - amt) + amt;
		b = b * (1.f - amt) + amt;
		col.col = (col.col & 0xff000000) |
			(((unsigned char)(r*255.f))<<16) |
			(((unsigned char)(g*255.f))<<8) |
			(((unsigned char)(b*255.f))<<0);
		fwrite (&col, 1, sizeof (PosAndCol), f);
	}
}
