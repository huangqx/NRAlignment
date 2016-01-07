// MainFrm.h : interface of the CMainFrame class
//


#pragma once

#include <vector>
using namespace std;

class COptionView;
class CNRAlignmentView;
class CMainFrame : public CFrameWnd {	
 protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

 // Attributes
 public:
  CSplitterWnd m_wndSplitter;
  CStatusBar m_wndStatusBar;
  CToolBar m_wndToolBar;
  COptionView* m_pOptView;
  CNRAlignmentView* m_pVGView;
 // Operations
 public:

 // Overrides
 public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

 // Implementation
 public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
  vector<int> feature_ids;
  afx_msg void OnRenderingSmoothshading();
  afx_msg void OnUpdateRenderingSmoothshading(CCmdUI *pCmdUI);
  afx_msg void OnRenderingLighting();
  afx_msg void OnUpdateRenderingLighting(CCmdUI *pCmdUI);
  afx_msg void OnRenderingCulling();
  afx_msg void OnUpdateRenderingCulling(CCmdUI *pCmdUI);
  afx_msg void OnRenderingAntialiasing();
  afx_msg void OnUpdateRenderingAntialiasing(CCmdUI *pCmdUI);
protected:
  virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
public:
  afx_msg void OnRenderingmodeTriangularmesh();
  afx_msg void OnUpdateRenderingmodeTriangularmesh(CCmdUI *pCmdUI);
  afx_msg void OnRenderingmodePoint();
  afx_msg void OnUpdateRenderingmodePoint(CCmdUI *pCmdUI);
  afx_msg void OnViewAligningerror();
  afx_msg void OnUpdateViewAligningerror(CCmdUI *pCmdUI);
  afx_msg void OnFileLoadcamera();
  afx_msg void OnFileSavecamera();
  afx_msg void OnViewTexture32932();
  afx_msg void OnUpdateViewTexture32932(CCmdUI *pCmdUI);
  afx_msg void OnViewCurrentscan();
  afx_msg void OnUpdateViewCurrentscan(CCmdUI *pCmdUI);
};


