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


#if !defined(AFX_EDITTREECTRL_H__508278DE_BE33_4505_80FE_51C64B2B3EBB__INCLUDED_)
#define AFX_EDITTREECTRL_H__508278DE_BE33_4505_80FE_51C64B2B3EBB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditTreeCtrl.h : header file
//

#pragma warning(4:4786)		// identifier too long in debug...

#include <map>

#include <afxcmn.h>

//---------------------------------------------------------------------------
// CEditTreeCtrl window

class CDragData;


class CEditTreeCtrl : public CTreeCtrl {
	protected:
		enum EDropHint {
			DROP_BELOW,
			DROP_ABOVE,
			DROP_CHILD,
			DROP_NODROP
		};

		typedef bool (CEditTreeCtrl::*method)(HTREEITEM);
		typedef std::map<bool, method>				shiftmap_t;
		typedef std::map<bool, shiftmap_t>			ctrlmap_t;
		typedef std::map<int, ctrlmap_t>			keymap_t;

		typedef std::map<bool, HCURSOR>				cursctrlmap_t;
		typedef std::map<EDropHint, cursctrlmap_t>	cursormap_t;

		typedef std::map<UINT, method>				cmdmap_t;

		keymap_t	m_Keymap;
		cmdmap_t	m_Commandmap;

		static cursormap_t	m_Cursormap;	// only once per session;

		// dragging
		CDragData *	m_pDragData;

	// Construction
	public:
		CEditTreeCtrl();

	// Attributes
	public:

	// Operations
	public:

	// Permissions
	protected:
			// Override this to control the behavior when a user wants to
			// edit an item's text. The default implementation always returns true.
		virtual bool	CanEditLabel(HTREEITEM hItem);

			// Override to control the setting of text for a label. Check the
			// pszText member of the TVITEM structure. The default implementation
			// always returns true. The <item> contains the new text.
		virtual bool	CanSetLabelText(TVITEM & item);

			// Is it allowed to delete this item?
			// The default implementation always returns true.
		virtual bool	CanDeleteItem(HTREEITEM hItem);

  // Is it allowed to delete this item?
  // The default implementation always returns true.
  virtual bool	CanLockItem(HTREEITEM hItem);

  // Is it allowed to delete this item?
  // The default implementation always returns true.
  virtual bool	CanUnLockItem(HTREEITEM hItem);

			// Is it OK to add a child item to this item?
			// <hParent> == NULL means top-level
			// The default implementation always returns true.
		virtual bool	CanInsertItem(HTREEITEM hParent);

			// Can we sort the level, the given item lies in?
			// This is called from the default implementation
			// of SortCurrentLevel() (see below)
			// if hItem is NULL, the root level is meant.
		virtual bool	CanSortLevel(HTREEITEM hItem);
		
			// Override to (dis-)allow dragging of an item.
			// The default implementation always allows dragging.
		virtual bool	CanDragItem(TVITEM & item);

			// Override to (dis-)allow dropping an item at this specific point.
			// The default implementation always allows dropping.
		virtual bool	CanDropItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint hint);

	// Overridables
	protected:
			// Override if you want to use other keys than the default.
		virtual bool	HandleKeyDown(WPARAM wParam, LPARAM lParam);

			// The following will be called whenever the user wants to insert a new item.
			// You might override this method to change the default text or other
			// settings for the new item.
			// Returns true, if the item could be added, false if not.
		virtual bool	NewItem(TVINSERTSTRUCT & ins);

			// OnNewItem() is called by the default implementation of NewItem()
			// after the new item was created and made the selected one and
			// before the editor is started to rename the item.
		virtual void	OnNewItem(HTREEITEM hItem);

			// The default-implementation creates the cursor-map
			// from the bit-masks described in "CursorDef.h".
			// You might override this method to provide other
			// cursors.
			// Note that this function will only be called once per session!
			// All CEditTreeCtrl objects will use the same cursor map!
		virtual void	CreateCursorMap();

			// Sort the level, the given item is member of.
		virtual void	SortCurrentLevel(HTREEITEM hItem = 0);

			// Sort the level, the given item is member of, and all sublevels.
			// With hItem set to zero, the entire tree will be sorted.
		virtual void	SortCurrentLevelAndBelow(HTREEITEM hItem = 0);

			// The default implementation will popup a context menu that offers
			// most of the tree's functionality.
			// 'point' is in screen coordinates
		virtual void	DisplayContextMenu(CPoint & point);

			// For your own context menu you have two choices:
			// 1. override "DisplayContextMenu()" to support a completly different menu
			// 2. override "ExtendContextMenu()" to provide additional functionality
			// The default implementation simply does nothing.
		virtual void	ExtendContextMenu(CMenu & menu);

	// Dragging
	protected:
			// Start dragging an item.
		virtual void	DragStart();

			// Move the dragging item.
		virtual void	DragMove();

			// End dragging the item.
		virtual void	DragEnd();

			// Reset dragging.
		virtual void	DragStop();

			// Set the dragcursor.
		virtual void	SetDragCursor(HTREEITEM hDropTarget, EDropHint hint);

			// Move (or copy) an item to its new position.
		virtual void	DragMoveItem(HTREEITEM hDrag, HTREEITEM hDrop, EDropHint hint, bool bCopy);

			// When right-dragging an item, we display a context menu as soon as
			// the user releases the right mouse button.
			// 'point' is in screen coordinates
		virtual void	DragDisplayContextMenu(CPoint & point);

			// If you derive your own version of the CDragData class (see below)
			// you have to override this method.
		virtual CDragData *	CreateDragData(HTREEITEM hDragItem, bool bRightDrag);

	// Keymapper
	protected:
		virtual bool	DoEditLabel(HTREEITEM);
		virtual bool	DoInsertSibling(HTREEITEM);
		virtual bool	DoInsertChild(HTREEITEM);
		virtual bool	DoInsertRoot(HTREEITEM);
		virtual bool	DoDeleteItem(HTREEITEM);
		virtual bool	DoSortCurrentLevel(HTREEITEM);
		virtual bool	DoSortCurrentLevelAndBelow(HTREEITEM);
		virtual bool	DoLockScan(HTREEITEM);
		virtual bool	DoUnLockScan(HTREEITEM);
  virtual bool DeleteScan(HTREEITEM item);
  virtual bool LockScan(HTREEITEM item);
  virtual bool UnLockScan(HTREEITEM item);

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CEditTreeCtrl)
		public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CEditTreeCtrl();

	protected:
			// Generate a drop hint from the HitTest()-flags
		EDropHint	GetDropHint(UINT flags);

			// Get the drop target item
		HTREEITEM	GetDropTarget(EDropHint & hint);
		
			// Create a copy of the item <hOrig> and put it under <hParent> at
			// position <hInsertAfter>.
			// This method copies children, too!
		HTREEITEM	CopyItem(HTREEITEM hOrig, HTREEITEM hParent, HTREEITEM hInsertAfter);

			// is <hItem> a direct ancestor of <hCheck>?
		bool		IsAncestor(HTREEITEM hItem, HTREEITEM hCheck) const;

		// Generated message map functions
		//{{AFX_MSG(CEditTreeCtrl)
		afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnMouseMove(UINT nFlags, CPoint point);
		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
		afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnBeginrdrag(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

		afx_msg void	OnContextCmd(UINT uID);
		afx_msg void	OnDragContextCmd(UINT uID);

		DECLARE_MESSAGE_MAP()
};


//---------------------------------------------------------------------------
// helper class CDragData

// If you want your own drag-data container, derive a class from CDragData and
// override the 'CreateDragData()' member of your derived CEditTreeCtrl class.

class CDragData {
	protected:
		CEditTreeCtrl &	m_Owner;
		HCURSOR			m_hOrigCursor;	// Original cursor (when not in dragging mode)
		HTREEITEM		m_hItemDrag;	// The item being dragged
		HTREEITEM		m_hLastDrop;	// The point where the last DragMove() occured
		USHORT			m_usNoMove;		// helper to auto-expand a collapsed node while dragging an item over it
		CImageList *	m_pDragImg;		// image of the item being dragged
		bool			m_bRightDrag;	// Are we dragging the item with the right mouse button?
		int				m_nDropHint;	// How to drop the item being dragged?

	public:
		CDragData(CEditTreeCtrl & tree, HTREEITEM hItem, bool bRightDrag)
			: m_Owner(tree)
			, m_hOrigCursor(::GetCursor())
			, m_hItemDrag(hItem)
			, m_hLastDrop(0)
			, m_usNoMove(0)
			, m_pDragImg(0)
			, m_bRightDrag(bRightDrag)
			, m_nDropHint(0)
		{
			// all real initialization is done in StartDrag() (see below)
		}

		virtual ~CDragData() {
					ASSERT(m_pDragImg == 0);
				}

	public:
		virtual void	StartDrag() {
							CreateDragImage();
							m_Owner.SetCapture();
						}

		virtual void	EndDrag() {
							ReleaseDragImage();
							m_Owner.SelectDropTarget(0);
							m_Owner.EnsureVisible(m_Owner.GetSelectedItem());
							::SetCursor(m_hOrigCursor);
							::ReleaseCapture();
						}

		virtual void	CreateDragImage() {
							ASSERT(m_pDragImg == 0);
							m_pDragImg = m_Owner.CreateDragImage(m_hItemDrag);
							if(m_pDragImg) {
								CPoint pt;
								::GetCursorPos(&pt);
								m_Owner.ScreenToClient(&pt);
								m_pDragImg->DragShowNolock(TRUE);  // lock updates and show drag image
								m_pDragImg->SetDragCursorImage(0,CPoint(0,0));  // define the hot spot for the new cursor image
								m_pDragImg->BeginDrag(0,CPoint(0,0));
								m_pDragImg->DragEnter(&m_Owner,pt);
								m_pDragImg->DragMove(pt);
							}
						}

		virtual void	ReleaseDragImage() {
							if(m_pDragImg) {
								DragLeave();
								m_pDragImg->EndDrag();
								delete m_pDragImg;
							}
							m_pDragImg = 0;
						}

		HTREEITEM		GetDragItem() const { return m_hItemDrag; }
		HTREEITEM		GetDropTarget() const { return m_hLastDrop; }
		void			SetDropTarget(HTREEITEM hItem) { m_hLastDrop = hItem; }
		int				GetDropHint() const { return m_nDropHint; }
		void			SetDropHint(int hint) { m_nDropHint = hint; }
		bool			IsRightDragging() const { return m_bRightDrag; }

		void			DragLeave() {
							if(m_pDragImg)
								m_pDragImg->DragLeave(&m_Owner);
						}

		void			DragEnter(CPoint & pt) {
							if(m_pDragImg)
								m_pDragImg->DragEnter(&m_Owner, pt);
						}

		virtual bool	CheckExpand(HTREEITEM hDrop) {
							if(hDrop == m_hLastDrop) {
								if(++m_usNoMove >= 15) {
									// mouse position unchanged for at least 15 circles.
									// If we're standing on a collapsed node - expand it now
									m_usNoMove = 0;
									return true;
								}
							} else {
								m_usNoMove = 0;	// reset counter if mouse really moved...
								m_hLastDrop = hDrop;
							}
							return false;
						}
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITTREECTRL_H__508278DE_BE33_4505_80FE_51C64B2B3EBB__INCLUDED_)
