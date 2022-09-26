/*
 * util.cpp
 * Various miscellaneous utility functions
 */

#include "StdAfx.h"

void safeStrcpy (char *buffer, const char *src, int length)
{
	strncpy (buffer, src, length-1);
	buffer[length-1] = '\0';
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject *GetTriObjectFromNode(Interface *maxInterface, INode *node, int &deleteIt) {
	assert (maxInterface != NULL); // Check the context
	
	deleteIt = FALSE;
	Object *obj = node->EvalWorldState(maxInterface->GetTime()).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(maxInterface->GetTime(), 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) deleteIt = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}
