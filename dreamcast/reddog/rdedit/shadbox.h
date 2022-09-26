/*
 * Shadow box
 */
#ifndef _SHADBOX_H
#define _SHADBOX_H

#include "simpobj.h"

#define SHADBOX_CLASSID Class_ID(0x7ff462cf, 0xc734b412)

class IShadBox : public SimpleObject, public IParamArray {
public:
	virtual void SetMat (const ::Matrix3 &mat) = 0;
	virtual bool InBox(Point3 &p) = 0;
	virtual float Value() = 0;
	virtual float Priority() = 0;

};

#endif
