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

// 07/22/2005:
//		o Fixed bug in OnBegindrag() & OnBeginrdrag()
//		o Fixed Bug in HandleKeyDown() (don't handle keys while in label edit mode)

// EditTreeCtrl.cpp : implementation file
//

#include "NRAlignmentView.h"
#include "NRAlignmentDoc.h"
#include "EditTreeCtrl.h"
#include "CursorDef.h"
#include "MainFrm.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(4:4503)		// decorated name length exceeded


enum ECmdHandler {
 ID_RENAME = 1,
 ID_DELETE,
	ID_LOCK_SCAN,
	ID_UNLOCK_SCAN,

	ID_MAX_CMD
};

enum ERightDragHandler {
	ID_DRAG_COPY = 15,
	ID_DRAG_MOVE,
	ID_DRAG_CANCEL,

	ID_MAX_DRH
};

// --------------------------------------------------------------------------
// CEditTreeCtrl

CEditTreeCtrl::cursormap_t CEditTreeCtrl::m_Cursormap;

CEditTreeCtrl::CEditTreeCtrl()
	: m_pDragData(0) {
	// m_Keymap[key][ctrl-modifier][shift-modifier] = method;
/*	m_Keymap[VK_F2    ][false][false] = &CEditTreeCtrl::DoEditLabel;
	m_Keymap[VK_INSERT][true ][false] = &CEditTreeCtrl::DoInsertChild;
	m_Keymap[VK_INSERT][false][true ] = &CEditTreeCtrl::DoInsertRoot;
	m_Keymap[VK_INSERT][false][false] = &CEditTreeCtrl::DoInsertSibling;
	m_Keymap[VK_DELETE][false][false] = &CEditTreeCtrl::DoDeleteItem;
	m_Keymap['S'      ][true ][false] = &CEditTreeCtrl::DoSortCurrentLevel;
	m_Keymap['S'      ][true ][true ] = &CEditTreeCtrl::DoSortCurrentLevelAndBelow;*/

	 m_Commandmap[ID_RENAME] = &CEditTreeCtrl::DoEditLabel;
	 m_Commandmap[ID_DELETE] = &CEditTreeCtrl::DoDeleteItem;
	 // add by mac for save to povray
	 m_Commandmap[ID_LOCK_SCAN]		= &CEditTreeCtrl::DoLockScan;
	 m_Commandmap[ID_UNLOCK_SCAN]		= &CEditTreeCtrl::DoUnLockScan;
}


CEditTreeCtrl::~CEditTreeCtrl() {
}


bool CEditTreeCtrl::CanEditLabel(HTREEITEM hItem) {
	ATLTRACE2(_T("CEditTreeCtrl::CanEditLabel('%s')\n"), LPCTSTR(GetItemText(hItem)));
	return true;
}


bool CEditTreeCtrl::CanSetLabelText(TVITEM & item) {
	ATLTRACE2(_T("CEditTreeCtrl::CanSetLabelText('%s')\n"), item.pszText);
	return true;
}


bool CEditTreeCtrl::CanDeleteItem(HTREEITEM hItem) {
  if (GetItemText(hItem) =="Shapes"|| GetItemText(hItem) =="RefSurf")
    return false;
	ATLTRACE2(_T("CEditTreeCtrl::CanDeleteItem('%s')\n"), LPCTSTR(GetItemText(hItem)));
	return true;
}

bool CEditTreeCtrl::CanLockItem(HTREEITEM hItem) {
  ATLTRACE2(_T("CEditTreeCtrl::CanLockItem('%s')\n"), LPCTSTR(GetItemText(hItem)));
  CString itemName = GetItemText(hItem);
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();

  return pDoc->data_pt->CanLockScan(itemName);
}

bool CEditTreeCtrl::CanUnLockItem(HTREEITEM hItem) {
  ATLTRACE2(_T("CEditTreeCtrl::CanUnLockItem('%s')\n"), LPCTSTR(GetItemText(hItem)));
  CString itemName = GetItemText(hItem);
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();

  return pDoc->data_pt->CanUnLockScan(itemName);
}

bool CEditTreeCtrl::CanInsertItem(HTREEITEM hItem) {
	ATLTRACE2(_T("CEditTreeCtrl::CanInsertItem('%s')\n"), (hItem && hItem != TVI_ROOT) ? LPCTSTR(GetItemText(hItem)) : _T("<Root>"));
	return true;
}

bool CEditTreeCtrl::CanSortLevel(HTREEITEM hItem) {
	ATLTRACE2(_T("CEditTreeCtrl::CanSortLevel('%s')\n"), (hItem && hItem != TVI_ROOT) ? LPCTSTR(GetItemText(hItem)) : _T("<Root>"));
	return true;
}

// --------------------------------------------------------------------------
// Keyboard handling

bool CEditTreeCtrl::HandleKeyDown(WPARAM wParam, LPARAM lParam) {
	bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
	bool bShift = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;
	HTREEITEM hCur = GetSelectedItem();

	if(GetEditControl() == 0) {
		method fnc = m_Keymap[int(wParam)][bCtrl][bShift];
		if(fnc)
			return (this->*fnc)(hCur);
	}

	// under some circumstances we need to check some more keys
	switch(int(wParam)) {
		case VK_ESCAPE:
			if(m_pDragData) {
				DragStop();
				return true;
			}
			/*FALLTHRU*/

		case VK_RETURN:
			if(GetEditControl() != 0) {
				GetEditControl()->SendMessage(WM_KEYDOWN, wParam, lParam);
				return true;
			}
			break;

		default:
			break;
	}

	return false;
}


bool CEditTreeCtrl::DoEditLabel(HTREEITEM hItem) {
  CString temp = LPCTSTR(GetItemText(hItem));
	 return hItem ? (EditLabel(hItem) != 0) : false;
}


static void SetupInsertStruct(TVINSERTSTRUCT & ins, HTREEITEM hParent) {
	memset(&ins, 0, sizeof(ins));
	ins.item.mask = TVIF_TEXT;
	ins.item.pszText = _T("New Item");
	ins.hParent = hParent;
	ins.hInsertAfter = TVI_LAST;
}


bool CEditTreeCtrl::DoInsertSibling(HTREEITEM hItem) 
{
	ATLTRACE2(_T("CEditTreeCtrl::OnInsertSibling('%s')\n"), LPCTSTR(GetItemText(hItem)));
	TVINSERTSTRUCT ins;
	SetupInsertStruct(ins, GetParentItem(hItem));
	return NewItem(ins);
}


bool CEditTreeCtrl::DoInsertChild(HTREEITEM hItem) 
{
	ATLTRACE2(_T("CEditTreeCtrl::OnInsertChild('%s')\n"), LPCTSTR(GetItemText(hItem)));
	TVINSERTSTRUCT ins;
	SetupInsertStruct(ins, hItem);
	return NewItem(ins);
}


bool CEditTreeCtrl::DoInsertRoot(HTREEITEM hItem) 
{
	ATLTRACE2(_T("CEditTreeCtrl::OnInsertRoot('%s')\n"), LPCTSTR(GetItemText(hItem)));
	TVINSERTSTRUCT ins;
	SetupInsertStruct(ins, TVI_ROOT);
	return NewItem(ins);
}


bool CEditTreeCtrl::DoDeleteItem(HTREEITEM hItem) 
{
	ATLTRACE2(_T("CEditTreeCtrl::OnDeleteItem('%s')\n"), LPCTSTR(GetItemText(hItem)));
	// add by mac for delete surface
	return hItem ? (CanDeleteItem(hItem) && DeleteItem(hItem)) : false;
}

bool CEditTreeCtrl::DoSortCurrentLevel(HTREEITEM hItem) 
{
	ATLTRACE2(_T("CEditTreeCtrl::OnSortCurrentLevel('%s')\n"), LPCTSTR(GetItemText(hItem)));
	SortCurrentLevel(hItem);
	return true;
}


bool CEditTreeCtrl::DoSortCurrentLevelAndBelow(HTREEITEM hItem) 
{
	ATLTRACE2(_T("CEditTreeCtrl::OnSortCurrentLevelAndBelow('%s')\n"), LPCTSTR(GetItemText(hItem)));
	SortCurrentLevelAndBelow(hItem);
	return true;
}


bool CEditTreeCtrl::DoLockScan( HTREEITEM hItem ) {
  ATLTRACE2(_T("CEditTreeCtrl::OnLockScan('%s')\n"), LPCTSTR(GetItemText(hItem)));
  return hItem ? (CanLockItem(hItem) && LockScan(hItem)) : false;
}

bool CEditTreeCtrl::DoUnLockScan( HTREEITEM hItem ) {
  ATLTRACE2(_T("CEditTreeCtrl::OnUnLockItem('%s')\n"), LPCTSTR(GetItemText(hItem)));
  // add by mac for delete surface
  return hItem ? (CanUnLockItem(hItem) && UnLockScan(hItem)) : false;
}

bool CEditTreeCtrl::DeleteScan(HTREEITEM item) {
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();
  
  pDoc->data_pt->DeleteScan(LPCTSTR(GetItemText(item)));
  pDoc->UpdateAllViews(NULL);
  return true;
}

bool CEditTreeCtrl::LockScan(HTREEITEM item) {
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();
//  pDoc->data_pt->ClearRenderingFlags();

  pDoc->data_pt->LockScan(LPCTSTR(GetItemText(item)));
  SetItem(item, TVIF_IMAGE, LPCTSTR(GetItemText(item)), 1, 1, TVIS_BOLD, 
    TVIS_BOLD, 0);
  pDoc->UpdateAllViews(NULL);
  return true;
}

bool CEditTreeCtrl::UnLockScan(HTREEITEM item) {
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();
  pDoc->data_pt->UnLockScan(LPCTSTR(GetItemText(item)));
  SetItem(item, TVIF_IMAGE, LPCTSTR(GetItemText(item)), 2, 2, TVIS_BOLD, 
    TVIS_BOLD, 0);
  pDoc->UpdateAllViews(NULL);
  return true;
}

bool CEditTreeCtrl::NewItem(TVINSERTSTRUCT & ins) 
{
	ATLTRACE2(_T("CEditTreeCtrl::NewItem('%s')\n"), ins.item.pszText ? ins.item.pszText : _T("NULL"));

	if(!CanInsertItem(ins.hParent))
		return false;

	HTREEITEM hItem = InsertItem(&ins);
	if(hItem) {
		SelectItem(hItem);
		OnNewItem(hItem);
		EditLabel(hItem);
	}
	return hItem != 0;
}


void CEditTreeCtrl::OnNewItem(HTREEITEM) 
{
}


// --------------------------------------------------------------------------
// Dragging

CDragData * CEditTreeCtrl::CreateDragData(HTREEITEM hDragItem, bool bRightDrag)
{
	return new CDragData(*this, hDragItem, bRightDrag);
}


CEditTreeCtrl::EDropHint CEditTreeCtrl::GetDropHint(UINT flags) 
{
	EDropHint hint = DROP_NODROP;

	if((flags & TVHT_ONITEMRIGHT))
		hint = DROP_CHILD;

	else if(flags & TVHT_ONITEM) {
		// check whether we should drop below or above
		// the item
		CRect rc;
		if(GetItemRect(GetDropHilightItem(), rc, false)) {
			CPoint pt;
			::GetCursorPos(&pt);
			ScreenToClient(&pt);
			if((pt.y - rc.top) > (rc.bottom - pt.y))
				hint = DROP_BELOW;
			else
				hint = DROP_ABOVE;
		} else
			hint = DROP_ABOVE;

	} else if((flags & TVHT_NOWHERE))
		// below the last item
		hint = DROP_BELOW;

	ASSERT(m_pDragData != 0);
	m_pDragData->SetDropHint(hint);

	return hint;
}


HTREEITEM CEditTreeCtrl::GetDropTarget(EDropHint & hint) {
	ASSERT(m_pDragData != 0);

	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	UINT flags;
	HTREEITEM hDrop = HitTest(pt, &flags);
	hint = GetDropHint(flags);
	m_pDragData->SetDropTarget(hDrop);
	if(hDrop) {
		m_pDragData->DragLeave();	// allow updates

		SelectDropTarget(hDrop);

		if(m_pDragData->CheckExpand(hDrop))
			Expand(hDrop, TVE_EXPAND);

		// Make sure the surrounding items are visible, too
		// This will scroll the tree if necessary.
		HTREEITEM hPrev = GetPrevVisibleItem(hDrop);
		if(hPrev)
			EnsureVisible(hPrev);
		HTREEITEM hNext = GetNextVisibleItem(hDrop);
		if(hNext)
			EnsureVisible(hNext);

		// if the drop target is a descendent of the dragged item, then
		// disallow dropping the item here...
		if(IsAncestor(m_pDragData->GetDragItem(), hDrop) || !CanDropItem(m_pDragData->GetDragItem(), hDrop, hint))
			hint = DROP_NODROP;

		m_pDragData->DragEnter(pt);
	} else if(hint != DROP_NODROP && !CanDropItem(m_pDragData->GetDragItem(), hDrop, hint))
		hint = DROP_NODROP;

	return hDrop;
}


bool CEditTreeCtrl::CanDragItem(TVITEM & item) 
{
	ATLTRACE2(_T("CEditTreeCtrl::CanDragItem('%s')\n"), item.pszText);
	return true;
}


bool CEditTreeCtrl::CanDropItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint) 
{
	ATLTRACE2(_T("CEditTreeCtrl::CanDropItem('%s', '%s')\n"), LPCTSTR(GetItemText(hDrag)), LPCTSTR(GetItemText(hDrop)));
	return true;
}


void CEditTreeCtrl::DragStart() 
{
	ASSERT(m_pDragData != 0);
	ATLTRACE2(_T("CEditTreeCtrl::DragStart('%s')\n"), LPCTSTR(GetItemText(m_pDragData->GetDragItem())));

	m_pDragData->StartDrag();
	SelectItem(m_pDragData->GetDragItem());
}


void CEditTreeCtrl::DragMove() 
{
	ASSERT(m_pDragData != 0);
	ATLTRACE2(_T("CEditTreeCtrl::DragMove('%s')\n"), LPCTSTR(GetItemText(m_pDragData->GetDragItem())));

	EDropHint eHint;
	HTREEITEM hDrop = GetDropTarget(eHint);
	SetDragCursor(hDrop, eHint);
}


void CEditTreeCtrl::DragEnd() 
{
	ASSERT(m_pDragData != 0);
	ATLTRACE2(_T("CEditTreeCtrl::DragEnd('%s')\n"), LPCTSTR(GetItemText(m_pDragData->GetDragItem())));

	EDropHint eHint;
	HTREEITEM hDrop = GetDropTarget(eHint);

	if(m_pDragData->IsRightDragging()) {
		if(eHint == DROP_NODROP)
			DragStop();
		else {
			CPoint point;
			GetCursorPos(&point);
			DragDisplayContextMenu(point);
		}
	} else {
		bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
		DragMoveItem(m_pDragData->GetDragItem(), hDrop, eHint, bCtrl);
		DragStop();
	}
}


void CEditTreeCtrl::DragStop() 
{
	ATLTRACE2(_T("CEditTreeCtrl::DragStop()\n"));

	// reset dragging variables...
	ASSERT(m_pDragData != 0);
	m_pDragData->EndDrag();
	delete m_pDragData;
	m_pDragData = 0;
}


void CEditTreeCtrl::SetDragCursor(HTREEITEM hDropTarget, EDropHint eHint) 
{
	if(m_Cursormap.size() == 0)
		CreateCursorMap();

	bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;

	SetCursor(m_Cursormap[eHint][bCtrl]);
}


void CEditTreeCtrl::DragDisplayContextMenu(CPoint & point) {
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	VERIFY(menu.AppendMenu(MF_STRING, ID_DRAG_COPY, _T("Copy Here")));
	VERIFY(menu.AppendMenu(MF_STRING, ID_DRAG_MOVE, _T("Move Here")));
	VERIFY(menu.AppendMenu(MF_SEPARATOR));
	VERIFY(menu.AppendMenu(MF_STRING, ID_DRAG_CANCEL, _T("Cancel")));

	bool bCtrl = (::GetKeyState(VK_CONTROL) & 0x8000) != 0;
	menu.SetDefaultItem(bCtrl ? ID_DRAG_COPY : ID_DRAG_MOVE);

	menu.TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}


void CEditTreeCtrl::DragMoveItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint eHint, bool bCopy) {
	if(eHint == DROP_NODROP)
		return;

	if(IsAncestor(hDrag, hDrop) || !CanDropItem(hDrag, hDrop, eHint))
		return;

	ASSERT(hDrag != 0);
	ASSERT(hDrop != 0 || eHint == DROP_BELOW);

	if(!hDrop && eHint == DROP_BELOW)
		for(hDrop = GetRootItem(); GetNextSiblingItem(hDrop) != 0; hDrop = GetNextSiblingItem(hDrop));

	// Setup insertion parameters
	HTREEITEM hInsertAfter = 0;
	HTREEITEM hParent = 0;
	switch(eHint) {
		case DROP_BELOW:
			hInsertAfter = hDrop;
			hParent = GetParentItem(hDrop);
			break;

		case DROP_ABOVE:
			hInsertAfter = GetPrevSiblingItem(hDrop);
			if(!hInsertAfter)
				hInsertAfter = TVI_FIRST;
			hParent = GetParentItem(hDrop);
			break;

		case DROP_CHILD:
			hInsertAfter = TVI_LAST;
			hParent = hDrop;
			break;

		default:
			ASSERT(false);
			break;
	}

	HTREEITEM hNew = CopyItem(hDrag, hParent, hInsertAfter);
	SelectItem(hNew);

	// If the control-key ist down, we copy the data, otherwise we move
	// it, thus we have to delete the dragged item.
	if(!bCopy)
		DeleteItem(hDrag);
}


HTREEITEM CEditTreeCtrl::CopyItem(HTREEITEM hOrig, HTREEITEM hParent, HTREEITEM hInsertAfter) {
	// Get item information
	TVITEMEX item;
	memset(&item, 0, sizeof(item));
	item.mask = TVIF_HANDLE|TVIF_IMAGE|TVIF_PARAM|TVIF_SELECTEDIMAGE|TVIF_STATE|TVIF_INTEGRAL ;
	item.stateMask = TVIS_OVERLAYMASK|TVIS_STATEIMAGEMASK;
	item.hItem = hOrig;
	VERIFY(GetItem((TVITEM*)&item));
	CString strText = GetItemText(hOrig);

	// Insert new item and copy data
	HTREEITEM hNew = InsertItem(strText, hParent, hInsertAfter);
	ASSERT(hNew != 0);
	item.hItem = hNew;
	VERIFY(SetItem((TVITEM*)&item));

	// copy entire subtree, too
	if(ItemHasChildren(hOrig))
		for(HTREEITEM hChild = GetChildItem(hOrig); hChild != 0; hChild = GetNextSiblingItem(hChild))
			CopyItem(hChild, hNew, TVI_LAST);

	return hNew;
}

// --------------------------------------------------------------------------
// misc

void CEditTreeCtrl::DisplayContextMenu(CPoint & point) 
{
	CPoint pt(point);
	ScreenToClient(&pt);
	UINT flags;
	HTREEITEM hItem = HitTest(pt, &flags);
	bool bOnItem = (flags & TVHT_ONITEM) != 0;

	CMenu add;
	VERIFY(add.CreatePopupMenu());
	if(bOnItem) {
/*		if(CanInsertItem(GetParentItem(hItem)))
			VERIFY(add.AppendMenu(MF_STRING, ID_ADD_SIBLING, _T("New Sibling\tINS")));
		if(CanInsertItem(hItem))
			VERIFY(add.AppendMenu(MF_STRING, ID_ADD_CHILD, _T("New Child Item\tCtrl+INS")));*/
	}
//	if(CanInsertItem(0))
//		VERIFY(add.AppendMenu(MF_STRING, ID_ADD_ROOT, _T("New Root Item\tShift+INS")));

//	CMenu sort;
//	VERIFY(sort.CreatePopupMenu());
//	VERIFY(sort.AppendMenu(MF_STRING, ID_SORT_LEVEL, _T("Current Level\tCtrl+S")));
//	VERIFY(sort.AppendMenu(MF_STRING, ID_SORT_LEVELANDBELOW, _T("Current Level And Below\tCtrl+Shift+S")));

	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	if (bOnItem &&
    GetItemText(hItem) !="Shapes" &&
    GetItemText(hItem) !="RefSurf") {
		if (CanEditLabel(hItem))
      VERIFY(menu.AppendMenu(MF_STRING, ID_RENAME, _T("Rename\tF2")));
		if (CanDeleteItem(hItem))
      VERIFY(menu.AppendMenu(MF_STRING, ID_DELETE, _T("Delete\tDEL")));
    if (CanLockItem(hItem))
      VERIFY(menu.AppendMenu(MF_STRING, ID_LOCK_SCAN, _T("Lock\tL")));
    if (CanUnLockItem(hItem))
      VERIFY(menu.AppendMenu(MF_STRING, ID_UNLOCK_SCAN, _T("UnLock\tUL")));
	}
/*	if(add.GetMenuItemCount() > 0)
		VERIFY(menu.AppendMenu(MF_POPUP, UINT(add.GetSafeHmenu()), _T("Add")));
	if(bOnItem) {
		if(menu.GetMenuItemCount() > 0)
			VERIFY(menu.AppendMenu(MF_SEPARATOR));
		VERIFY(menu.AppendMenu(MF_POPUP, UINT(sort.GetSafeHmenu()), _T("Sort")));
	}*/

	ExtendContextMenu(menu);

	// maybe the menu is empty...
	if(menu.GetMenuItemCount() > 0)
		menu.TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
}


void CEditTreeCtrl::ExtendContextMenu(CMenu & menu) {
}


bool CEditTreeCtrl::IsAncestor(HTREEITEM hItem, HTREEITEM hCheck) const {
	for(HTREEITEM hParent = hCheck; hParent != 0; hParent = GetParentItem(hParent))
		if(hParent == hItem)
			return true;
	return false;
}


void CEditTreeCtrl::SortCurrentLevel(HTREEITEM hItem) {
	if(!hItem)
		hItem = GetSelectedItem();
	if(CanSortLevel(hItem))
		SortChildren(hItem ? GetParentItem(hItem) : hItem);
}


void CEditTreeCtrl::SortCurrentLevelAndBelow(HTREEITEM hItem) {
	SortCurrentLevel(hItem);
	for(hItem = hItem ? GetChildItem(GetParentItem(hItem)) : GetRootItem(); hItem != 0; hItem = GetNextSiblingItem(hItem))
		if(ItemHasChildren(hItem))
			SortCurrentLevelAndBelow(GetChildItem(hItem));
}


BOOL CEditTreeCtrl::PreTranslateMessage(MSG* pMsg) {
	switch(pMsg->message) {
		case WM_KEYDOWN:
			if(HandleKeyDown(pMsg->wParam, pMsg->lParam))
				return true;
			break;

		default:
			break;
	}
	return CTreeCtrl::PreTranslateMessage(pMsg);
}


void CEditTreeCtrl::CreateCursorMap() {
	// make sure this method is only called once.
	ASSERT(m_Cursormap.size() == 0);

	HINSTANCE hInst = HINSTANCE(::GetModuleHandle(0));
	HCURSOR hNoDrop    = ::CreateCursor(hInst, HSpotXNoDrop,    HSpotYNoDrop,    32, 32, ANDmaskNoDrop,    XORmaskNoDrop);
	HCURSOR hMoveAbove = ::CreateCursor(hInst, HSpotXMoveAbove, HSpotYMoveAbove, 32, 32, ANDmaskMoveAbove, XORmaskMoveAbove);
	HCURSOR hCopyAbove = ::CreateCursor(hInst, HSpotXCopyAbove, HSpotYCopyAbove, 32, 32, ANDmaskCopyAbove, XORmaskCopyAbove);
	HCURSOR hMoveBelow = ::CreateCursor(hInst, HSpotXMoveBelow, HSpotYMoveBelow, 32, 32, ANDmaskMoveBelow, XORmaskMoveBelow);
	HCURSOR hCopyBelow = ::CreateCursor(hInst, HSpotXCopyBelow, HSpotYCopyBelow, 32, 32, ANDmaskCopyBelow, XORmaskCopyBelow);
	HCURSOR hMoveChild = ::CreateCursor(hInst, HSpotXMoveChild, HSpotYMoveChild, 32, 32, ANDmaskMoveChild, XORmaskMoveChild);
	HCURSOR hCopyChild = ::CreateCursor(hInst, HSpotXCopyChild, HSpotYCopyChild, 32, 32, ANDmaskCopyChild, XORmaskCopyChild);

	ASSERT(hNoDrop    != 0);
	ASSERT(hMoveAbove != 0);
	ASSERT(hCopyAbove != 0);
	ASSERT(hMoveBelow != 0);
	ASSERT(hCopyBelow != 0);
	ASSERT(hMoveChild != 0);
	ASSERT(hCopyChild != 0);

	// The first index is the drop-hint, the second is an indicator for the control-key.
	m_Cursormap[DROP_NODROP][true ] = hNoDrop;
	m_Cursormap[DROP_NODROP][false] = hNoDrop;
	m_Cursormap[DROP_BELOW ][true ] = hCopyBelow;
	m_Cursormap[DROP_BELOW ][false] = hMoveBelow;
	m_Cursormap[DROP_ABOVE ][true ] = hCopyAbove;
	m_Cursormap[DROP_ABOVE ][false] = hMoveAbove;
	m_Cursormap[DROP_CHILD ][true ] = hCopyChild;
	m_Cursormap[DROP_CHILD ][false] = hMoveChild;
}


BEGIN_MESSAGE_MAP(CEditTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CEditTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT, OnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_NOTIFY_REFLECT(TVN_BEGINRDRAG, OnBeginrdrag)
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_SELCHANGING, OnSelchanging)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_RENAME, ID_MAX_CMD-1, OnContextCmd)
//	ON_COMMAND_RANGE(ID_DRAG_COPY, ID_MAX_DRH-1, OnDragContextCmd)
END_MESSAGE_MAP()

// --------------------------------------------------------------------------
// CEditTreeCtrl message handlers

void CEditTreeCtrl::OnContextCmd(UINT id) {
	HTREEITEM hCur = GetSelectedItem();

	method fnc = m_Commandmap[id];
	if(fnc) {
		(this->*fnc)(hCur);
		return;
	}

	// unknown command in context menu
	ASSERT(false);
}


void CEditTreeCtrl::OnDragContextCmd(UINT id) {
	// Should only reach this during a right-drag...
	ASSERT(m_pDragData != 0);

	if(id != ID_DRAG_CANCEL) {
		bool bCopy = (id == ID_DRAG_COPY);
		DragMoveItem(m_pDragData->GetDragItem(), m_pDragData->GetDropTarget(), EDropHint(m_pDragData->GetDropHint()), bCopy);
	}

	DragStop();
}


void CEditTreeCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	*pResult = 1;

	if(CanEditLabel(pTVDispInfo->item.hItem))
		*pResult = 0;
}

void CEditTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	TVITEM & item = pTVDispInfo->item;
	*pResult = 1;

	if(item.pszText && CanSetLabelText(item)) {
		SetItem(&item);
		*pResult = 0;
	}
}

void CEditTreeCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	if(CanDragItem(pNMTreeView->itemNew)) {
		m_pDragData = CreateDragData(pNMTreeView->itemNew.hItem, false);
		DragStart();
	}
}

void CEditTreeCtrl::OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;

	if(CanDragItem(pNMTreeView->itemNew)) {
		m_pDragData = CreateDragData(pNMTreeView->itemNew.hItem, true);
		DragStart();
	}
}

void CEditTreeCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	if(m_pDragData)
		DragMove();
	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CEditTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_pDragData)
		DragEnd();
	CTreeCtrl::OnLButtonUp(nFlags, point);
}


void CEditTreeCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if(!m_pDragData)
		CTreeCtrl::OnRButtonDown(nFlags, point);
}

void CEditTreeCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	if(m_pDragData) {
		if(m_pDragData->IsRightDragging())
			DragEnd();
		else
			// Right-click during a left-click-drag breaks the drag operation
			DragStop();
	} else
		CTreeCtrl::OnRButtonUp(nFlags, point);
}

void CEditTreeCtrl::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	DisplayContextMenu(point);
}

void CEditTreeCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 1;
	UINT flags;
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);
	HTREEITEM hItem = HitTest(point, &flags);
	if(hItem && (flags & TVHT_ONITEM) && !(flags & TVHT_ONITEMRIGHT))
		SelectItem(hItem);
	ClientToScreen(&point);
	DisplayContextMenu(point);
}

void CEditTreeCtrl::OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	*pResult = 0;
	UINT & action = pNMTreeView->action;
	if(action == TVC_UNKNOWN) {
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
				HTREEITEM hNew = pNMTreeView->itemNew.hItem;
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
}
