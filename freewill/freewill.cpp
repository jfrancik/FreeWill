// freewill.cpp : Defines the entry point for the DLL application.
// 

#include "stdafx.h"

#include "device.h"
#include "matrix.h"
#include "kinebone.h"
#include "body.h"
#include "meshdict.h"
#include "mesh.h"
#include "material.h"
#include "bbox.h"
#include "bsphere.h"
#include "scene.h"
#include "sceneobj.h"
#include "sceneacc.h"
#include "fileloader.h"
#include "kinereader.h"
//#include "action.h"

#include "common_i.c"
#include "freewill_i.c"
#include "transplus_i.c"
#include "kineplus_i.c"
#include "bodyplus_i.c"
#include "meshplus_i.c"
#include "matplus_i.c"
#include "boundplus_i.c"
#include "sceneplus_i.c"
#include "fileplus_i.c"
//#include "actionplus_i.c"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Factory Map 

BEGIN_FACTORY_MAP
    FACTORY_CLASS(FWDevice, L"FreeWill+ Main Device", L"FreeWillPlus.Device", L"FreeWillPlus.Device.1")

    FACTORY_CLASS(TransMatrix, L"Trans Plus Matrix 3D Transformation", L"TransPlus.Matrix", L"TransPlus.Matrix.1")

	FACTORY_CLASS(KineBone, L"Kine Plus Bone 3D Object", L"KinePlus.Bone", L"KinePlus.Bone.1")

	FACTORY_CLASS(Body, L"Body Plus Body Abstract Layer", L"BodyPlus.Body", L"BodyPlus.Body.1")

	FACTORY_CLASS(MeshDictionary, L"Mesh Plus Bone Dictionary", L"MeshPlus.Dictionary", L"MeshPlus.Dictionary.1")
	FACTORY_CLASS(Mesh, L"Mesh Plus Mesh Object", L"MeshPlus.Mesh", L"MeshPlus.Mesh.1")

	FACTORY_CLASS(Material, L"Mat Plus Material Object", L"MatPlus.Material", L"MatPlus.Material.1")

	FACTORY_CLASS(BoundingBox, L"Bound Plus Bounding Box", L"BoundPlus.BBox", L"BoundPlus.BBox.1")
	FACTORY_CLASS(BoundingSphere, L"Bound Plus Bounding Sphere", L"BoundPlus.BSphere", L"BoundPlus.BSphere.1")

	FACTORY_CLASS(Scene, L"Scene Plus Animation Scene", L"ScenePlus.Scene", L"ScenePlus.Scene.1")
	FACTORY_CLASS(SceneObject, L"Scene Plus Meshed Object", L"ScenePlus.Object", L"ScenePlus.Object.1")
	FACTORY_CLASS(SceneLightPoint, L"Scene Plus Point Light", L"ScenePlus.LightPoint", L"ScenePlus.LightPoint.1")
	FACTORY_CLASS(SceneLightDir, L"Scene Plus Directional Light", L"ScenePlus.LightDir", L"ScenePlus.LightDir.1")
	FACTORY_CLASS(SceneLightSpot, L"Scene Plus Spot Light", L"ScenePlus.LightSpot", L"ScenePlus.LightSpot.1")
	FACTORY_CLASS(SceneCamera, L"Scene Plus Camera", L"ScenePlus.Camera", L"ScenePlus.Camera.1")

    FACTORY_CLASS(FileLoader, L"File Plus Loader & File Sink", L"FilePlus.FileLoader", L"FilePlus.FileLoader.1")
    FACTORY_CLASS_EX(KineReader, StdFileIn, L"File Plus Standard 3D/XML Input File", L"FilePlus.StdReader", L"FilePlus.StdReader.1")

//	FACTORY_CLASS(ActionVerb, L"Action Plus Verb", L"ActionPlus.Verb", L"ActionPlus.Verb.1")
//	FACTORY_CLASS(Action, L"Action Plus Generic Action", L"ActionPlus.Action", L"ActionPlus.Action.1")
END_FACTORY_MAP

BEGIN_FREEWILL_MAP
	FREEWILL_CLASS(L"Transform",		L"Generic",		TransMatrix)
	FREEWILL_CLASS(L"KineBone",			L"Generic",		KineBone)
	FREEWILL_CLASS(L"Body",				L"Generic",		Body)
	FREEWILL_CLASS(L"Mesh",				L"Generic",		Mesh)
	FREEWILL_CLASS(L"MeshDictionary",	L"Generic",		MeshDictionary)
	FREEWILL_CLASS(L"Material",			L"Generic",		Material)
	FREEWILL_CLASS(L"Bounding",			L"Generic",		BoundingBox)
	FREEWILL_CLASS(L"SceneObject",		L"Generic",		SceneObject)
	FREEWILL_CLASS(L"PointLight",		L"Generic",		SceneLightPoint)
	FREEWILL_CLASS(L"DirLight",			L"Generic",		SceneLightDir)
	FREEWILL_CLASS(L"SpotLight",		L"Generic",		SceneLightSpot)
	FREEWILL_CLASS(L"Camera",			L"Generic",		SceneCamera)
	FREEWILL_CLASS(L"Scene",			L"Generic",		Scene)
	FREEWILL_CLASS(L"FileLoader",		L"Generic",		FileLoader)
	FREEWILL_CLASS(L"FileIn",			L"Generic",		StdFileIn)
//	FREEWILL_CLASS(L"Action",			L"Generic",		Action)
	FREEWILL_CLASS(L"FileLoader",		L"3D",			FileLoader)
	FREEWILL_CLASS(L"FileLoader",		L"XML",			FileLoader)
	FREEWILL_CLASS(L"Bounding",			L"Box",			BoundingBox)
	FREEWILL_CLASS(L"Bounding",			L"Sphere",		BoundingSphere)
END_FREEWILL_MAP


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

