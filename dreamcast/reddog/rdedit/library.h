/*
 * Library.h
 * Templatised generic library of 'objects'
 */

#ifndef _LIBRARY_H
#define _LIBRARY_H

#include "util.h"

template <class T>
class Library : public Tab<T>
{
public:
	/*
	 * Find an object in the library
	 * Returns the index into the library, or -1 if the object was not found
	 */
	int FindObject (T *object)
	{
		for (int count = 0; count < Count(); ++count)
		{
			if (*object == (*this)[count])
				return count;
		}
		// Not found
		return -1;
	}
	/*
	 * Add an object to the library
	 * Returns the index into the library of the added value,
	 * or a previous index number if this object has already been
	 * inserted into the library
	 */
	int AddObject (T *object)
	{
		int result;
		result = FindObject (object);
		// Can we find the object?
		if (result != -1)
			return result;
		// Add the object to the library
		Append (1, object, Count());
		return (Count() - 1);
	}
};

#endif
