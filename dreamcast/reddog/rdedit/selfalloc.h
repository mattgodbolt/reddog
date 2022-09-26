/*
 * SelfAlloc.h
 * Template, self allocating class
 * Derive any fixed-size class from this
 * Has a private 'next' field that may be used
 * in its descendants freely
 * You must instantiate a Class *SelfAlloc<Class>::freeList = NULL;
 */

#ifndef _SELFALLOC_H
#define _SELFALLOC_H

#include <stddef.h>

template
class SelfAlloc <class Class>
{
private:
	static Class	*freeList;
	Class			*next;
public:
	void *operator new (size_t size)
	{
		if (freeList) {
			void *retVal = (void *)freeList;
			freeList = freeList->next;
			return retVal;
		}
		return malloc (size);
	}
	void operator delete (void *area)
	{
		Class *entry = (Class *)area;
		entry->next = freeList;
		freeList = entry;
	}
};

#endif
