// matrix.h: CTransMatrix coclass
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_MATRIX__)
#define _MATRIX__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\fwlib\factory.h"
#include "..\fwlib\fwunknown.h"
#include "transplus.h"

class CTransMatrix : public FWUNKNOWN<ITransform, IID_ITransform, ITransform>
{
public:
	// orthonormality tests
	virtual HRESULT __stdcall IsOrthonormalQuick();
	virtual HRESULT __stdcall IsOrthonormal();
	virtual HRESULT __stdcall IsOrthonormalEx(FWDOUBLE *bIsOrthogonal, FWDOUBLE *bIsNormal);
	virtual HRESULT __stdcall Orthonormalize();

	// construction and convertion from other formats
	virtual HRESULT __stdcall FromIdentity();
	virtual HRESULT __stdcall FromTranslationXYZ(FWDOUBLE x, FWDOUBLE y, FWDOUBLE z);
	virtual HRESULT __stdcall FromTranslationVector(FWVECTOR *pV);
	virtual HRESULT __stdcall FromTranslationTransform(ITransform *pT);
	virtual HRESULT __stdcall FromRotationX(FWDOUBLE fTheta);
	virtual HRESULT __stdcall FromRotationTanX(FWDOUBLE fTan1, FWDOUBLE fTan2);
	virtual HRESULT __stdcall FromRotationSinCosX(FWDOUBLE fSin, FWDOUBLE fCos);
	virtual HRESULT __stdcall FromRotationY(FWDOUBLE fTheta);
	virtual HRESULT __stdcall FromRotationTanY(FWDOUBLE fTan1, FWDOUBLE fTan2);
	virtual HRESULT __stdcall FromRotationSinCosY(FWDOUBLE fSin, FWDOUBLE fCos);
	virtual HRESULT __stdcall FromRotationZ(FWDOUBLE fTheta);
	virtual HRESULT __stdcall FromRotationTanZ(FWDOUBLE fTan1, FWDOUBLE fTan2);
	virtual HRESULT __stdcall FromRotationSinCosZ(FWDOUBLE fSin, FWDOUBLE fCos);
	virtual HRESULT __stdcall FromRotationYawPitchRoll(FWDOUBLE fYaw, FWDOUBLE fPitch, FWDOUBLE fRoll);
	virtual HRESULT __stdcall FromRotationTanYawPitchRoll(FWDOUBLE fTan1Yaw, FWDOUBLE fTan2Yaw, FWDOUBLE fTan1Pitch, FWDOUBLE fTan2Pitch, FWDOUBLE fTan1Roll, FWDOUBLE fTan2Roll);
	virtual HRESULT __stdcall FromRotationSinCosYawPitchRoll(FWDOUBLE fSinYaw, FWDOUBLE fCosYaw, FWDOUBLE fSinPitch, FWDOUBLE fCosPitch, FWDOUBLE fSinRoll, FWDOUBLE fCosRoll);
	virtual HRESULT __stdcall FromRotationEuler(FWEULER *pEuler);
	virtual HRESULT __stdcall FromRotationAxisAngle(FWVECTOR *p, FWDOUBLE fAngle);
	virtual HRESULT __stdcall FromRotationQuat(FWQUAT *pQuat);
	virtual HRESULT __stdcall FromRotationTransform(ITransform *pT);
	virtual HRESULT __stdcall FromScaling(FWDOUBLE fScaleX, FWDOUBLE fScaleY, FWDOUBLE fScaleZ);
	virtual HRESULT __stdcall FromMatrix(FWMATRIX);
	virtual HRESULT __stdcall FromMatrix12(FWMATRIX12);
	virtual HRESULT __stdcall FromMatrix9(FWMATRIX9);
	virtual HRESULT __stdcall FromTransform(ITransform *pT);
	virtual HRESULT __stdcall FromInvertion(ITransform *pT);

	// Perspective and Similar Transformations
	virtual HRESULT __stdcall FromLookAtLH(FWVECTOR *pEye, FWVECTOR *pAt, FWVECTOR *pUp); 
	virtual HRESULT __stdcall FromLookAtRH(FWVECTOR *pEye, FWVECTOR *pAt, FWVECTOR *pUp); 
	virtual HRESULT __stdcall FromPerspectiveLH(FWDOUBLE fFOV, FWDOUBLE fZN, FWDOUBLE fZF, FWDOUBLE fAspect); 
	virtual HRESULT __stdcall FromPerspectiveRH(FWDOUBLE fFOV, FWDOUBLE fZN, FWDOUBLE fZF, FWDOUBLE fAspect); 

	// modifications
	virtual HRESULT __stdcall MulTranslationXYZ(FWDOUBLE x, FWDOUBLE y, FWDOUBLE z);
	virtual HRESULT __stdcall MulTranslationVector(FWVECTOR *pV);
	virtual HRESULT __stdcall MulTranslationTransform(ITransform*);
	virtual HRESULT __stdcall MulRotationX(FWDOUBLE fTheta);
	virtual HRESULT __stdcall MulRotationTanX(FWDOUBLE fTan1, FWDOUBLE fTan2);
	virtual HRESULT __stdcall MulRotationSinCosX(FWDOUBLE fSin, FWDOUBLE fCos);
	virtual HRESULT __stdcall MulRotationY(FWDOUBLE fTheta);
	virtual HRESULT __stdcall MulRotationTanY(FWDOUBLE fTan1, FWDOUBLE fTan2);
	virtual HRESULT __stdcall MulRotationSinCosY(FWDOUBLE fSin, FWDOUBLE fCos);
	virtual HRESULT __stdcall MulRotationZ(FWDOUBLE fTheta);
	virtual HRESULT __stdcall MulRotationTanZ(FWDOUBLE fTan1, FWDOUBLE fTan2);
	virtual HRESULT __stdcall MulRotationSinCosZ(FWDOUBLE fSin, FWDOUBLE fCos);
	virtual HRESULT __stdcall MulRotationYawPitchRoll(FWDOUBLE fYaw, FWDOUBLE fPitch, FWDOUBLE fRoll);
	virtual HRESULT __stdcall MulRotationTanYawPitchRoll(FWDOUBLE fTan1Yaw, FWDOUBLE fTan2Yaw, FWDOUBLE fTan1Pitch, FWDOUBLE fTan2Pitch, FWDOUBLE fTan1Roll, FWDOUBLE fTan2Roll);
	virtual HRESULT __stdcall MulRotationSinCosYawPitchRoll(FWDOUBLE fSinYaw, FWDOUBLE fCosYaw, FWDOUBLE fSinPitch, FWDOUBLE fCosPitch, FWDOUBLE fSinRoll, FWDOUBLE fCosRoll);
	virtual HRESULT __stdcall MulRotationEuler(FWEULER *pEuler);
	virtual HRESULT __stdcall MulRotationAxisAngle(FWVECTOR *p, FWDOUBLE fAngle);
	virtual HRESULT __stdcall MulRotationQuat(FWQUAT *pQuat);
	virtual HRESULT __stdcall MulRotationTransform(ITransform*);
	virtual HRESULT __stdcall MulScale(FWDOUBLE fScaleX, FWDOUBLE fScaleY, FWDOUBLE fScaleZ);
	virtual HRESULT __stdcall MulMatrix(FWMATRIX);
	virtual HRESULT __stdcall Multiply(ITransform *pT);
	virtual HRESULT __stdcall MultiplyEx(ITransform *pT, FWULONG mode);
	// special forms: m_hOrtho is not modified
//	HRESULT MulMatrixNO(FWMATRIX);
//	HRESULT MulMatrixNO12(FWMATRIX12 M12);
//	HRESULT MulMatrixNO9(FWMATRIX9);

	// conversions to other types
	virtual HRESULT __stdcall AsVector(FWVECTOR*);
	virtual HRESULT __stdcall AsMatrix(FWMATRIX);
	virtual HRESULT __stdcall AsMatrixT(FWMATRIX);
	virtual HRESULT __stdcall AsMatrix12(FWMATRIX12);
	virtual HRESULT __stdcall AsMatrix12T(FWMATRIX12);
	virtual HRESULT __stdcall AsMatrix9(FWMATRIX9);
	virtual HRESULT __stdcall AsMatrix9T(FWMATRIX9);
	virtual HRESULT __stdcall AsEuler(FWEULER*);
	virtual HRESULT __stdcall AsAxisAngle(FWVECTOR *p, FWDOUBLE *pfAngle);
	virtual HRESULT __stdcall AsQuat(FWQUAT*);
	virtual HRESULT __stdcall AsTransform(ITransform*);
	virtual HRESULT __stdcall AsInvertion(ITransform *pT);

	// operations
	virtual HRESULT __stdcall ApplyTo(FWVECTOR *pVector);
	virtual HRESULT __stdcall ApplyInvertionTo(FWVECTOR *pVector);
	virtual HRESULT __stdcall ApplyTranslationTo(FWVECTOR *pVector);
	virtual HRESULT __stdcall ApplyRotationTo(FWVECTOR *pVector);
	virtual HRESULT __stdcall Reset(BOOL bZeroRotScale, BOOL bZeroTranslate);
	virtual HRESULT __stdcall Transpose();
	virtual HRESULT __stdcall Inverse();
	virtual HRESULT __stdcall Interpolate(ITransform *pSrc, ITransform *pDest, FWDOUBLE fPhase);

	// implementation dependent items
	virtual HRESULT __stdcall GetItem(FWULONG nIndex, FWDOUBLE *pVal);
	virtual HRESULT __stdcall PutItem(FWULONG nIndex, FWDOUBLE newVal);

	DECLARE_FACTORY_CLASS(TransMatrix, Transform)
	FW_RTTI(TransMatrix)

protected:
	FWMATRIX A;			// four vectors (columns) each 4 rows long; A[0][3]=A[1][3]=A[2][3]=0; A[3][3]=1
	HRESULT m_hOrtho;	// cache value for orthonormality tests

public:
	CTransMatrix() : m_hOrtho(S_FALSE)	{ FromIdentity(); }
	~CTransMatrix()						{ }
};

#endif