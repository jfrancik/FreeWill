// LeftView.cpp : implementation of the CLeftView class
//

#include "stdafx.h"
#include "fwview.h"

#include "FreeWillDoc.h"
#include "LeftView.h"
#include ".\leftview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnTvnSelchanged)
	ON_COMMAND(ID_LEFT_EXPAND_ALL, OnLeftExpandAll)
	ON_COMMAND(ID_LEFT_EXPAND_BIPED_HANDS, OnLeftExpandBipedHands)
	ON_COMMAND(ID_LEFT_EXPAND_BIPED_FINGERS, OnLeftExpandBipedFingers)
	ON_COMMAND_RANGE(ID_LEFT_EXPAND2, ID_LEFT_EXPAND16, OnLeftExpandNum)
	ON_NOTIFY_REFLECT(TVN_GETINFOTIP, OnTvnGetInfoTip)
END_MESSAGE_MAP()


// CLeftView construction/destruction

CLeftView::CLeftView()
{
}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying the CREATESTRUCT cs

	cs.style = cs.style | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_SHOWSELALWAYS | TVS_INFOTIP;

	return CTreeView::PreCreateWindow(cs);
}

void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	IScene *pScene;
	GetDocument()->GetFreeWillObjects(NULL, NULL, &pScene, NULL);
	if (!pScene) return;

	GetTreeCtrl().DeleteAllItems();
	GetDocument()->SetCurrentNode(NULL);
	HTREEITEM h = InsertItem(pScene);
	GetTreeCtrl().SelectSetFirstVisible(h);
	pScene->Release();

	return;
}

void CLeftView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	switch (lHint)
	{
	case WM_PAINT:
		break;
	case RELOAD_SCENE_STRUCT:
		{
			IScene *pScene;
			GetDocument()->GetFreeWillObjects(NULL, NULL, &pScene, NULL);
			if (!pScene) return;

			GetTreeCtrl().DeleteAllItems();
			GetDocument()->SetCurrentNode(NULL);
			HTREEITEM h = InsertItem(pScene);
			GetTreeCtrl().SelectSetFirstVisible(h);
			pScene->Release();
		}
		break;
	}
}

HTREEITEM CLeftView::InsertItem(IKineChild *pItemNode, HTREEITEM hParent, HTREEITEM hInsertAfter)
{
	if (!pItemNode)
		return NULL;

	// get label
	FWSTRING pLabel;
	FWSTRING pClass;
	CString strLabel;

	pItemNode->GetLabel(&pLabel);
	pItemNode->GetClassId(&pClass);
	strLabel.Format(L"%ls (%ls)", pLabel, pClass);

	// set tree item
	TVINSERTSTRUCT tvInsert;
	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = hInsertAfter;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;
	tvInsert.item.pszText = (LPTSTR)(LPCTSTR)strLabel;
	tvInsert.item.lParam = (LPARAM)pItemNode;
	HTREEITEM hTreeItem = GetTreeCtrl().InsertItem(&tvInsert);
	if (strLabel == "Pelvis (KineBone)")
	{
		GetTreeCtrl().Select(hTreeItem, TVGN_CARET);
		GetDocument()->SetCurrentNode((IKineNode*)pItemNode);
		GetDocument()->SetManipArbitraryAxis(pItemNode);
	}

	// enumerate children
	IKineNode *pNode = NULL;
	HRESULT h = pItemNode->QueryInterface(&pNode);
	if (SUCCEEDED(h))
	{
		IKineChild *p;
		IKineEnumChildren *pEnum = NULL;
		pNode->EnumChildren(&pEnum);
		while (pEnum->Next(&p) == S_OK)
		{
			if (p->IsParent(pItemNode) == S_OK)
				InsertItem(p, hTreeItem);
			p->Release();
		}
		pEnum->Release();
		pNode->Release();
	}

	GetTreeCtrl().Expand(hTreeItem, TVE_EXPAND);

	return hTreeItem;
}

// CLeftView diagnostics

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CFreeWillDoc *CLeftView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFreeWillDoc)));
	return (CFreeWillDoc*)m_pDocument;
}
#endif //_DEBUG


// CLeftView message handlers

void CLeftView::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	m_pCurrent = (IKineNode*)pNMTreeView->itemNew.lParam;
	GetDocument()->SetCurrentNode(m_pCurrent);
	*pResult = 0;
}

	void ExpandTree(CTreeCtrl &tree, HTREEITEM hItem, int nNumLevels = 9999, int nSpecial = 0)
	{
		if (hItem == NULL) return;

		// termination condition
		bool b = false;
		CString str;
		switch (nSpecial)
		{
		case 1:	// Biped Hands and Feet
			str = tree.GetItemText(hItem);
			b = str.Mid(2, 4) == "Hand" || str.Mid(2, 4) == "Foot";
			break;
		case 2:
			str = tree.GetItemText(hItem);
			b = str.Mid(2, 6) == "Finger" || str.Mid(2, 3) == "Toe";
			break;
		default:
			b = nNumLevels == 1;
		}

		if (b)
			tree.Expand(hItem, TVE_COLLAPSE);
		else
		{
			tree.Expand(hItem, TVE_EXPAND);
			for (HTREEITEM h = tree.GetNextItem(hItem, TVGN_CHILD); h != NULL; h = tree.GetNextItem(h, TVGN_NEXT))
				ExpandTree(tree, h, nNumLevels - 1, nSpecial);
		}
	}

void CLeftView::OnLeftExpandAll()
{
	ExpandTree(GetTreeCtrl(), GetTreeCtrl().GetNextItem(NULL, TVGN_ROOT));
}

void CLeftView::OnLeftExpandBipedHands()
{
	ExpandTree(GetTreeCtrl(), GetTreeCtrl().GetNextItem(NULL, TVGN_ROOT), 9999, 1);
}

void CLeftView::OnLeftExpandBipedFingers()
{
	ExpandTree(GetTreeCtrl(), GetTreeCtrl().GetNextItem(NULL, TVGN_ROOT), 9999, 2);
}

void CLeftView::OnLeftExpandNum(UINT n)
{
	n = n - ID_LEFT_EXPAND2 + 2;
	ExpandTree(GetTreeCtrl(), GetTreeCtrl().GetNextItem(NULL, TVGN_ROOT), n);
}


void CLeftView::OnTvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);

	IKineNode *pKineNode = (IKineNode*)(pGetInfoTip->lParam);
	if (!pKineNode) return;

	FWSTRING pLabel;
	FWVECTOR v = { 0, 0, 0 };
	FWMATRIX MG, MB, ML;
	static CString str;

	pKineNode->GetLabel(&pLabel);

	pKineNode->LtoG(&v);
	pKineNode->GetGlobalMatrix(MG);
	ITransform *pTB = NULL;
	pKineNode->GetBaseTransformRef(&pTB);
	pTB->AsMatrix(MB);
	pTB->Release();
	ITransform *pTL = NULL;
	pKineNode->GetLocalTransformRef(&pTL);
	pTL->AsMatrix(ML);
	pTL->Release();
	str.Format(
		L"%ls @ (%.2f, %.2f, %.2f):\r\n"
		L"G = [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"B = [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"L = [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n"
		L"       [ %14.10lf   %14.10lf   %14.10lf   %14.10lf ]\r\n",
			pLabel, v.x, v.y, v.z,
			MG[0][0], MG[0][1], MG[0][2], MG[0][3],
			MG[1][0], MG[1][1], MG[1][2], MG[1][3],
			MG[2][0], MG[2][1], MG[2][2], MG[2][3],
			MG[3][0], MG[3][1], MG[3][2], MG[3][3],
			MB[0][0], MB[0][1], MB[0][2], MB[0][3],
			MB[1][0], MB[1][1], MB[1][2], MB[1][3],
			MB[2][0], MB[2][1], MB[2][2], MB[2][3],
			MB[3][0], MB[3][1], MB[3][2], MB[3][3],
			ML[0][0], ML[0][1], ML[0][2], ML[0][3],
			ML[1][0], ML[1][1], ML[1][2], ML[1][3],
			ML[2][0], ML[2][1], ML[2][2], ML[2][3],
			ML[3][0], ML[3][1], ML[3][2], ML[3][3]);

	pGetInfoTip->pszText = (LPTSTR)(LPCTSTR)str;

	*pResult = 0;
}
