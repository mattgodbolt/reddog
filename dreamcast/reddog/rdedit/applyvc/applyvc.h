/**********************************************************************
 *<
	FILE:		    ApplyVC.h
	DESCRIPTION:	Vertex Color Utility
	CREATED BY:		Christer Janson
	HISTORY:		Created Monday, June 02, 1997

 *>	Copyright (c) 1997 Kinetix, All Rights Reserved.
 **********************************************************************/

#ifndef __APPLYVC__H
#define __APPLYVC__H

#include "Max.h"
#include "..\resource.h"
#include "utilapi.h"
#include "istdplug.h"
#include "EvalCol.h"

#define APPLYVC_UTIL_CLASS_ID	Class_ID(0x6e989199, 0x5dfb41b0)
#define APPLYVC_MOD_CLASS_ID	Class_ID(0x104170c9, 0x66373200)

class ModInfo {
public:
	Modifier*	mod;
	INodeTab	nodes;
};

typedef Tab<ModInfo*> ModInfoTab;
class ProgCallback;

class ApplyVCUtil : public UtilityObj {
	public:
		IUtil	*iu;
		Interface	*ip;
		HWND	hPanel;
		int		lastSelection;

		ApplyVCUtil();
		~ApplyVCUtil();

		void	BeginEditParams(Interface *ip,IUtil *iu);
		void	EndEditParams(Interface *ip,IUtil *iu);
		void	DeleteThis() {}

		void	Init(HWND hWnd);
		void	Destroy(HWND hWnd);

		void	ApplySelected();
		BOOL	ApplyNode(INode* node, int lightModel, ProgCallback *foo = NULL);
		Modifier*	GetModifier(INode* node, Class_ID modCID);
		void	UpdateAll();
		BOOL	UpdateAllEnum(INode* node, int lightModel);

		void	CheckForAndMakeUnique(int lightModel);
		void	CheckAllNodesEnum(INode* node, ModInfoTab& modTab);
		BOOL	MakeUnique(ModInfo* mh, int lightModel);
};

class ApplyVCMod : public Modifier {	
	public:
		static IObjParam*	ip;
		HWND			hPanel;
		ColorTab		mixedVertexColors;
		VertexColorTab	vertexColors;
		Interval		iValid;

		ApplyVCMod(BOOL create);
		~ApplyVCMod() { ResetColTab(); }
		void		InitControl(ModContext &mc,TriObject *obj,int type,TimeValue t);

		// From Animatable
		void		DeleteThis();
		void		GetClassName(TSTR& s);
		Class_ID	ClassID();
		void		BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
		void		EndEditParams( IObjParam *ip,ULONG flags,Animatable *next);
		TCHAR*		GetObjectName();
		CreateMouseCallBack*	GetCreateMouseCallBack();
		BOOL		CanCopyAnim();
		BOOL		DependOnTopology(ModContext &mc);

		ChannelMask	ChannelsUsed();
		ChannelMask	ChannelsChanged();
		Class_ID	InputType();
		void		ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
		Interval	LocalValidity(TimeValue t);

		int			NumRefs();
		RefTargetHandle	GetReference(int i);
		void		SetReference(int i, RefTargetHandle rtarg);

		int			NumSubs();
		Animatable* SubAnim(int i);
		TSTR		SubAnimName(int i);

		RefTargetHandle	Clone(RemapDir& remap = NoRemap());
		RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);

		IOResult	Load(ILoad *iload);
		IOResult	Save(ISave *isave);

		void		ResetColTab();
		void		SetColors(VertexColorTab& colorTab);
		void		SetMixedColors(ColorTab& colorTab);
};

extern ClassDesc*	GetApplyVCUtilDesc();
extern ClassDesc*	GetApplyVCModDesc();
extern HINSTANCE	hInstance;
extern TCHAR*		GetString(int id);
extern TriObject*	GetTriObjectFromNode(INode *node, TimeValue t, int &deleteIt);

#endif // __APPLYVC__H
