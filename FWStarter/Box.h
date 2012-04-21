// Box.h - AdVisuo Common Source File

#pragma once

#include "Vector.h"
#include <sstream>
#pragma warning( disable : 4244 )

struct BOX
{
private:
	AVVECTOR A;			// internal left front lower
	AVVECTOR B;			// internal right rear upper
	AVVECTOR A1, B1;	// external points (including wall thickness)

public:
	BOX()														{ this->A = this->A1 = Vector(0, 0, 0); this->B = this->B1 = Vector(0, 0, 0); }
	BOX(AVVECTOR A, AVVECTOR B)									{ this->A = this->A1 = A; this->B = this->B1 = B; }
	BOX(AVFLOAT x, AVFLOAT y, AVFLOAT z, AVFLOAT w, AVFLOAT d, AVFLOAT h)	{ this->A = this->A1 = Vector(x, y, z); this->B = this->B1 = Vector(x + w, y + d, z + h); }
	BOX(AVFLOAT x, AVFLOAT y, AVFLOAT w, AVFLOAT d)				{ this->A = this->A1 = Vector(x, y, 0); this->B = this->B1 = Vector(x + w, y + d, 0); }

	operator AVVECTOR&()	{ return A; }
	BOX &operator +=(AVVECTOR v)	{ Move(v); return *this; }
	BOX &operator -=(AVVECTOR v)	{ Move(-v.x, -v.y, -v.z); return *this; }
	BOX operator +(AVVECTOR v)		{ BOX box = *this; box += v; return box; }
	BOX operator -(AVVECTOR v)		{ BOX box = *this; box -= v; return box; }
	BOX &operator *=(AVFLOAT f)		{ Scale(f); return *this; }

	bool InBox(AVVECTOR &v)	{ return ((v.x >= Left() && v.x <= Right()) || (v.x <= Left() && v.x >= Right())) && ((v.y >= Front() && v.y <= Rear()) || (v.y <= Front() && v.y >= Rear())); }
	bool InBoxExt(AVVECTOR &v)	{ return ((v.x >= LeftExt() && v.x <= RightExt()) || (v.x <= LeftExt() && v.x >= RightExt())) && ((v.y >= FrontExt() && v.y <= RearExt()) || (v.y <= FrontExt() && v.y >= RearExt())); }

	bool InWidth(AVFLOAT x)		{ return (x >= Left() && x <= Right()) || (x <= Left() && x >= Right()); }
	bool InWidthExt(AVFLOAT x)	{ return (x >= LeftExt() && x <= RightExt()) || (x <= LeftExt() && x >= RightExt()); }
	bool InDepth(AVFLOAT y)		{ return (y >= Front() && y <= Rear()) || (y <= Front() && y >= Rear()); }
	bool InDepthExt(AVFLOAT y)	{ return (y >= FrontExt() && y <= RearExt()) || (y <= FrontExt() && y >= RearExt()); }
	bool InHeight(AVFLOAT z)	{ return (z >= Lower() && z <= Upper()) || (z <= Lower() && z >= Upper()); }
	bool InHeightExt(AVFLOAT z)	{ return (z >= LowerExt() && z <= UpperExt()) || (z <= LowerExt() && z >= UpperExt()); }

	// divides the bix into nXDiv x nYDiv checkerboard-like sections and returns indices of section corresponding to the given vector
	// -1 and nXDiv/nYDiv values reserved for vectors outside the box
	bool InBoxSection(AVVECTOR &v, AVLONG nXDiv, AVLONG nYDiv, AVLONG &nX, AVLONG &nY)
	{
		nX = (AVLONG)floor((v.x - Left()) / (Width() / (double)nXDiv)); 
		nY = (AVLONG)floor((v.y - Front()) / (Depth() / (double)nYDiv));
		nX = max(-1, min(nX, nXDiv)); 
		nY = max(-1, min(nY, nYDiv));
		return nX >= 0 && nX < nXDiv && nY >= 0 && nY < nYDiv;
	}
	// azimuth from the box centre towards the given point; if normalised; corners are at PI/4 + N*PI/2 as if aspect ratio was 1.0
	AVFLOAT InBoxAzimuth(AVVECTOR &v, bool bNormalise = false)
	{
		AVFLOAT f = 1; if (bNormalise) f = 1 / Aspect();
		return atan2(f * (v.x - CenterX()), v.y - CenterY());
	}

	AVFLOAT Left()		{ return A.x; }
	AVFLOAT Right()		{ return B.x; }
	AVFLOAT Front()		{ return A.y; }
	AVFLOAT Rear()		{ return B.y; }
	AVFLOAT Lower()		{ return A.z; }
	AVFLOAT Upper()		{ return B.z; }

	AVFLOAT CenterX()	{ return (A.x + B.x) / 2; }
	AVFLOAT CenterY()	{ return (A.y + B.y) / 2; }
	AVFLOAT CenterZ()	{ return (A.z + B.z) / 2; }

	AVFLOAT LeftExt()	{ return A1.x; }
	AVFLOAT RightExt()	{ return B1.x; }
	AVFLOAT FrontExt()	{ return A1.y; }
	AVFLOAT RearExt()	{ return B1.y; }
	AVFLOAT LowerExt()	{ return A1.z; }
	AVFLOAT UpperExt()	{ return B1.z; }

	AVFLOAT CenterXExt()	{ return (A1.x + B1.x) / 2; }
	AVFLOAT CenterYExt()	{ return (A1.y + B1.y) / 2; }
	AVFLOAT CenterZExt()	{ return (A1.z + B1.z) / 2; }

	AVFLOAT Width()		{ return B.x - A.x; };
	AVFLOAT Depth()		{ return B.y - A.y; };
	AVFLOAT Height()	{ return B.z - A.z; };
	AVFLOAT Aspect()	{ return Width() / Depth(); }

	AVFLOAT WidthExt()	{ return B1.x - A1.x; };
	AVFLOAT DepthExt()	{ return B1.y - A1.y; };
	AVFLOAT HeightExt()	{ return B1.z - A1.z; };

	AVFLOAT WidthLWall()	{ return B.x - A1.x; };
	AVFLOAT DepthFWall()	{ return B.y - A1.y; };
	AVFLOAT HeightLSlab()	{ return B.z - A1.z; };
	AVFLOAT WidthRWall()	{ return B1.x - A.x; };
	AVFLOAT DepthRWall()	{ return B1.y - A.y; };
	AVFLOAT HeightUSlab()	{ return B1.z - A.z; };

	AVFLOAT FrontThickness()	{ return A.y - A1.y; }
	AVFLOAT RearThickness()		{ return B1.y - B.y; }
	AVFLOAT LeftThickness()		{ return A.x - A1.x; }
	AVFLOAT RightThickness()	{ return B1.x - B.x; }
	AVFLOAT LowerThickness()	{ return A.z - A1.z; }
	AVFLOAT UpperThickness()	{ return B1.z - B.z; }

	void SetThickness(AVFLOAT t)										{ A1 = A; A1.x -= t; A1.y -= t; A1.z -= t; B1 = B; B1.x += t; B1.y += t; B1.z += t; }
	void SetThickness(AVFLOAT ltrt, AVFLOAT ftre)						{ SetThickness(ltrt, ltrt, ftre, ftre); }
	void SetThickness(AVFLOAT lt, AVFLOAT rt, AVFLOAT ft, AVFLOAT re)	{ A1 = A; A1.x -= lt; A1.y -= ft; B1 = B; B1.x += rt; B1.y += re; }
	void SetThickness(AVFLOAT lt, AVFLOAT rt, AVFLOAT ft, AVFLOAT re, AVFLOAT lw, AVFLOAT up)	
																		{ A1 = A; A1.x -= lt; A1.y -= ft; A1.z -= lw; B1 = B; B1.x += rt; B1.y += re; B1.z += up; }
	void SetFrontThickness(AVFLOAT f)	{ A1.y = A.y - f; }
	void SetRearThickness(AVFLOAT f)	{ B1.y = B.y + f; }
	void SetLeftThickness(AVFLOAT f)	{ A1.x = A.x - f; }
	void SetRightThickness(AVFLOAT f)	{ B1.x = B.x + f; }
	void SetLowerThickness(AVFLOAT f)	{ A1.z = A.z - f; }
	void SetUpperThickness(AVFLOAT f)	{ B1.z = B.z + f; }

	void SetWidth(AVFLOAT w)			{ AVFLOAT dw = w - B.x; B.x += dw; B1.x += dw; }
	void SetDepth(AVFLOAT d)			{ AVFLOAT dd = d - B.y; B.y += dd; B1.y += dd; }
	void SetHeight(AVFLOAT h)			{ AVFLOAT dh = h - B.z; B.z += dh; B1.z += dh; }

	void Move(AVVECTOR v)				{ A.x += v.x; A.y += v.y; A.z += v.z; B.x += v.x; B.y += v.y; B.z += v.z; A1.x += v.x; A1.y += v.y; A1.z += v.z; B1.x += v.x; B1.y += v.y; B1.z += v.z; }
	void Move(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ A.x += x; A.y += y; A.z += z; B.x += x; B.y += y; B.z += z; A1.x += x; A1.y += y; A1.z += z; B1.x += x; B1.y += y; B1.z += z; }
	void MoveX(AVFLOAT f)				{ A.x += f; B.x += f; A1.x += f; B1.x += f; }
	void MoveY(AVFLOAT f)				{ A.y += f; B.y += f; A1.y += f; B1.y += f; }
	void MoveZ(AVFLOAT f)				{ A.z += f; B.z += f; A1.z += f; B1.z += f; }
	void Scale(AVFLOAT f)				{ A.x *= f; A.y *= f; A.z *= f; B.x *= f; B.y *= f; B.z *= f; A1.x *= f; A1.y *= f; A1.z *= f; B1.x *= f; B1.y *= f; B1.z *= f; }
	void Scale(AVFLOAT x, AVFLOAT y, AVFLOAT z)	{ A.x *= x; A.y *= y; A.z *= z; B.x *= x; B.y *= y; B.z *= z; A1.x *= x; A1.y *= y; A1.z *= z; B1.x *= x; B1.y *= y; B1.z *= z; }
	void ScaleX(AVFLOAT f)				{ A.x *= f; B.x *= f; A1.x *= f; B1.x *= f; }
	void ScaleY(AVFLOAT f)				{ A.y *= f; B.y *= f; A1.y *= f; B1.y *= f; }
	void ScaleZ(AVFLOAT f)				{ A.z *= f; B.z *= f; A1.z *= f; B1.z *= f; }

	AVVECTOR LeftFrontLower()			{ return A; }
	AVVECTOR LeftFrontLowerExt()		{ return Vector(A.x, A.y, A1.z); }
	AVVECTOR LeftFrontExtLower()		{ return Vector(A.x, A1.y, A.z); }
	AVVECTOR LeftFrontExtLowerExt()		{ return Vector(A.x, A1.y, A1.z); }
	AVVECTOR LeftExtFrontLower()		{ return Vector(A1.x, A.y, A.z); }
	AVVECTOR LeftExtFrontLowerExt()		{ return Vector(A1.x, A.y, A1.z); }
	AVVECTOR LeftExtFrontExtLower()		{ return Vector(A1.x, A1.y, A.z); }
	AVVECTOR LeftExtFrontExtLowerExt()	{ return A1; }
	
	AVVECTOR LeftFrontUpper()			{ return Vector(A.x, A.y, B.z); }
	AVVECTOR LeftFrontUpperExt()		{ return Vector(A.x, A.y, B1.z); }
	AVVECTOR LeftFrontExtUpper()		{ return Vector(A.x, A1.y, B.z); }
	AVVECTOR LeftFrontExtUpperExt()		{ return Vector(A.x, A1.y, B1.z); }
	AVVECTOR LeftExtFrontUpper()		{ return Vector(A1.x, A.y, B.z); }
	AVVECTOR LeftExtFrontUpperExt()		{ return Vector(A1.x, A.y, B1.z); }
	AVVECTOR LeftExtFrontExtUpper()		{ return Vector(A1.x, A1.y, B.z); }
	AVVECTOR LeftExtFrontExtUpperExt()	{ return Vector(A1.x, A1.y, B1.z); }
	
	AVVECTOR LeftRearLower()			{ return Vector(A.x, B.y, A.z); }
	AVVECTOR LeftRearLowerExt()			{ return Vector(A.x, B.y, A1.z); }
	AVVECTOR LeftRearExtLower()			{ return Vector(A.x, B1.y, A.z); }
	AVVECTOR LeftRearExtLowerExt()		{ return Vector(A.x, B1.y, A1.z); }
	AVVECTOR LeftExtRearLower()			{ return Vector(A1.x, B.y, A.z); }
	AVVECTOR LeftExtRearLowerExt()		{ return Vector(A1.x, B.y, A1.z); }
	AVVECTOR LeftExtRearExtLower()		{ return Vector(A1.x, B1.y, A.z); }
	AVVECTOR LeftExtRearExtLowerExt()	{ return Vector(A1.x, B1.y, A1.z); }
	
	AVVECTOR LeftRearUpper()			{ return Vector(A.x, B.y, B.z); }
	AVVECTOR LeftRearUpperExt()			{ return Vector(A.x, B.y, B1.z); }
	AVVECTOR LeftRearExtUpper()			{ return Vector(A.x, B1.y, B.z); }
	AVVECTOR LeftRearExtUpperExt()		{ return Vector(A.x, B1.y, B1.z); }
	AVVECTOR LeftExtRearUpper()			{ return Vector(A1.x, B.y, B.z); }
	AVVECTOR LeftExtRearUpperExt()		{ return Vector(A1.x, B.y, B1.z); }
	AVVECTOR LeftExtRearExtUpper()		{ return Vector(A1.x, B1.y, B.z); }
	AVVECTOR LeftExtRearExtUpperExt()	{ return Vector(A1.x, B1.y, B1.z); }
	
	AVVECTOR RightFrontLower()			{ return Vector(B.x, A.y, A.z); }
	AVVECTOR RightFrontLowerExt()		{ return Vector(B.x, A.y, A1.z); }
	AVVECTOR RightFrontExtLower()		{ return Vector(B.x, A1.y, A.z); }
	AVVECTOR RightFrontExtLowerExt()	{ return Vector(B.x, A1.y, A1.z); }
	AVVECTOR RightExtFrontLower()		{ return Vector(B1.x, A.y, A.z); }
	AVVECTOR RightExtFrontLowerExt()	{ return Vector(B1.x, A.y, A1.z); }
	AVVECTOR RightExtFrontExtLower()	{ return Vector(B1.x, A1.y, A.z); }
	AVVECTOR RightExtFrontExtLowerExt()	{ return Vector(B1.x, A1.y, A1.z); }
	
	AVVECTOR RightFrontUpper()			{ return Vector(B.x, A.y, B.z); }
	AVVECTOR RightFrontUpperExt()		{ return Vector(B.x, A.y, B1.z); }
	AVVECTOR RightFrontExtUpper()		{ return Vector(B.x, A1.y, B.z); }
	AVVECTOR RightFrontExtUpperExt()	{ return Vector(B.x, A1.y, B1.z); }
	AVVECTOR RightExtFrontUpper()		{ return Vector(B1.x, A.y, B.z); }
	AVVECTOR RightExtFrontUpperExt()	{ return Vector(B1.x, A.y, B1.z); }
	AVVECTOR RightExtFrontExtUpper()	{ return Vector(B1.x, A1.y, B.z); }
	AVVECTOR RightExtFrontExtUpperExt()	{ return Vector(B1.x, A1.y, B1.z); }
	
	AVVECTOR RightRearLower()			{ return Vector(B.x, B.y, A.z); }
	AVVECTOR RightRearLowerExt()		{ return Vector(B.x, B.y, A1.z); }
	AVVECTOR RightRearExtLower()		{ return Vector(B.x, B1.y, A.z); }
	AVVECTOR RightRearExtLowerExt()		{ return Vector(B.x, B1.y, A1.z); }
	AVVECTOR RightExtRearLower()		{ return Vector(B1.x, B.y, A.z); }
	AVVECTOR RightExtRearLowerExt()		{ return Vector(B1.x, B.y, A1.z); }
	AVVECTOR RightExtRearExtLower()		{ return Vector(B1.x, B1.y, A.z); }
	AVVECTOR RightExtRearExtLowerExt()	{ return Vector(B1.x, B1.y, A1.z); }
	
	AVVECTOR RightRearUpper()			{ return B; }
	AVVECTOR RightRearUpperExt()		{ return Vector(B.x, B.y, B1.z); }
	AVVECTOR RightRearExtUpper()		{ return Vector(B.x, B1.y, B.z); }
	AVVECTOR RightRearExtUpperExt()		{ return Vector(B.x, B1.y, B1.z); }
	AVVECTOR RightExtRearUpper()		{ return Vector(B1.x, B.y, B.z); }
	AVVECTOR RightExtRearUpperExt()		{ return Vector(B1.x, B.y, B1.z); }
	AVVECTOR RightExtRearExtUpper()		{ return Vector(B1.x, B1.y, B.z); }
	AVVECTOR RightExtRearExtUpperExt()	{ return B1; }

	AVVECTOR CentreFrontLower()			{ return Vector((A.x + B.x) / 2, A.y, A.z); }
	AVVECTOR CentreFrontLowerExt()		{ return Vector((A.x + B.x) / 2, A.y, A1.z); }
	AVVECTOR CentreFrontExtLower()		{ return Vector((A.x + B.x) / 2, A1.y, A.z); }
	AVVECTOR CentreFrontExtLowerExt()	{ return Vector((A.x + B.x) / 2, A1.y, A1.z); }
	
	AVVECTOR CentreFrontUpper()			{ return Vector((A.x + B.x) / 2, A.y, B.z); }
	AVVECTOR CentreFrontUpperExt()		{ return Vector((A.x + B.x) / 2, A.y, B1.z); }
	AVVECTOR CentreFrontExtUpper()		{ return Vector((A.x + B.x) / 2, A1.y, B.z); }
	AVVECTOR CentreFrontExtUpperExt()	{ return Vector((A.x + B.x) / 2, A1.y, B1.z); }
	
	AVVECTOR CentreRearLower()			{ return Vector((A.x + B.x) / 2, B.y, A.z); }
	AVVECTOR CentreRearLowerExt()		{ return Vector((A.x + B.x) / 2, B.y, A1.z); }
	AVVECTOR CentreRearExtLower()		{ return Vector((A.x + B.x) / 2, B1.y, A.z); }
	AVVECTOR CentreRearExtLowerExt()	{ return Vector((A.x + B.x) / 2, B1.y, A1.z); }
	
	AVVECTOR CentreRearUpper()			{ return Vector((A.x + B.x) / 2, B.y, B.z); }
	AVVECTOR CentreRearUpperExt()		{ return Vector((A.x + B.x) / 2, B.y, B1.z); }
	AVVECTOR CentreRearExtUpper()		{ return Vector((A.x + B.x) / 2, B1.y, B.z); }
	AVVECTOR CentreRearExtUpperExt()	{ return Vector((A.x + B.x) / 2, B1.y, B1.z); }

	std::wstring Stringify()
	{
		std::wstringstream s;
		s << A .x << L" "<< A .y << L" "<< A .z << L" "<< B .x << L" "<< B .y << L" "<< B .z << L" "
		  << A1.x << L" "<< A1.y << L" "<< A1.z << L" "<< B1.x << L" "<< B1.y << L" "<< B1.z << L" ";
		std::wstring dupa = s.str();
		return s.str();
	}

	void ParseFromString(std::wstring str)
	{
		std::wstringstream s(str);
		s >> A .x >> A .y >> A .z >> B .x >> B .y >> B .z 
		  >> A1.x >> A1.y >> A1.z >> B1.x >> B1.y >> B1.z;
	}
};
