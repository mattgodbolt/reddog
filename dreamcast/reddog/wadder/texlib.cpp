// *burp*
//

#include "stdafx.h"
#include "Wadder.h"
#include "WadderDlg.h"
#include "TexLib.h"

TexLib TexLib::SystemTexList;
bool TexLib::DoneSystem = false;

void TexLib::Add (unsigned int GBIX, CString fileName)
{
	// Was it an invalid texture?
	if (GBIX == 0xffffffff)
		invalidTextures++;
	// Is it a 'magic' texture?
	if (GBIX == 0x013ef13e)
		return; // FireFire
	// Try and find the GBIX in the map
	CString string;
	if (SystemTexList.lib.Lookup (GBIX, string)) {
		// Its in the system texlist
		return;
	}
	if (lib.Lookup (GBIX, string))
		return;
	// Otherwise register a new jobby
	lib.SetAt (GBIX, fileName);
	if (!DoneSystem)
		SystemTexList.lib.SetAt (GBIX, fileName);
}

void TexLib::Reset (void)
{
	pos = lib.GetStartPosition();
}

bool TexLib::More (void)
{
	return (pos==NULL) ? false : true;
}

CString TexLib::GetElement (void)
{
	CString retVal;
	unsigned int i;
	lib.GetNextAssoc (pos, i, retVal);
	return retVal;
}

int TexLib::Count (void)
{
	return lib.GetCount();
}

void TexLib::Clear ()
{
	lib.RemoveAll();
}

void TexLib::SetAsSystem ()
{
	DoneSystem = true;
}