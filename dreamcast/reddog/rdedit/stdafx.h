// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0319A382_C0F8_11D2_BE00_00104B47455E__INCLUDED_)
#define AFX_STDAFX_H__0319A382_C0F8_11D2_BE00_00104B47455E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "max.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "RDEdit.h"
#include "Library.h"
#include "MatLib.h"
#include "RDObject.h"
#include "MeshSupt.h"
#include "Normals.h"
#include "BBox.h"
#include "ModelSave.h"
#include "RDStratMap.h"
#include "RedDog.h"

#include "HelpBase.h"
#include "resource.h"
#include "util.h"
#include "animcontrol.h"
#include "Objload.h"
#include "RDScape.h"
#include "Strip.h"
#include "Preview.h"
#include "Material.h"
// horrendous hack :
#define OPENFILENAME int
#include <stdmat.h>
#include <bmmlib.h>

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0319A382_C0F8_11D2_BE00_00104B47455E__INCLUDED_)
