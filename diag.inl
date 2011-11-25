class CDiag
{
public:
	enum MODE	{ COORD = 1, DELTA = 2, TILT = 4, HEAD = 8, CMP = 16, ALL = 0xFFFF };

private:
	FWSTRING m_pName;
	IBody *m_pBody;
	FWULONG m_nId;
	FWULONG m_modeDef;

	FWVECTOR m_vecPrev;
	bool m_bIsPrev;

	// for CMP mode only
	FWVECTOR m_vecCmp1;
	FWQUAT m_quatCmp1;
	IKineChild *m_pNodeCmp;

public:
	CDiag() : m_bIsPrev(false)
	{
		Init(L"???", NULL, 0, ALL);
	}

	CDiag(FWSTRING pName, IBody *pBody, FWULONG nId, FWULONG mode = ALL) : m_bIsPrev(false)
	{
		Init(pName, pBody, nId, mode);
	}

	CDiag(FWSTRING pName, IBody *pBody, FWULONG nId, IKineChild *pNodeCmp, FWULONG mode = ALL) : m_bIsPrev(false)
	{
		Init(pName, pBody, nId, pNodeCmp, mode);
	}

public:

	void Init(FWSTRING pName, IBody *pBody, FWULONG nId, FWULONG mode = ALL)
	{
		m_pName = pName;
		m_pBody = pBody;
		m_nId = nId;
		m_modeDef = mode;
		m_pNodeCmp = NULL;
	}

	void Init(FWSTRING pName, IBody *pBody, FWULONG nId, IKineChild *pNodeCmp, FWULONG mode = ALL)
	{
		Init(pName, pBody, nId, mode);
		ITransform *pT = NULL;
		pNodeCmp->CreateCompatibleTransform(&pT);
		pNodeCmp->GetTransform(pT, KINE_GLOBAL);
		pT->AsVector(&m_vecCmp1); pT->AsQuat(&m_quatCmp1);
		m_pNodeCmp = pNodeCmp;
		pT->Release();
	}

	CString GetDiag()	{ return GetDiag(m_modeDef); }

	CString GetDiag(FWULONG mode)
	{
		if (!m_pBody) return L"???";
		CString s1, s2, s3, s4, s5;

		IKineChild *pNode = m_pBody->BodyChild(m_nId);
		if (!pNode) return L"???";
		ITransform *pT = NULL;
		pNode->CreateCompatibleTransform(&pT);
		pNode->Invalidate(); pNode->GetGlobalTransform(pT);
		pNode->Release(); pNode = NULL;

		FWVECTOR v0 = { 0, 0, 0 }; 
		FWMATRIX A;
		pT->ApplyTo(&v0);
		pT->AsMatrix(A);

		if (mode & COORD)
			s1.Format(L"%ls \t\t\t pos:   (%lf, %lf, %lf)\n", m_pName, v0.x, v0.y, v0.z);
		if ((mode & DELTA) && m_bIsPrev) 
			s2.Format(L"%ls\tlen: %lf;\tdelta: (%lf, %lf, %lf)\n", 
				m_pName, 
				sqrt((v0.x - m_vecPrev.x)*(v0.x - m_vecPrev.x)+(v0.y - m_vecPrev.y)*(v0.y - m_vecPrev.y)), //RAD2DEG(atan2((v0.x - m_vecPrev.x), -(v0.y - m_vecPrev.y))),
				v0.x - m_vecPrev.x, v0.y - m_vecPrev.y, v0.z - m_vecPrev.z);
		if (mode & TILT)
		{
			double d = A[0][2];
			if (d < -1) d = -1; else if (d > 1) d = 1;
			s3.Format(L"Vertical Tilt:\t%lf\n", RAD2DEG(acos(d)));
		}
		if (mode & HEAD)
			s4.Format(L"Turning angle:\t%lf\n", RAD2DEG(atan2(A[1][0], -A[1][1])));

		if ((mode & CMP) && m_pNodeCmp)
		{
			FWVECTOR vecCmp2; FWQUAT quatCmp2;
			m_pNodeCmp->GetTransform(pT, KINE_GLOBAL);
			pT->AsVector(&vecCmp2); pT->AsQuat(&quatCmp2);

			double distA = RAD2DEG(acos(m_quatCmp1.w*quatCmp2.w + m_quatCmp1.x*quatCmp2.x + m_quatCmp1.y*quatCmp2.y + m_quatCmp1.z*quatCmp2.z));
			double distV = sqrt((m_vecCmp1.x-vecCmp2.x)*(m_vecCmp1.x-vecCmp2.x) + (m_vecCmp1.y-vecCmp2.y)*(m_vecCmp1.y-vecCmp2.y) + (m_vecCmp1.z-vecCmp2.z)*(m_vecCmp1.z-vecCmp2.z));
			s5.Format(L"Angular Dist: %lf\nSpatial Dist: %lf\n", distA, distV);
		}

		memcpy(&m_vecPrev, &v0, sizeof(FWVECTOR));
		m_bIsPrev = true;
		pT->Release();

		return (s1 + s2 + s3 + s4 + s5);
	}

	operator CString() { return GetDiag(); }
	operator LPCTSTR() { return (LPCTSTR)GetDiag(); }
};
