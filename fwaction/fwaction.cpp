// fwactions.cpp : Defines the entry point for the DLL application. 
// 

#include "stdafx.h" 

// mandatory FW includes 
#include "../fwlib/factory.h"
#include "../fwlib/fwunknown.h"
#include "common_i.c" // for IID_IFWUnknown 

// CLSID definitions 
#include "fwaction_i.c" 

// interfaces and IID definitions 
#include "actionplus.h" 
#include "actionplus_i.c" 

// Class implementation 
#include "action.h"
#include "rotate.h" 
#include "move.h" 
#include "reach.h" 
#include "manip.h"
#include "bodymove.h"
#include "wait.h"
#include "walk.h"
#include "detcoll.h" 

// extra IID definitions
#include "transplus_i.c" 
#include "kineplus_i.c" 
#include "bodyplus_i.c" 

//////////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Factory Map - Used to create a factory for aech class 
// Add line for each class implemented 

BEGIN_FACTORY_MAP 
	FACTORY_CLASS(Action, L"Action Plus Generic Action", L"ActionPlus.Action", L"ActionPlus.Action.1")

	FACTORY_CLASS(ActionRotate, L"FreeWill+ Actions Rotate Action", L"Actions.Rotate", L"Actions.Rotate.1") 
    FACTORY_CLASS(ActionRotateInv, L"FreeWill+ Actions Rotate Inversed Action", L"Actions.RotateInv", L"Actions.RotateInv.1") 
    FACTORY_CLASS(ActionRotateTo, L"FreeWill+ Actions Rotate To Action", L"Actions.RotateTo", L"Actions.RotateTo.1") 
    FACTORY_CLASS(ActionMultiRotate, L"FreeWill+ Actions Multiple Rotate Action", L"Actions.MultiRotate", L"Actions.MultiRotate.1") 
    FACTORY_CLASS(ActionMultiRotateTo, L"FreeWill+ Actions Multiple Rotate To Action", L"Actions.MultiRotateTo", L"Actions.MultiRotateTo.1") 
    FACTORY_CLASS(ActionMove, L"FreeWill+ Actions Move Action", L"Actions.Move", L"Actions.Move.1") 
    FACTORY_CLASS(ActionMoveTo, L"FreeWill+ Actions Move To Action", L"Actions.MoveTo", L"Actions.MoveTo.1") 
    FACTORY_CLASS(ActionReach, L"FreeWill+ Actions Reach Action", L"Actions.Reach", L"Actions.Reach.1") 
    FACTORY_CLASS(ActionGrasp, L"FreeWill+ Actions Grasp Action", L"Actions.Grasp", L"Actions.Grasp.1") 
    FACTORY_CLASS(ActionTake, L"FreeWill+ Actions Take Action", L"Actions.Take", L"Actions.Take.1") 
    FACTORY_CLASS(ActionPoint, L"FreeWill+ Actions Point Action", L"Actions.Point", L"Actions.Point.1") 
    FACTORY_CLASS(ActionBend, L"FreeWill+ Actions Bend Action", L"Actions.Bend", L"Actions.Bend.1") 
    FACTORY_CLASS(ActionSquat, L"FreeWill+ Actions Squat Action", L"Actions.Squat", L"Actions.Squat.1") 
    FACTORY_CLASS(ActionSwing, L"FreeWill+ Actions Swing Action", L"Actions.Swing", L"Actions.Swing.1") 
    FACTORY_CLASS(ActionWait, L"FreeWill+ Actions Wait Action", L"Actions.Wait", L"Actions.Wait.1") 
    FACTORY_CLASS(ActionStep, L"FreeWill+ Actions Step Action", L"Actions.Step", L"Actions.Step.1") 
    FACTORY_CLASS(ActionTurn, L"FreeWill+ Actions Turn Action", L"Actions.Turn", L"Actions.Turn.1") 
    FACTORY_CLASS(ActionWalk, L"FreeWill+ Actions Walk Action", L"Actions.Walk", L"Actions.Walk.1") 
    FACTORY_CLASS(ActionDetColl, L"FreeWill+ Actions Collision Detection Action", L"Actions.DetColl", L"Actions.DetColl.1") 
END_FACTORY_MAP 

//////////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// FreeWill Map - Used to register each class to create its object by FWDevice 
// Add line for each class implemented 

BEGIN_FREEWILL_MAP 
	FREEWILL_CLASS(L"Action", L"Generic", Action)

	FREEWILL_CLASS(L"Action", L"Rotate", ActionRotate) 
    FREEWILL_CLASS(L"Action", L"RotateInv", ActionRotateInv) 
    FREEWILL_CLASS(L"Action", L"RotateTo", ActionRotateTo) 
    FREEWILL_CLASS(L"Action", L"MultiRotate", ActionMultiRotate) 
    FREEWILL_CLASS(L"Action", L"MultiRotateTo", ActionMultiRotateTo) 
    FREEWILL_CLASS(L"Action", L"Move", ActionMove) 
    FREEWILL_CLASS(L"Action", L"MoveTo", ActionMoveTo) 
    FREEWILL_CLASS(L"Action", L"Reach", ActionReach) 
    FREEWILL_CLASS(L"Action", L"Grasp", ActionGrasp) 
    FREEWILL_CLASS(L"Action", L"Take", ActionTake) 
    FREEWILL_CLASS(L"Action", L"Point", ActionPoint) 
    FREEWILL_CLASS(L"Action", L"Bend", ActionBend) 
    FREEWILL_CLASS(L"Action", L"Squat", ActionSquat) 
    FREEWILL_CLASS(L"Action", L"Swing", ActionSwing) 
    FREEWILL_CLASS(L"Action", L"Wait", ActionWait) 
    FREEWILL_CLASS(L"Action", L"Step", ActionStep) 
    FREEWILL_CLASS(L"Action", L"Turn", ActionTurn) 
    FREEWILL_CLASS(L"Action", L"Walk", ActionWalk) 
    FREEWILL_CLASS(L"Action", L"DetColl", ActionDetColl) 
END_FREEWILL_MAP 

//////////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// Mandatory initialisation of the DLL module 

#ifdef USE_MFC 
class CMyApp : public CWinApp 
{ 
    public: virtual BOOL InitInstance() { DllMainHelper(AfxGetInstanceHandle(), DLL_PROCESS_ATTACH); return TRUE; } 
}; 
CMyApp myApp; 
#else
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) 
{ 
    return DllMainHelper(hModule, reason); 
} 
#endif

