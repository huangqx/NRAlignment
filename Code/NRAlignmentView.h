// NRAlignmentView.h : interface of the CNRAlignmentView class
//


#pragma once

#include <afxwin.h>
#include <afxext.h>

#include "scene.h"

class CNRAlignmentDoc;
class CNRAlignmentView : public CView {
 protected: // create from serialization only
	CNRAlignmentView();
	DECLARE_DYNCREATE(CNRAlignmentView)

 // Attributes
 public:
	CNRAlignmentDoc* GetDocument() const;
  CPoint m_ptStart;
  bool pick_start;
  //Describe the whole environment
  Scene3Dim	m_scene;
  // Operations
 public:
 // Overrides
 public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
 protected:

 // Implementation
 public:
	virtual ~CNRAlignmentView();
 #ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
 #endif

 protected:

 // Generated message map functions
 protected:
	DECLARE_MESSAGE_MAP()
 public:
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnMouseMove(UINT nFlags, CPoint point);
  afx_msg void OnPaint();
  afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
  afx_msg void OnSize(UINT nType, int cx, int cy);
};

#ifndef _DEBUG  // debug version in NRAlignmentView.cpp
inline CNRAlignmentDoc* CNRAlignmentView::GetDocument() const
   { return reinterpret_cast<CNRAlignmentDoc*>(m_pDocument); }
#endif

