// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "NRAlignment.h"

#include "MainFrm.h"
#include "OptionView.h"
#include "data_container.h"
#include "NRAlignmentDoc.h"
#include "NRAlignmentView.h"
#include "console.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
  ON_COMMAND(ID_RENDERING_SMOOTHSHADING, &CMainFrame::OnRenderingSmoothshading)
  ON_COMMAND(ID_RENDERING_LIGHTING, &CMainFrame::OnRenderingLighting)
  ON_COMMAND(ID_RENDERING_CULLING, &CMainFrame::OnRenderingCulling)
  ON_COMMAND(ID_RENDERING_ANTIALIASING, &CMainFrame::OnRenderingAntialiasing)
  ON_UPDATE_COMMAND_UI(ID_RENDERING_SMOOTHSHADING, &CMainFrame::OnUpdateRenderingSmoothshading)
  ON_UPDATE_COMMAND_UI(ID_RENDERING_LIGHTING, &CMainFrame::OnUpdateRenderingLighting)
  ON_UPDATE_COMMAND_UI(ID_RENDERING_CULLING, &CMainFrame::OnUpdateRenderingCulling)
  ON_UPDATE_COMMAND_UI(ID_RENDERING_ANTIALIASING, &CMainFrame::OnUpdateRenderingAntialiasing)
  ON_COMMAND(ID_RENDERINGMODE_TRIANGULARMESH, &CMainFrame::OnRenderingmodeTriangularmesh)
  ON_UPDATE_COMMAND_UI(ID_RENDERINGMODE_TRIANGULARMESH, &CMainFrame::OnUpdateRenderingmodeTriangularmesh)
  ON_COMMAND(ID_RENDERINGMODE_POINT, &CMainFrame::OnRenderingmodePoint)
  ON_UPDATE_COMMAND_UI(ID_RENDERINGMODE_POINT, &CMainFrame::OnUpdateRenderingmodePoint)
  ON_COMMAND(ID_VIEW_ALIGNINGERROR, &CMainFrame::OnViewAligningerror)
  ON_UPDATE_COMMAND_UI(ID_VIEW_ALIGNINGERROR, &CMainFrame::OnUpdateViewAligningerror)
  ON_COMMAND(ID_FILE_LOADCAMERA, &CMainFrame::OnFileLoadcamera)
  ON_COMMAND(ID_FILE_SAVECAMERA, &CMainFrame::OnFileSavecamera)
  ON_COMMAND(ID_VIEW_TEXTURE32932, &CMainFrame::OnViewTexture32932)
  ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTURE32932, &CMainFrame::OnUpdateViewTexture32932)
  ON_COMMAND(ID_VIEW_CURRENTSCAN, &CMainFrame::OnViewCurrentscan)
  ON_UPDATE_COMMAND_UI(ID_VIEW_CURRENTSCAN, &CMainFrame::OnUpdateViewCurrentscan)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame() {
	// TODO: add member initialization code here
  RedirectIOToConsole(LPCWSTR("Console."));
  feature_ids.resize(6);
  feature_ids[0] = 10;
  feature_ids[1] = 3141;
  feature_ids[2] = 3072;
  feature_ids[3] = 2912;
  feature_ids[4] = 6334;
  feature_ids[5] = 6339;
}

CMainFrame::~CMainFrame() {
  FreeConsole();
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
		 | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_MAXIMIZE | WS_SYSMENU;

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext) {
  // TODO: Add your specialized code here and/or call the base class
  CRect rect;
  GetClientRect( &rect );
  int width	= 2560;
  int	height	= 1600;
  int	width0  = 355;
  if (rect.Width()< 1200) {
    width = 1440;
    height= 900;
  }

  m_wndSplitter.CreateStatic(this, 1, 2);
  m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(COptionView),
    CSize(width0, height-220), pContext);
  m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CNRAlignmentView),
    CSize(width - width0, height-220), pContext);
  m_wndSplitter.SetColumnInfo(0, width0, 0);
  m_wndSplitter.SetColumnInfo(1, width - width0, 0);

  m_pOptView = (COptionView*)m_wndSplitter.GetPane(0, 0);
  m_pVGView	= (CNRAlignmentView*)m_wndSplitter.GetPane(0, 1);

  return TRUE;
}

void CMainFrame::OnRenderingSmoothshading() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateSmoothShadingFlag();
  InvalidateRect(NULL,FALSE);
}

void CMainFrame::OnUpdateRenderingSmoothshading(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->UsingSmoothShading());
}

void CMainFrame::OnRenderingLighting() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateLightingFlag();
  InvalidateRect(NULL,FALSE);
}

void CMainFrame::OnUpdateRenderingLighting(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->UsingLighting());
}

void CMainFrame::OnRenderingCulling() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateCullingFlag();
  InvalidateRect(NULL,FALSE);
}

void CMainFrame::OnUpdateRenderingCulling(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->UsingCulling());
}

void CMainFrame::OnRenderingAntialiasing() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateAntialiasingFlag();
  InvalidateRect(NULL,FALSE);
}

void CMainFrame::OnUpdateRenderingAntialiasing(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->UsingAntialiasing());
}

void CMainFrame::OnRenderingmodeTriangularmesh() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateRenderingMode();
  InvalidateRect(NULL,FALSE);
}

void CMainFrame::OnUpdateRenderingmodeTriangularmesh(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->CheckRenderingModeMesh());
}

void CMainFrame::OnRenderingmodePoint() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateRenderingMode();
  InvalidateRect(NULL,FALSE);
}

void CMainFrame::OnUpdateRenderingmodePoint(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->CheckRenderingModePointCloud());
}


void CMainFrame::OnViewAligningerror() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateViewAligningError();
  InvalidateRect(NULL,FALSE);
}

void CMainFrame::OnUpdateViewAligningerror(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->CheckViewAligningError());
}


void CMainFrame::OnViewCurrentscan() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateViewOriginalSetting();
  InvalidateRect(NULL,FALSE);
}


void CMainFrame::OnUpdateViewCurrentscan(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->CheckViewOriginalSetting());
}

void CMainFrame::OnViewTexture32932() {
  // TODO: Add your command handler code here
  m_pVGView->m_scene.GetDisplay()->UpdateViewTexture();
  InvalidateRect(NULL,FALSE);
}


void CMainFrame::OnUpdateViewTexture32932(CCmdUI *pCmdUI) {
  // TODO: Add your command update UI handler code here
  pCmdUI->SetCheck(m_pVGView->m_scene.GetDisplay()->CheckViewTexture());
}

void CMainFrame::OnFileLoadcamera() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
    CString("VIEW Files(*.)|*.view|All data formats(*.*)|*.*|"), NULL); 

  FILE *file_ptr = NULL;

  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      CString filename = dlgFile.GetNextPathName(ps);
	  CT2CA filen(filename);
      fopen_s(&file_ptr, filen, "r");
    }
  }

  CameraContainer *camera = m_pVGView->m_scene.GetDisplay()->GetCamera();
  LightContainer *lc = m_pVGView->m_scene.GetDisplay()->GetLight();

  if (file_ptr != NULL) {
    fscanf_s(file_ptr, "%lf %lf %lf\n",
      &camera->eye_point_pos[0],
      &camera->eye_point_pos[1],
      &camera->eye_point_pos[2]);

    fscanf_s(file_ptr, "%lf %lf %lf\n",
      &camera->front_direc_vec[0],
      &camera->front_direc_vec[1],
      &camera->front_direc_vec[2]);

    fscanf_s(file_ptr, "%lf %lf %lf\n",
      &camera->gaze_point_pos[0],
      &camera->gaze_point_pos[1],
      &camera->gaze_point_pos[2]);

    fscanf_s(file_ptr, "%lf %lf %lf\n",
      &camera->up_direc_vec[0],
      &camera->up_direc_vec[1],
      &camera->up_direc_vec[2]);

    fscanf_s(file_ptr, "%lf\n",
      &camera->view_distance);

    int num_lights;
    fscanf_s(file_ptr, "%ld\n", &num_lights);
    lc->lights.resize(num_lights);
    for (int l_id = 0; l_id < num_lights; l_id++) {
      Light3Dim &light = lc->lights[l_id];
      fscanf_s(file_ptr, "%f %f %f %f\n",
        &light.ambient[0],
        &light.ambient[1],
        &light.ambient[2],
        &light.ambient[3]);
      fscanf_s(file_ptr, "%f %f %f %f\n",
        &light.diffuse[0],
        &light.diffuse[1],
        &light.diffuse[2],
        &light.diffuse[3]);
      fscanf_s(file_ptr, "%f %f %f %f\n",
        &light.position[0],
        &light.position[1],
        &light.position[2],
        &light.position[3]);
      fscanf_s(file_ptr, "%f %f %f %f\n",
        &light.specular[0],
        &light.specular[1],
        &light.specular[2],
        &light.specular[3]);
    }

    fclose(file_ptr);
  }
  m_pVGView->Invalidate(FALSE);
}


void CMainFrame::OnFileSavecamera() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
    CString("VIEW Files(*.)|*.view|All data formats(*.*)|*.*|"), NULL); 

  FILE *file_ptr = NULL;

  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      CString filename = dlgFile.GetNextPathName(ps);
	  CT2CA filen(filename);
      fopen_s(&file_ptr, filen, "w");
    }
  }

  CameraContainer *camera = m_pVGView->m_scene.GetDisplay()->GetCamera();
  LightContainer *lc = m_pVGView->m_scene.GetDisplay()->GetLight();

  if (file_ptr != NULL) {
    fprintf_s(file_ptr, "%.20f %.20f %.20f\n",
      camera->eye_point_pos[0],
      camera->eye_point_pos[1],
      camera->eye_point_pos[2]);

    fprintf_s(file_ptr, "%.20f %.20f %.20f\n",
      camera->front_direc_vec[0],
      camera->front_direc_vec[1],
      camera->front_direc_vec[2]);

    fprintf_s(file_ptr, "%.20f %.20f %.20f\n",
      camera->gaze_point_pos[0],
      camera->gaze_point_pos[1],
      camera->gaze_point_pos[2]);

    fprintf_s(file_ptr, "%.20f %.20f %.20f\n",
      camera->up_direc_vec[0],
      camera->up_direc_vec[1],
      camera->up_direc_vec[2]);

    fprintf_s(file_ptr, "%.20f\n",
      camera->view_distance);

    int num_lights = static_cast<int> (lc->lights.size());
    fprintf_s(file_ptr, "%ld\n", num_lights);
    for (int l_id = 0; l_id < num_lights; l_id++) {
      Light3Dim &light = lc->lights[l_id];
      fprintf_s(file_ptr, "%f %f %f %f\n",
        light.ambient[0],
        light.ambient[1],
        light.ambient[2],
        light.ambient[3]);
      fprintf_s(file_ptr, "%f %f %f %f\n",
        light.diffuse[0],
        light.diffuse[1],
        light.diffuse[2],
        light.diffuse[3]);
      fprintf_s(file_ptr, "%f %f %f %f\n",
        light.position[0],
        light.position[1],
        light.position[2],
        light.position[3]);
      fprintf_s(file_ptr, "%f %f %f %f\n",
        light.specular[0],
        light.specular[1],
        light.specular[2],
        light.specular[3]);
    }
    fclose(file_ptr);
  }
}

