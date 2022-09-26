/*
 * Preview.h
 */
#include "utilapi.h"

#ifndef _PREVIEW_H
#define _PREVIEW_H
class Preview : public UtilityObj
{
private:
	static HWND		paramWindow;
	Interface		*iface;
	ICustEdit		*edit;
	ICustEdit		*saveParams[4];
	friend BOOL CALLBACK PreviewParamProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
public:
	void nameupdate (TCHAR* name);
	Preview() : iface(NULL) {}
	void BeginEditParams (Interface *ip, IUtil *iu);
	void EndEditParams (Interface *ip, IUtil *iu);
	void DeleteThis() { delete this; }
};


extern ClassDesc *GetPreviewDesc();
extern INode *NamedNode;
extern void FindNamedNode(INode *node,char *name);

extern Preview* LEVNAMEBOX;
#endif
