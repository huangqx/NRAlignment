/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2005 by Joerg Koenig
// All rights reserved
//
// Distribute freely, except: don't remove my name from the source or
// documentation (don't take credit for my work), mark your changes (don't
// get me blamed for your possible bugs), don't alter or remove this
// notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    J.Koenig@adg.de                 (company site)
//    Joerg.Koenig@rhein-neckar.de    (private site)
/////////////////////////////////////////////////////////////////////////////


// EditTreeCtrlEx.cpp : implementation file
//

#include <string>
using namespace std;

#include "NRAlignmentView.h"
#include "NRAlignmentDoc.h"
#include "EditTreeCtrlEx.h"
#include "MainFrm.h"

#include "data_container.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(4:4503)		// decorated name length exceeded

enum EExCmds {
	ID_SELECT_ALL = 34555,
	ID_SELECT_NONE,

	ID_EX_MAX
};

//---------------------------------------------------------------------------
// helper class for tracking

void CEditTreeTracker::SetupTracker(CEditTreeCtrlEx * pTree, CPoint & pt) {
	m_pEditTree = pTree;
	ASSERT(m_pEditTree != 0);
	// Before we start tracking, we store all selected
	// items. If the user interupts the tracking, we have to
	// restore them...
	list<HTREEITEM> listSel;
	for(HTREEITEM hItem = pTree->GetFirstSelectedItem(); hItem != 0; hItem = pTree->GetNextSelectedItem(hItem)) {
		listSel.push_back(hItem);
		pTree->SetItemState(hItem, UINT(~TVIS_SELECTED), UINT(TVIS_SELECTED));
	}

	if(!TrackRubberBand(pTree, pt)) {
		// Deselect all items selected by the tracker
		pTree->ClearSelection();

		// Restore previously selected items
		for(list<HTREEITEM>::iterator it = listSel.begin(); it != listSel.end(); ++it)
			pTree->SetItemState(*it, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));

		// if the user canceled the tracking, it might happen, that the
		// rubber band leaves artefacts on the display :-\ 
		pTree->Invalidate();
	}
	m_pEditTree = 0;
}


void CEditTreeTracker::OnChangedRect(const CRect & rcOld) {
	ASSERT(m_pEditTree != 0);

	CRect rc(rcOld);
	rc.NormalizeRect();

	CRect rcClient;
	m_pEditTree->GetClientRect(rcClient);

	m_pEditTree->SetRedraw(false);

	bool bFirstItem = true;

	// Walk the visible items
	for(HTREEITEM hItem = m_pEditTree->GetFirstVisibleItem(); hItem != 0; hItem = m_pEditTree->GetNextVisibleItem(hItem)) {
		CRect rcItem;
		m_pEditTree->GetItemRect(hItem, rcItem, true);

		if(rcClient.PtInRect(rcItem.TopLeft()) || rcClient.PtInRect(rcItem.BottomRight())) {
			// The item is actually (partially) visible in the client area
			if(rc.PtInRect(rcItem.TopLeft()) || rc.PtInRect(rcItem.BottomRight())) {
				// We have to move the focus to the first item of the
				// selction. If we don't do it, the default implementation
				// of the CTreeCtrl class will automatically select the
				// focused item as soon as we start dragging the selection...
				if(bFirstItem) {
					m_pEditTree->SelectItem(hItem);
					bFirstItem = false;
				}
				// the item is at least partially inside the tracker's rectangle
				m_pEditTree->SetItemState(hItem, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));
			} else {
				// The item lies outside the tracker's rectangle
				m_pEditTree->SetItemState(hItem, UINT(~TVIS_SELECTED), UINT(TVIS_SELECTED));
			}
			m_pEditTree->InvalidateRect(rcItem);
		}
	}

	rc.DeflateRect(1,1);
	m_pEditTree->InvalidateRect(rc);
	m_pEditTree->SetRedraw();
}

//---------------------------------------------------------------------------
// CEditTreeCtrlEx

CEditTreeCtrlEx::CEditTreeCtrlEx()
	: m_bOldItemSelected(false)
	, m_bMultiSel(true)
{
	// Extend the keymapper with our own methods:

	// Ctrl+A selects all items
	m_Keymap['A'][true][false] = method(&CEditTreeCtrlEx::DoSelectAll);
}


CEditTreeCtrlEx::~CEditTreeCtrlEx()
{
}


int CEditTreeCtrlEx::GetSelectedCount() const {
	int nResult = 0 ;
	for(HTREEITEM hItem = GetRootItem(); hItem != 0; hItem = GetNextVisibleItem(hItem))
		if(GetItemState(hItem, UINT(TVIS_SELECTED)) & TVIS_SELECTED)
			++nResult;
	return nResult;
}


HTREEITEM CEditTreeCtrlEx::GetFirstSelectedItem() const {
	for(HTREEITEM hItem = GetRootItem(); hItem; hItem = GetNextVisibleItem(hItem))
		if(GetItemState(hItem, UINT(TVIS_SELECTED)) & TVIS_SELECTED)
			return hItem;
	return 0;
}


HTREEITEM CEditTreeCtrlEx::GetNextSelectedItem(HTREEITEM hItem) const {
	if(hItem)
		for(hItem = GetNextVisibleItem(hItem); hItem; hItem = GetNextVisibleItem(hItem))
			if(GetItemState(hItem, UINT(TVIS_SELECTED)) & TVIS_SELECTED)
				return hItem;
	return 0;
}


HTREEITEM CEditTreeCtrlEx::GetPrevSelectedItem(HTREEITEM hItem) const {
	if(hItem)
		for(hItem = GetPrevVisibleItem(hItem); hItem; hItem = GetPrevVisibleItem(hItem))
			if(GetItemState(hItem, UINT(TVIS_SELECTED)) & TVIS_SELECTED)
				return hItem;
	return 0;
}


void CEditTreeCtrlEx::ClearSelection(HTREEITEM hExcept, HTREEITEM hItem) {
	if(!hItem)
		hItem = GetRootItem();
	while(hItem) {
		if(hItem != hExcept)
			SetItemState(hItem, UINT(~TVIS_SELECTED), UINT(TVIS_SELECTED));

		if(ItemHasChildren(hItem))
			ClearSelection(hExcept, GetChildItem(hItem));
		hItem = GetNextSiblingItem(hItem);
	}
}


void CEditTreeCtrlEx::SelectAll() {
	for(HTREEITEM hItem = GetRootItem(); hItem != 0; hItem = GetNextVisibleItem(hItem))
		SetItemState(hItem, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));
}


void CEditTreeCtrlEx::DeselectItem(HTREEITEM hItem) {
	if(hItem)
		SetItemState(hItem, UINT(~TVIS_SELECTED), UINT(TVIS_SELECTED));
}


void CEditTreeCtrlEx::InvalidateSelectedItems() {
	for(HTREEITEM hItem = GetFirstSelectedItem(); hItem != 0; hItem = GetNextSelectedItem(hItem)) {
		CRect rc;
		if(GetItemRect(hItem, rc, true))
			InvalidateRect(rc);
	}
}


void CEditTreeCtrlEx::SelectItems(HTREEITEM hFrom, HTREEITEM hTo) {
	HTREEITEM hItem = GetRootItem();
	bool bSelect = false;
	while(hItem) {
		if(hItem == hFrom || hItem == hTo)
			bSelect = !bSelect;
		if(bSelect || hItem == hFrom || hItem == hTo)
			SetItemState(hItem, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));

		hItem = GetNextVisibleItem(hItem);
	}
}


void CEditTreeCtrlEx::GetSelectedItemsWithoutDescendents(list<HTREEITEM> & listSel) {
	for(HTREEITEM hItem = GetFirstSelectedItem(); hItem != 0; hItem = GetNextSelectedItem(hItem)) {
		bool bChild = false;
		for(list<HTREEITEM>::iterator it = listSel.begin(); it != listSel.end(); ++it)
			if(IsAncestor(*it, hItem)) {
				bChild = true;
				break;
			}
		if(!bChild)
			listSel.push_back(hItem);
	}
}


void CEditTreeCtrlEx::DeselectTree(HTREEITEM hItem) {
	while(hItem) {
		SetItemState(hItem, UINT(~TVIS_SELECTED), UINT(TVIS_SELECTED));
		if(ItemHasChildren(hItem))
			DeselectTree(GetChildItem(hItem));
		hItem = GetNextSiblingItem(hItem);
	}
}


bool CEditTreeCtrlEx::DoSelectAll(HTREEITEM) {
	SelectAll();
	return true;
}


CImageList * CEditTreeCtrlEx::CreateDragImageEx() {
	CImageList	*pResultImageList = 0;

	int nNumSelected = GetSelectedCount();

	if(nNumSelected >= 1) {
		HTREEITEM hItem = 0;
		CString strItemText;
		CRect rcItem;
		int nMaxWidth = 0;

		CDC * pDragImageCalcDC = GetDC();
		if(pDragImageCalcDC == NULL)
			return 0;

		CImageList * pImageList = GetImageList(TVSIL_NORMAL);
		if(!pImageList)
			// even a normal CTreeCtrl can't create a drag image without an imagelist set... :-\ 
			return 0;
		int cx,cy;
		ImageList_GetIconSize(*pImageList, &cx, &cy);

		// Calculate the maximum width of the bounding rectangle
		for(hItem = GetFirstSelectedItem(); hItem; hItem = GetNextSelectedItem(hItem)) {
			// Get the item's height and width one by one
			strItemText = GetItemText(hItem);
			rcItem.SetRectEmpty();
			pDragImageCalcDC->DrawText(strItemText, rcItem, DT_CALCRECT);
			if(nMaxWidth < ( rcItem.Width()+cx))
				nMaxWidth = rcItem.Width()+cx;
		}

		// Get the first item's height and width
		hItem = GetFirstSelectedItem();
		strItemText = GetItemText(hItem);
		rcItem.SetRectEmpty();
		pDragImageCalcDC->DrawText(strItemText, rcItem, DT_CALCRECT);
		ReleaseDC(pDragImageCalcDC);

		// Initialize textRect for the first item
		CRect rcText;  // Holds text area of image
		rcText.SetRect(1, 1, nMaxWidth, rcItem.Height());

		// Find the bounding rectangle of the bitmap
		CRect rcBounding; // Holds rectangle bounding area for bitmap
		rcBounding.SetRect(0,0, nMaxWidth+2, (rcItem.Height()+2)*nNumSelected);

		// Create bitmap		
		CDC MemoryDC; // Memory Device Context used to draw the drag image
		CClientDC DraggedNodeDC(this); // To draw drag image
		if(!MemoryDC.CreateCompatibleDC(&DraggedNodeDC))
			return 0;
		CBitmap DraggedNodeBmp; // Instance used for holding  dragged bitmap
		if(!DraggedNodeBmp.CreateCompatibleBitmap(&DraggedNodeDC, rcBounding.Width(), rcBounding.Height()))
			return 0;

		CBitmap * pBitmapOldMemDCBitmap = MemoryDC.SelectObject(&DraggedNodeBmp);
		CFont * pFontOld = MemoryDC.SelectObject(GetFont());

		CBrush brush(RGB(255,255,255));
		MemoryDC.FillRect(&rcBounding,&brush);
		MemoryDC.SetBkColor(RGB(255,255,255));
		MemoryDC.SetBkMode(TRANSPARENT);
		MemoryDC.SetTextColor(RGB(0,0,0));

		// Search through array list
		for(hItem = GetFirstSelectedItem(); hItem; hItem = GetNextSelectedItem(hItem)) {
			int nImg, nSelImg;
			GetItemImage(hItem,nImg,nSelImg);
			HICON hIcon = pImageList->ExtractIcon(nImg);
			int nLeft = rcText.left;
			rcText.left += 3;
			::DrawIconEx(MemoryDC.m_hDC, rcText.left, rcText.top, hIcon, 16, 16, 0, 0, DI_NORMAL);
			rcText.left += cx;
			MemoryDC.DrawText(GetItemText(hItem), rcText, DT_LEFT| DT_SINGLELINE|DT_NOPREFIX);
			rcText.left = nLeft;
			rcText.OffsetRect(0, rcItem.Height()+2);
			DestroyIcon(hIcon);
		}
		MemoryDC.SelectObject(pFontOld);
		MemoryDC.SelectObject(pBitmapOldMemDCBitmap);
		MemoryDC.DeleteDC();

		// Create imagelist
		pResultImageList = new CImageList;
		pResultImageList->Create(rcBounding.Width(), rcBounding.Height(), ILC_COLOR | ILC_MASK, 0, 1);
		pResultImageList->Add(&DraggedNodeBmp, RGB(255, 255,255)); 
	} else if(nNumSelected == 1)
		pResultImageList = CreateDragImage(GetFirstSelectedItem());

	return pResultImageList;
}

//---------------------------------------------------------------------------
// Dragging overrides

bool CEditTreeCtrlEx::CanDropItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint) {
	// We have to take care about more than one dragged item
	for(HTREEITEM hItem = GetFirstSelectedItem(); hItem != 0; hItem = GetNextSelectedItem(hItem))
		if(IsAncestor(hItem, hDrop))
			return false;
	return true;
}


void CEditTreeCtrlEx::DragMoveItem(HTREEITEM, HTREEITEM hDrop, EDropHint hint, bool bCopy) {
	list<HTREEITEM> listSel;
	GetSelectedItemsWithoutDescendents(listSel);

	SetRedraw(false);

	for(list<HTREEITEM>::iterator it = listSel.begin(); it != listSel.end(); ++it)
		CEditTreeCtrl::DragMoveItem(*it, hDrop, hint, bCopy);

	SetRedraw();
}


void CEditTreeCtrlEx::DragStart() {
	CEditTreeCtrl::DragStart();
	InvalidateSelectedItems();
	UpdateWindow();
}


void CEditTreeCtrlEx::DragStop() {
	CEditTreeCtrl::DragStop();
	InvalidateSelectedItems();
	UpdateWindow();
}

CDragData * CEditTreeCtrlEx::CreateDragData(HTREEITEM hDragItem, bool bRightDrag) {
	return new CDragDataEx(*this, hDragItem, bRightDrag);
}


//---------------------------------------------------------------------------
// other overrides

bool CEditTreeCtrlEx::CanEditLabel(HTREEITEM hItem) {
	// Don't edit label if multiple items are selected
	return GetSelectedCount() <= 1;
}


bool CEditTreeCtrlEx::CanInsertItem(HTREEITEM hItem) {
	// Don't edit label if multiple items are selected
	return GetSelectedCount() <= 1;
}

void CEditTreeCtrlEx::DeleteTree(HTREEITEM hItem) 
{
	if(ItemHasChildren(hItem))
	{
		HTREEITEM	cIterm = GetChildItem(hItem);
		while (cIterm)
		{
			HTREEITEM	ncIterm = GetNextSiblingItem(cIterm);
			DeleteTree(cIterm);
			cIterm = ncIterm;
		}
	}
	CEditTreeCtrl::DoDeleteItem(hItem);
}

void CEditTreeCtrlEx::DeleteBranch(HTREEITEM hItem) 
{
	if(ItemHasChildren(hItem))
	{
		HTREEITEM	cIterm = GetChildItem(hItem);
		while (cIterm)
		{
			HTREEITEM	ncIterm = GetNextSiblingItem(cIterm);
			DeleteTree(cIterm);
			cIterm = ncIterm;
		}
	}
}

bool CEditTreeCtrlEx::DoDeleteItem(HTREEITEM) {
  list<HTREEITEM> listSel;
  GetSelectedItemsWithoutDescendents(listSel);

  CNRAlignmentDoc* pDoc = 
    ((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();

  for (unsigned shape_id = 0; shape_id < pDoc->data_pt->GetInputShapes()->size(); ++shape_id)
    (*pDoc->data_pt->GetInputShapes())[shape_id].surface.SetRenderBit(false);

  for(list<HTREEITEM>::iterator it = listSel.begin(); it != listSel.end(); ++it)
    pDoc->data_pt->SetRenderingFlags(GetItemText(*it));

  pDoc->data_pt->DeleteSelected();

  for(list<HTREEITEM>::iterator it = listSel.begin(); it != listSel.end(); ++it)
    if(!CEditTreeCtrl::DoDeleteItem(*it))
      return false;



  return true;
}

bool CEditTreeCtrlEx::DoLockItems(HTREEITEM) {
  list<HTREEITEM> listSel;
  GetSelectedItemsWithoutDescendents(listSel);

  for(list<HTREEITEM>::iterator it = listSel.begin(); it != listSel.end(); ++it)
    if(!CEditTreeCtrl::DoLockScan(*it))
      return false;

  return true;
}

bool CEditTreeCtrlEx::DoUnLockItems(HTREEITEM) {
  list<HTREEITEM> listSel;
  GetSelectedItemsWithoutDescendents(listSel);

  for(list<HTREEITEM>::iterator it = listSel.begin(); it != listSel.end(); ++it)
    if(!CEditTreeCtrl::DoUnLockScan(*it))
      return false;

  return true;
}

void CEditTreeCtrlEx::ExtendContextMenu(CMenu & menu) {
	if(IsMultiSelectEnabled()) {
/*		if(menu.GetMenuItemCount())
			VERIFY(menu.AppendMenu(MF_SEPARATOR));
		 VERIFY(menu.AppendMenu(MF_STRING, ID_SELECT_ALL, _T("Select All")));
		 VERIFY(menu.AppendMenu(MF_STRING, ID_SELECT_NONE, _T("Select None")));*/
	}
}


//---------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CEditTreeCtrlEx, CEditTreeCtrl)
	//{{AFX_MSG_MAP(CEditTreeCtrlEx)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelchanged)
	ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchanging)
	ON_NOTIFY_REFLECT(NM_KILLFOCUS, OnKillfocus)
	ON_NOTIFY_REFLECT(NM_SETFOCUS, OnSetfocus)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemexpanded)
	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_SELECT_ALL, ID_EX_MAX-1, OnExCmd)
#if _MFC_VER >= 0x0420
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomTreeDraw)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditTreeCtrlEx message handlers

void CEditTreeCtrlEx::OnExCmd(UINT id) {
	switch(id) {
		case ID_SELECT_ALL:
			DoSelectAll(0);
			break;

		case ID_SELECT_NONE:
			ClearSelection(0);
			break;

		default:
			// New command?
			ASSERT(false);
			break;
	}
}


void CEditTreeCtrlEx::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	* pResult = 0 ;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	HTREEITEM hNew = pNMTreeView->itemNew.hItem;

	if(IsMultiSelectEnabled()) 
	{
		ATLTRACE2(_T("OnSelchanging()\n"));

		HTREEITEM hOld = pNMTreeView->itemOld.hItem;
		UINT & action = pNMTreeView->action;

		bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool bShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
		m_bOldItemSelected = hOld && (GetItemState(hOld, UINT(TVIS_SELECTED)) & TVIS_SELECTED);

		if((action == TVC_BYMOUSE && bCtrl) ) {
			// Ctrl+Mouse - if the item was selected before, deselect it
			if(pNMTreeView->itemNew.state & TVIS_SELECTED) {
				SetItemState(hNew, UINT(~TVIS_SELECTED), UINT(TVIS_SELECTED));
				UpdateWindow() ;
				*pResult = 1 ;	// abort change of selection !
			} else if(!(pNMTreeView->itemOld.state & TVIS_SELECTED))
				// The old item is not selected, so make sure OnSelchanged()
				// will not "re-select" it !
				m_bOldItemSelected = false;
		} else if(action == TVC_BYKEYBOARD && bShift) {
			if(pNMTreeView->itemNew.state & TVIS_SELECTED)
				// misuse of the m_bOldItemSelected data member :-)
				// this marks wether the list of selected items expands or
				// collapses (i.e. the user presses shift on one item and
				// then moves first up a few items and then down or vice
				// versa while holding the shift key)
				m_bOldItemSelected = false;
		} else if(action == TVC_UNKNOWN) {
			// Software generated change of selection.
			// The CTreeCtrl implements a strange behavior when beginning
			// a drag operation by clicking an a different item than the
			// default and not releasing the mouse button before starting to drag:
			// First the 'begin drag' operation occurs. The selected item is
			// the old item, so the "CreateDragImage() method returns the image of the
			// old item. Then occur the 'selection changed' events :-\ 
			// We try to correct this behavior here...
			if(m_pDragData) {
				// Yep - that's it. The 'begin drag' event was already fired while the
				// old item was still selected...
				// If the 'new' item still has no selected state, then we have to
				// deselect all the other selected items.
				if(!(pNMTreeView->itemNew.state & TVIS_SELECTED)) {
					if(!bCtrl) {
						ClearSelection(hNew);
						m_bOldItemSelected = false;
					}
					// The new item still has not the selected state set.
					// Creating a new drag image might fail if we don't
					// set it now.
					SetItemState(hNew, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));
					// The drag image must be changed, too
					m_pDragData->ReleaseDragImage();
					m_pDragData->CreateDragImage();
				}
			}
		}
	} else {
		// single selection only
		ClearSelection(hNew);
		CEditTreeCtrl::OnSelchanging(pNMHDR, pResult);
	}
}


void CEditTreeCtrlEx::OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	if(IsMultiSelectEnabled()) 
	{
		ATLTRACE2(_T("OnSelchanged()\n"));

		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

		HTREEITEM hNew = pNMTreeView->itemNew.hItem;
		HTREEITEM hOld = pNMTreeView->itemOld.hItem;
		UINT & action = pNMTreeView->action;

		bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool bShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;

		// make sure the old selection will not disappear if
		// o CTRL or SHIFT is down (mouse)
		// o SHIFT is down (keyboard)

		if((bCtrl && action == TVC_BYMOUSE) || (bShift && action == TVC_BYKEYBOARD)) {
			// Keep selection at old item
			if(hOld && m_bOldItemSelected)
				SetItemState(hOld, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));
		} else if(bShift && action == TVC_BYMOUSE) {
			// select all items between the old and new item inclusive.
			SelectItems(hOld, hNew);
		} else if(pNMTreeView->action != TVC_UNKNOWN)
			// NOTE: TVC_UNKNOWN is set, if the programmer changes
			// the selection (no user action). We remove all
			// selctions only in the case of a user action!
			// remove all selections made earlier ...
			ClearSelection(/* except */ hNew);
		else if(pNMTreeView->action == TVC_UNKNOWN && m_bOldItemSelected && hOld)
			// Item changed programmatically. Keep old selection
			SetItemState(hOld, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));
	}
}



void CEditTreeCtrlEx::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	if(IsMultiSelectEnabled()) {
		CPoint point;
		GetCursorPos(&point);
		ScreenToClient(&point);

		bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		bool bShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
		
		UINT uFlags ;
		HTREEITEM hNew = HitTest( point, &uFlags ) ;
		HTREEITEM hCur = GetSelectedItem() ;

		// Always keep in mind:
		// If the currently clicked item already has the focus (hNew == hCur)
		// then no TVN_SELCHANG* reflection message will be generated. These
		// special cases will be handled here !
		 
		if(bCtrl) {
			if(hNew == hCur && (uFlags & TVHT_ONITEMLABEL)) {
				// CTRL+LeftClick twice on the same item toggles selection.
				if(GetItemState(hCur, UINT(TVIS_SELECTED)) & TVIS_SELECTED)
					SetItemState(hCur, UINT(~TVIS_SELECTED), UINT(TVIS_SELECTED));
				else
					SetItemState(hCur, UINT(TVIS_SELECTED), UINT(TVIS_SELECTED));
				UpdateWindow();
				return;
			}
		} else if(!bCtrl && !bShift && hNew == hCur && (uFlags & TVHT_ONITEMLABEL) && GetSelectedCount() > 1) {
			// special case: if there is more than one item selected and
			// the user clicks at the item that has the focus (hNew == hCur)
			// then nothing happens (i.e. no reflection message will
			// be generated !). So we have to handle this ourself to get
			// the right behaviour (current item keeps selected and all other
			// selections will be removed):
			ClearSelection(hCur);
			UpdateWindow();
			*pResult = 1;
			return;
		}
	}
}


void CEditTreeCtrlEx::OnKillfocus(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(GetSelectedCount() > 1)
		// Ensure the multiple selected items are drawn correctly when loosing
		// the focus
		InvalidateSelectedItems();
	*pResult = 0;
}


void CEditTreeCtrlEx::OnSetfocus(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if(GetSelectedCount() > 1)
		// Ensure the multiple selected items are drawn correctly when getting
		// the focus
		InvalidateSelectedItems();
	*pResult = 0;
}


void CEditTreeCtrlEx::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	if(pNMTreeView->action == TVE_COLLAPSE) {
		// make sure there are no selected items in collapsed trees
		// otherwise the user might be confused by the behavior of
		// the tree control.
		HTREEITEM hItem = pNMTreeView->itemNew.hItem;
		ASSERT(hItem != 0);
		DeselectTree(GetChildItem(hItem));
	}
}


void CEditTreeCtrlEx::OnLButtonDown(UINT nFlags, CPoint point) 
{
	UINT flags;
	HTREEITEM hItem = HitTest(point, &flags);
	if(IsMultiSelectEnabled() && ((flags & TVHT_ONITEMRIGHT) || (flags & TVHT_NOWHERE))) {
		m_Tracker.SetupTracker(this, point);
	} else
		CEditTreeCtrl::OnLButtonDown(nFlags, point);
	
	CNRAlignmentDoc* pDoc = 
    ((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();
  pDoc->data_pt->ClearRenderingFlags();

	CString str("");
	for(HTREEITEM hItem = GetFirstSelectedItem();
    hItem != 0;
    hItem = GetNextSelectedItem(hItem)) {
		CString itemName = GetItemText(hItem);
		pDoc->data_pt->SetRenderingFlags(itemName);
	}
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->Invalidate( FALSE );
	((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_wndStatusBar.SetPaneText(0, (LPCTSTR)str);
}

void CEditTreeCtrlEx::GetSelectedTexts(vector<CString> &names) {
	names.clear();
	for(HTREEITEM hItem = GetFirstSelectedItem();
    hItem != 0;
    hItem = GetNextSelectedItem(hItem)) {
		CString surf_name = GetItemText(hItem);
		names.push_back(surf_name);
	}
}

BOOL CEditTreeCtrlEx::OnEraseBkgnd(CDC* pDC) {
	if(m_Tracker.IsTracking())
		return true;	// otherwise the tree control will scratch the rubber band...
	return CEditTreeCtrl::OnEraseBkgnd(pDC);
}


#if _MFC_VER >= 0x0420
void CEditTreeCtrlEx::OnCustomTreeDraw(NMHDR * pNMHDR, LRESULT * pResult) {
	*pResult = CDRF_DODEFAULT;

	if (m_pDragData) {
		// Have to take care on the visual effects when dragging multiple items
		// The normal behavior of a tree control would be to hide the selection.
		// With multiple selection this would be too much confusing...
		LPNMTVCUSTOMDRAW pCDRW = (LPNMTVCUSTOMDRAW) pNMHDR;
		switch (pCDRW->nmcd.dwDrawStage ) {
			case CDDS_PREPAINT:
				*pResult = CDRF_NOTIFYITEMDRAW ;	// ask for item notifications
				break ;
			case CDDS_ITEMPREPAINT:
				{
					HTREEITEM hCur = (HTREEITEM) pCDRW->nmcd.dwItemSpec;
					HTREEITEM hDrop = GetDropHilightItem();
					if(hDrop && hCur != hDrop) {
						for(HTREEITEM hItem = GetFirstSelectedItem(); hItem != 0; hItem = GetNextSelectedItem(hItem))
							if(hItem == hCur) {
								// mark selected items with a different background color
								pCDRW->clrTextBk = ::GetSysColor(COLOR_BTNFACE);
								return;
							}
						}
				}
				break;
			default:
				break;
		}
	}
}
#endif // _MFC_VER
