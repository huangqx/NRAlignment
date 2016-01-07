// NRAlignmentView.cpp : implementation of the CNRAlignmentView class
//

#include "stdafx.h"
#include "NRAlignment.h"
#include <iostream>

#include "NRAlignmentDoc.h"
#include "NRAlignmentView.h"
#include "data_container.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNRAlignmentView

IMPLEMENT_DYNCREATE(CNRAlignmentView, CView)

BEGIN_MESSAGE_MAP(CNRAlignmentView, CView)
  ON_WM_CREATE()
  ON_WM_ERASEBKGND()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_PAINT()
  ON_WM_RBUTTONDOWN()
  ON_WM_RBUTTONUP()
  ON_WM_SIZE()
END_MESSAGE_MAP()

// CNRAlignmentView construction/destruction

CNRAlignmentView::CNRAlignmentView() {
	// TODO: add construction code here
  pick_start = false;
}

CNRAlignmentView::~CNRAlignmentView() {
}

BOOL CNRAlignmentView::PreCreateWindow(CREATESTRUCT& cs) {
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CNRAlignmentView drawing

void CNRAlignmentView::OnDraw(CDC* /*pDC*/) {
	CNRAlignmentDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CNRAlignmentView diagnostics

#ifdef _DEBUG
void CNRAlignmentView::AssertValid() const {
	CView::AssertValid();
}

void CNRAlignmentView::Dump(CDumpContext& dc) const {
	CView::Dump(dc);
}

CNRAlignmentDoc* CNRAlignmentView::GetDocument() const {
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNRAlignmentDoc)));
	return (CNRAlignmentDoc*)m_pDocument;
}
#endif //_DEBUG


// CNRAlignmentView message handlers

int CNRAlignmentView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
  if (CView::OnCreate(lpCreateStruct) == -1)
    return -1;

  // TODO:  Add your specialized creation code here
  CNRAlignmentDoc *pDoc	= GetDocument();
  m_scene.GetDisplay()->SetData(pDoc->data_pt);
  HWND hWnd = GetSafeHwnd();
  m_scene.OnCreate(hWnd);
  return 0;
}

BOOL CNRAlignmentView::OnEraseBkgnd(CDC* pDC) {
  // TODO: Add your message handler code here and/or call default
  return TRUE;
}

void CNRAlignmentView::OnLButtonDown(UINT nFlags, CPoint point) {
  // TODO: Add your message handler code here and/or call default
  m_ptStart		= point;
  CView::OnLButtonDown(nFlags, point);
}

void CNRAlignmentView::OnLButtonUp(UINT nFlags, CPoint point) {
  // TODO: Add your message handler code here and/or call default
  if(nFlags & MK_CONTROL)
    return;
  if(nFlags & MK_SHIFT)
    return;
  if(::GetKeyState(VK_MENU) & 0xf0)
    return;
  CView::OnLButtonUp(nFlags, point);
}

void CNRAlignmentView::OnMouseMove(UINT nFlags, CPoint point) {
  // TODO: Add your message handler code here and/or call default
  CSize dif = m_ptStart - point;
  m_ptStart = point;
  m_scene.GetDisplay()->MouseMove(nFlags, dif.cx, dif.cy);
  Invalidate(FALSE);
  CView::OnMouseMove(nFlags, point);
}

void CNRAlignmentView::OnPaint() {
  CPaintDC dc(this); // device context for painting
  // TODO: Add your message handler code here
  // Do not call CView::OnPaint() for painting messages
  HWND hWnd = GetSafeHwnd();

  m_scene.DrawScene(hWnd, dc);
}

void CNRAlignmentView::OnRButtonDown(UINT nFlags, CPoint point) {
  // TODO: Add your message handler code here and/or call default
  CView::OnRButtonDown(nFlags, point);
}

void CNRAlignmentView::OnRButtonUp(UINT nFlags, CPoint point) {
  // TODO: Add your message handler code here and/or call default
  CNRAlignmentDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  CView::OnRButtonUp(nFlags, point);
}

void CNRAlignmentView::OnSize(UINT nType, int cx, int cy) {
  CView::OnSize(nType, cx, cy);

  // TODO: Add your message handler code here
  m_scene.GetDisplay()->Reshape(cx, cy);
  Invalidate(FALSE);
}
