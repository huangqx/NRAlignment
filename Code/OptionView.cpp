// OptionView.cpp : implementation file
//

#include <math.h>
#include <iostream>

#include "NRAlignmentDoc.h"
#include "NRAlignmentView.h"
#include "OptionView.h"
#include "MainFrm.h"
#include "data_container.h"
#include "patch_generator.h"
#include "init_opt_struct.h"
#include "gauss_newton_optimizer.h"
#include "pairwise_rigid_alignment.h"
#include "multile_rigid_alignment.h"
#include "pairwise_non_rigid_align.h"
#include "target_point_generator.h"

using namespace std;


// COptionView

IMPLEMENT_DYNCREATE(COptionView, CFormView)

COptionView::COptionView()
  : CFormView(COptionView::IDD) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  // Default parameters for rigid alignment
  rigid_weight_color_ = 4.0;
  rigid_initial_average_corr_distance_ = 0.25;
  rigid_final_average_corr_distance_ = 0.09;
  rigid_weight_point_2_plane_distance_ = 0.75;
  rigid_num_of_samples_ = 2048;
  rigid_num_of_levels_ = 4;
  rigid_num_of_iterations_ = 8;
  loop_closure_ = false;

  // Default parameters for pair-wise non-rigid alignment
  pw_non_rigid_weight_point_2_plane_distance_ = 0.75;
  pw_non_rigid_initial_average_corr_distance_ = 0.08;
  pw_non_rigid_final_average_corr_distance_ = 0.01;
  pw_non_rigid_weight_color_ = 0.08;
  pw_non_rigid_weight_normal_ = 0.16;
  pw_non_rigid_patch_size_ = 0.02;
  pw_non_rigid_num_of_samples_ = 8192;
  pw_non_rigid_initial_regu_weight_ = 2.0;
  pw_non_rigid_final_regu_weight_ = 1.0/2.0;
  pw_non_rigid_num_of_levels_ = 4;
  pw_non_rigid_num_of_iterations_ = 8;

  // Default parameters for non-rigid alignment
  non_rigid_initial_grid_res_ = 3.2e-2;
  non_rigid_final_grid_res_ = 4e-3;
  non_rigid_num_of_levels_ = 4;
  non_rigid_num_of_iterations_ = 5;
  initial_smoothness_weight_ = 1.0;
  final_smoothness_weight_ = 1.0/8.0;
  patch_size_ratio_ = 1.5;
  min_patch_size_ = 0.01;
  patch_support_region_size_ = 2.5;

  non_rigid_weight_color_ = 1.0;
  ref_surf_cluster_size_ = 1.4;
  ref_surf_half_window_width_ = 1;
  non_rigid_weight_point_2_plane_distance_ = 0.9;

  memory_efficient_ = false;
}

COptionView::~COptionView() {
}

void COptionView::DoDataExchange(CDataExchange* pDX) {
  CFormView::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_TREE1, m_ctrlTree);
  DDX_Control(pDX, IDC_EDIT_rigid_weight_color, m_edit_rigid_weight_color);
  DDX_Control(pDX, IDC_EDIT_rigid_initial_average_corr_distance, m_edit_rigid_initial_average_corr_distance);
  DDX_Control(pDX, IDC_EDIT_rigid_FINAL_AVERAGE_CORR_DISTANCE, m_edit_rigid_final_average_corr_distance);
  DDX_Control(pDX, IDC_EDIT_rigid_WEIGHT_POINT_2_PLANE_DISTANCE, m_edit_rigid_weight_point_2_plane_distance);
  DDX_Control(pDX, IDC_EDIT_rigid_NUM_OF_SAMPLES, m_edit_rigid_num_of_samples);
  DDX_Control(pDX, IDC_EDIT_rigid_NUM_LEVELS, m_edit_rigid_num_of_levels);
  DDX_Control(pDX, IDC_EDIT_rigid_NUM_OF_ITERATIONS, m_edit_rigid_num_of_iterations);
  DDX_Control(pDX, IDC_CHECK_LOOP_CLOSURE, m_check_loop_closure);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_INITIAL_GRID_RES, m_edit_non_rigid_initial_grid_res);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_FINAL_GRID_RES, m_edit_non_rigid_final_grid_res);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_NUMBER_GRID_LEVELS, m_edit_non_rigid_num_grid_levels);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_RELATIVE_PATCH_SIZE, m_edit_non_rigid_patch_size);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_WEIGHT_POINT_2_PLANE_DISTANCE, m_edit_non_rigid_weight_point_2_plane_distance);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_INITIAL_REGU_WEIGHT, m_edit_non_rigid_initial_regu_weight);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_FINAL_REGU_WEIGHT, m_edit_non_rigid_final_regu_weight);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_NUM_OF_ITERATIONS, m_edit_non_rigid_num_of_iterations);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_NEIGHBORHOOD_WIDTH, m_edit_ref_surf_half_window_width);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_WEIGHT_COLOR, m_edit_ref_surf_weight_color);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_CLUSTER_SIZE, m_edit_ref_surf_cluster_size);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_rigid_WEIGHT_POINT_2_PLANE_DISTANCE, m_edit_pw_non_rigid_weight_point_2_plane_distance);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_rigid_initial_average_corr_distance, m_edit_pw_non_rigid_initial_average_corr_distance);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_rigid_FINAL_AVERAGE_CORR_DISTANCE, m_edit_pw_non_rigid_final_average_corr_distance);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_RIGID_RELATIVE_PATCH_SIZE, m_edit_pw_non_rigid_patch_size);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_rigid_weight_color, m_edit_pw_non_rigid_weight_color);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_RIGID_INITIAL_REGU_WEIGHT, m_edit_pw_non_rigid_initial_regu_weight);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_RIGID_FINAL_REGU_WEIGHT, m_edit_pw_non_rigid_final_regu_weight);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_rigid_NUM_LEVELS, m_edit_pw_non_rigid_num_of_levels);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_rigid_NUM_OF_ITERATIONS, m_edit_pw_non_rigid_num_of_iterations);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_rigid_NUM_OF_SAMPLES, m_edit_pw_non_rigid_num_of_samples);
  DDX_Control(pDX, IDC_EDIT_PAIRWISE_NON_RIGID_WEIGHT_NORMAL, m_edit_pw_non_rigid_weight_normal);
  DDX_Control(pDX, IDC_CHECK_MEMORY_EFFICIENT, m_check_memory_efficient);
  DDX_Control(pDX, IDC_EDIT_NON_RIGID_PATCH_SUPPORT_REGION_SIZE, m_edit_non_rigid_patch_support_region_size);
}

BEGIN_MESSAGE_MAP(COptionView, CFormView)
	ON_WM_CREATE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE1, OnTvnSelchangedTree1)
	ON_WM_VSCROLL()
  ON_WM_HSCROLL()
  ON_BN_CLICKED(IDC_BUTTON_PERFORM_RIGID_ALIGNMENT, &COptionView::OnBnClickedButtonPerformRigidAlignment)
  ON_BN_CLICKED(IDC_BUTTON_PERFORM_NON_RIGID_ALIGNMENT, &COptionView::OnBnClickedButtonPerformNonRigidAlignment)
  ON_BN_CLICKED(IDC_BUTTON_PERFORM_PAIRWISE_ALIGNMENT, &COptionView::OnBnClickedButtonPerformPairwiseAlignment)
END_MESSAGE_MAP()


// COptionView diagnostics

#ifdef _DEBUG
void COptionView::AssertValid() const {
	CFormView::AssertValid();
}

void COptionView::Dump(CDumpContext& dc) const {
	CFormView::Dump(dc);
}
#endif //_DEBUG


// COptionView message handlers
void COptionView::OnInitialUpdate() {
	CNRAlignmentDoc	*pDoc = 
		((CMainFrame*)(AfxGetApp()->m_pMainWnd))->m_pVGView->GetDocument();
	 CFormView::OnInitialUpdate();
	 SetIcon(m_hIcon, TRUE);			// Set big icon
	 SetIcon(m_hIcon, FALSE);		// Set small icon

	 // Without an image list we won't get a drag image...
  VERIFY(m_listTreeImages.Create(16, 16, ILC_COLOR8|ILC_MASK, 1, 1));
	 m_listTreeImages.Add(AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	 m_listTreeImages.Add(AfxGetApp()->LoadIcon(IDI_ICON1));
  m_ctrlTree.SetImageList(&m_listTreeImages, TVSIL_NORMAL);

  root_shapes = m_ctrlTree.InsertItem(CString("Shapes"), 0, 0);
  root_reference_model = m_ctrlTree.InsertItem(CString("RefSurf"), 0, 0);
  

  UpdateUI();
}

void COptionView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
  // TODO: Add your message handler code here and/or call default
  CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
  ((CMainFrame *)GetParentFrame())->m_pVGView->Invalidate(true);
}


void	COptionView::UpdateUI() {
  // Parameters for rigid alignments
  wchar_t str[256];
  memset(str, 0, 256);

  swprintf_s(str, L"%.1f", rigid_weight_color_);
  m_edit_rigid_weight_color.SetWindowText(str);
  swprintf_s(str, L"%.3f", rigid_initial_average_corr_distance_);
  m_edit_rigid_initial_average_corr_distance.SetWindowText(str);
  swprintf_s(str, L"%.3f", rigid_final_average_corr_distance_);
  m_edit_rigid_final_average_corr_distance.SetWindowText(str);
  swprintf_s(str, L"%.2f", rigid_weight_point_2_plane_distance_);
  m_edit_rigid_weight_point_2_plane_distance.SetWindowText(str);
  swprintf_s(str, L"%d", rigid_num_of_samples_);
  m_edit_rigid_num_of_samples.SetWindowText(str);
  swprintf_s(str, L"%d", rigid_num_of_levels_);
  m_edit_rigid_num_of_levels.SetWindowText(str);
  swprintf_s(str, L"%d", rigid_num_of_iterations_);
  m_edit_rigid_num_of_iterations.SetWindowText(str);
  m_check_loop_closure.SetCheck(loop_closure_);
  
  // Parameters for pair-wise non-rigid alignments
  swprintf_s(str, L"%.2f", pw_non_rigid_weight_point_2_plane_distance_);
  m_edit_pw_non_rigid_weight_point_2_plane_distance.SetWindowText(str);

  swprintf_s(str, L"%.2f", pw_non_rigid_initial_average_corr_distance_);
  m_edit_pw_non_rigid_initial_average_corr_distance.SetWindowText(str);
  swprintf_s(str, L"%.2f", pw_non_rigid_final_average_corr_distance_);
  m_edit_pw_non_rigid_final_average_corr_distance.SetWindowText(str);

  swprintf_s(str, L"%.2f", pw_non_rigid_weight_color_);
  m_edit_pw_non_rigid_weight_color.SetWindowText(str);
  swprintf_s(str, L"%.2f", pw_non_rigid_weight_normal_);
  m_edit_pw_non_rigid_weight_normal.SetWindowText(str);

  swprintf_s(str, L"%.2f", pw_non_rigid_patch_size_);
  m_edit_pw_non_rigid_patch_size.SetWindowText(str);
  swprintf_s(str, L"%ld", pw_non_rigid_num_of_samples_);
  m_edit_pw_non_rigid_num_of_samples.SetWindowText(str);
  swprintf_s(str, L"%.2f", pw_non_rigid_initial_regu_weight_);
  m_edit_pw_non_rigid_initial_regu_weight.SetWindowText(str);
  swprintf_s(str, L"%.2f", pw_non_rigid_final_regu_weight_);
  m_edit_pw_non_rigid_final_regu_weight.SetWindowText(str);
  swprintf_s(str, L"%ld", pw_non_rigid_num_of_levels_);
  m_edit_pw_non_rigid_num_of_levels.SetWindowText(str);
  swprintf_s(str, L"%ld", pw_non_rigid_num_of_iterations_);
  m_edit_pw_non_rigid_num_of_iterations.SetWindowText(str);

  // Parameters for non-rigid alignments
  swprintf_s(str, L"%.3f", non_rigid_initial_grid_res_);
  m_edit_non_rigid_initial_grid_res.SetWindowText(str);
  swprintf_s(str, L"%.3f", non_rigid_final_grid_res_);
  m_edit_non_rigid_final_grid_res.SetWindowText(str);
  swprintf_s(str, L"%d ", non_rigid_num_of_levels_);
  m_edit_non_rigid_num_grid_levels.SetWindowText(str);
  swprintf_s(str, L"%.1f ", patch_size_ratio_);
  m_edit_non_rigid_patch_size.SetWindowText(str);
  swprintf_s(str, L"%.1f ", patch_support_region_size_);
  m_edit_non_rigid_patch_support_region_size.SetWindowText(str);

  swprintf_s(str, L"%.2f ", non_rigid_weight_point_2_plane_distance_);
  m_edit_non_rigid_weight_point_2_plane_distance.SetWindowText(str);
  swprintf_s(str, L"%.1f ", initial_smoothness_weight_);
  m_edit_non_rigid_initial_regu_weight.SetWindowText(str);
  swprintf_s(str, L"%.2f ", final_smoothness_weight_);
  m_edit_non_rigid_final_regu_weight.SetWindowText(str);
  swprintf_s(str, L"%d ", non_rigid_num_of_iterations_);
  m_edit_non_rigid_num_of_iterations.SetWindowText(str);
  swprintf_s(str, L"%d ", ref_surf_half_window_width_);
  m_edit_ref_surf_half_window_width.SetWindowText(str);
  swprintf_s(str, L"%.1f ", non_rigid_weight_color_);
  m_edit_ref_surf_weight_color.SetWindowText(str);
  swprintf_s(str, L"%.1f ", ref_surf_cluster_size_);
  m_edit_ref_surf_cluster_size.SetWindowText(str);

  m_check_memory_efficient.SetCheck(memory_efficient_);
}

int COptionView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CFormView::OnCreate(lpCreateStruct) == -1)
		return -1;
	// TODO:  Add your specialized creation code here
	return 0;
}

void COptionView::OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult) {
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void COptionView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	// TODO: Add your message handler code here and/or call default
	if(nSBCode == SB_ENDSCROLL)
		return;

	CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}

void COptionView::GetUserInputParameters() {
  CString	str;
  // Parameters for rigid alignment
  m_edit_rigid_weight_color.GetWindowText(str);
  rigid_weight_color_ = min(20.0, _wtof(str));
  m_edit_rigid_initial_average_corr_distance.GetWindowText(str);
  rigid_initial_average_corr_distance_ = min(0.25, _wtof(str));
  m_edit_rigid_final_average_corr_distance.GetWindowText(str);
  rigid_final_average_corr_distance_ = max(0.004, _wtof(str));
  m_edit_rigid_weight_point_2_plane_distance.GetWindowText(str);
  rigid_weight_point_2_plane_distance_ = max(0, min(1, _wtof(str)));
  m_edit_rigid_num_of_samples.GetWindowText(str);
  rigid_num_of_samples_ = _wtoi(str);
  m_edit_rigid_num_of_levels.GetWindowText(str);
  rigid_num_of_levels_ = _wtoi(str);
  m_edit_rigid_num_of_iterations.GetWindowText(str);
  rigid_num_of_iterations_ = _wtoi(str);
  loop_closure_ = m_check_loop_closure.GetCheck() == 1;

  // Parameters for pair-wise non-rigid alignment
  m_edit_pw_non_rigid_weight_point_2_plane_distance.GetWindowText(str);
  pw_non_rigid_weight_point_2_plane_distance_ =
    max(0.0, min(1.0, _wtof(str)));  
  m_edit_pw_non_rigid_initial_average_corr_distance.GetWindowText(str);
  pw_non_rigid_initial_average_corr_distance_ = min(0.128, _wtof(str));
  m_edit_pw_non_rigid_final_average_corr_distance.GetWindowText(str);
  pw_non_rigid_final_average_corr_distance_ = max(0.008, _wtof(str));
  m_edit_pw_non_rigid_weight_color.GetWindowText(str);
  pw_non_rigid_weight_color_ = max(0.0, min(1.0, _wtof(str)));
  m_edit_pw_non_rigid_weight_normal.GetWindowText(str);
  pw_non_rigid_weight_normal_ = max(0.0, min(1.0, _wtof(str)));
  m_edit_pw_non_rigid_patch_size.GetWindowText(str);
  pw_non_rigid_patch_size_ = max(0.02, min(0.08, _wtof(str)));
  m_edit_pw_non_rigid_initial_regu_weight.GetWindowText(str);
  pw_non_rigid_initial_regu_weight_ = max(1.0, min(10.0, _wtof(str)));
  m_edit_pw_non_rigid_final_regu_weight.GetWindowText(str);
  pw_non_rigid_final_regu_weight_ = max(0.1, min(1.0, _wtof(str)));
  m_edit_pw_non_rigid_num_of_levels.GetWindowText(str);
  pw_non_rigid_num_of_levels_ = _wtoi(str);
  m_edit_pw_non_rigid_num_of_iterations.GetWindowText(str);
  pw_non_rigid_num_of_iterations_ = _wtoi(str);

  // Parameters for non-rigid alignment
  m_edit_non_rigid_initial_grid_res.GetWindowText(str);
  non_rigid_initial_grid_res_ = max(0.016, min(0.064, _wtof(str)));
  m_edit_non_rigid_final_grid_res.GetWindowText(str);
  non_rigid_final_grid_res_ = max(0.002, min(0.008, _wtof(str)));
  m_edit_non_rigid_num_grid_levels.GetWindowText(str);
  non_rigid_num_of_levels_ = max(2, min(5, _wtoi(str)));
  m_edit_non_rigid_patch_size.GetWindowText(str);
  patch_size_ratio_ = max(1.5, min(3.0, _wtof(str)));
  m_edit_non_rigid_weight_point_2_plane_distance.GetWindowText(str);
  non_rigid_weight_point_2_plane_distance_ = max(0.0, min(1.0, _wtof(str)));
  m_edit_non_rigid_initial_regu_weight.GetWindowText(str);
  initial_smoothness_weight_ = max(1.0, min(10.0, _wtof(str)));
  m_edit_non_rigid_final_regu_weight.GetWindowText(str);
  final_smoothness_weight_ = max(0.1, min(1.0, _wtof(str)));
  m_edit_non_rigid_num_of_iterations.GetWindowText(str);
  non_rigid_num_of_iterations_ = max(5, min(10, _wtoi(str)));
  m_edit_ref_surf_half_window_width.GetWindowText(str);
  ref_surf_half_window_width_ = max(1, min(2, _wtoi(str)));
  m_edit_ref_surf_weight_color.GetWindowText(str);
  non_rigid_weight_color_ = max(0.0, min(4.0, _wtof(str)));
  m_edit_ref_surf_cluster_size.GetWindowText(str);
  ref_surf_cluster_size_ = max(0.5, min(2.0, _wtof(str)));
  m_edit_non_rigid_patch_support_region_size.GetWindowText(str);
  patch_support_region_size_ = max(2.0, min(3.0, _wtof(str)));

  memory_efficient_ = m_check_memory_efficient.GetCheck() == 1;
  UpdateUI();
}

void COptionView::OnBnClickedButtonPerformRigidAlignment() {
  // TODO: Add your control notification handler code here
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame *)GetParentFrame())->m_pVGView->GetDocument();

  if (pDoc->data_pt->GetInputShapes()->size() < 2)
    return;

  // Get parameters
  GetUserInputParameters();
  PairwiseRigidAligPara para;
  para.weightColor = rigid_weight_color_;
  para.initialAverageCorresDis = rigid_initial_average_corr_distance_;
  para.finalAverageCorresDis = rigid_final_average_corr_distance_;
  para.weightPoint2PlaneDis = rigid_weight_point_2_plane_distance_;
  para.numSamples = rigid_num_of_samples_;
  para.numLevels = rigid_num_of_levels_;
  para.numIterations = rigid_num_of_iterations_;

  // Perform pair-wise alignment between adjacent scans
  vector<Affine3d> adjRigidTransforms;
  if (loop_closure_)
    adjRigidTransforms.resize(pDoc->data_pt->GetInputShapes()->size());
  else
    adjRigidTransforms.resize(pDoc->data_pt->GetInputShapes()->size()-1);

  PairwiseRigidAlign rigid_reg;
  for (unsigned fixedSurfId = 0;
    fixedSurfId < pDoc->data_pt->GetInputShapes()->size();
    ++fixedSurfId) {
    unsigned movingSurfId = fixedSurfId + 1;
    if (movingSurfId == pDoc->data_pt->GetInputShapes()->size()) {
      if (!loop_closure_)
        break;
      movingSurfId = 0;
    }
    printf("Aligning scan %d with scan %d...\n", movingSurfId, fixedSurfId);
    rigid_reg.Compute(
      (*pDoc->data_pt->GetInputShapes())[fixedSurfId].surface,
      (*pDoc->data_pt->GetInputShapes())[movingSurfId].surface,
      para,
      &adjRigidTransforms[fixedSurfId]);  
  }

  // Global bundle adjustment
  MultipleRigidAlign multiple_rigid_reg;
  if (loop_closure_) { //Perform loop closure
    printf("Performing loop closure...\n");
    multiple_rigid_reg.LoopClosure(&adjRigidTransforms);
  }

  printf("Transforming scans...\n");
  multiple_rigid_reg.ApplyOptimizedRigidPoses(
    adjRigidTransforms,
    pDoc->data_pt->GetInputShapes());
}

void COptionView::OnBnClickedButtonPerformNonRigidAlignment() {
  // TODO: Add your control notification handler code here
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame *)GetParentFrame())->m_pVGView->GetDocument();

  vector<ShapeContainer> *shapes = pDoc->data_pt->GetInputShapes();
  if (shapes->size() < 2)
    return;

  // All size parameters are related to the size of the input dataset
  double sceneSize =
    sqrt(pDoc->data_pt->GetBoundingBox()->size.getSqrNorm());

  GetUserInputParameters();
  for (int level_id = 0; level_id < non_rigid_num_of_levels_; ++level_id) {
    double t = level_id/(non_rigid_num_of_levels_-1.0);
    double gridRes = exp((1-t)*log(non_rigid_initial_grid_res_)
      + t*log(non_rigid_final_grid_res_));
    printf("OuterLoop: iteration = %d, gridRes = %f\n",
      level_id, gridRes);

    if (!memory_efficient_) {
      /************************************************************************/
      // Generate patches for each scan, each patch is
      // associated with a rigid transform
      /************************************************************************/

      // patches can be two small!
      double patchSize = max(min_patch_size_, patch_size_ratio_*gridRes)*sceneSize;

      PatchGenerator patch_generator;
      InitOptStruct opt_struct_generator;
      vector<PatchContainer> patch_containers;
      vector<OptStructContainer> opt_struct_containers;
      patch_containers.resize(shapes->size());
      opt_struct_containers.resize(shapes->size());

      for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
        printf("  Computing patches of shape %d...\n", shape_id);
        patch_generator.Compute((*shapes)[shape_id].surface,
          patchSize,
          patch_support_region_size_,
          &patch_containers[shape_id]);
        ShapeContainer &sc = (*shapes)[shape_id];
        // Pre-compute the smoothness term of the objective function
        opt_struct_generator.SmoothnessTerm(sc.surface, patch_containers[shape_id],
          &opt_struct_containers[shape_id]);
      }

      for (int iteration = 0; iteration < non_rigid_num_of_iterations_;
        ++iteration) {
        printf("  inner iteration %d\n", iteration);

        // Compute the current poses
        for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
          (*shapes)[shape_id].GenerateCurrentSurface(
            patch_containers[shape_id], false);
        }

        // Compute the reference surface from current poses.
        // Setup up the optimization problem which aims at
        // minimizing the distances of these scans to the reference surface
        printf(" Computing the reference surface...\n");
        // Set parameters
        RefSurfPara para;
        para.gridRes = gridRes*sceneSize;
        para.clusterSize = ref_surf_cluster_size_;
        para.halfWindowWidth = ref_surf_half_window_width_;
        para.weightColor = non_rigid_weight_color_;
        para.weightPointPlaneDis = non_rigid_weight_point_2_plane_distance_;

        /************************************************************************/
        // Generate the reference surface
        // Store the target point indices of each scan point
        /************************************************************************/
        vector<vector<int>> foot_point_indices;
        TargetPointGenerator tp_generator;
        tp_generator.Compute(*pDoc->data_pt->GetInputShapes(),
          para,
          pDoc->data_pt->GetReferenceModel()->GetVertexArray(),
          &foot_point_indices);
        pDoc->data_pt->GetReferenceModel()->ComputeBoundingBox();

        // Compute the data term for each scan
        for (unsigned shape_id = 0;
          shape_id < pDoc->data_pt->GetInputShapes()->size();
          ++shape_id) {
            opt_struct_generator.DataTerm(
              (*pDoc->data_pt->GetInputShapes())[shape_id],
              patch_containers[shape_id],
              *pDoc->data_pt->GetReferenceModel(),
              foot_point_indices[shape_id],
              para.weightPointPlaneDis,
              &opt_struct_containers[shape_id]);
        }

        t = iteration/(non_rigid_num_of_iterations_-1.0);
        double smoothness_weight = exp((1-t)*log(initial_smoothness_weight_)
          + t*log(final_smoothness_weight_));

        // Solve the optimization problem for each scan
        // which minimizes the objective function that combines a data term(just computed)
        // and the smoothness term (which is pre-computed)
        GaussNewtonOptimizer gauss_newton_optimizer;
        gauss_newton_optimizer.SetSmoothnessWeight(smoothness_weight);
        for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
          ShapeContainer &shape = (*shapes)[shape_id];
          if (shape.surface.Locked())
            continue;

          vector<Affine3d> poses;
          poses.resize(patch_containers[shape_id].surface_patches.size());
          for (unsigned i = 0; i < patch_containers[shape_id].surface_patches.size(); ++i)
            poses[i] = patch_containers[shape_id].surface_patches[i].motion;

          gauss_newton_optimizer.Compute(opt_struct_containers[shape_id], &poses);
          for (unsigned i = 0; i < patch_containers[shape_id].surface_patches.size(); ++i)
            patch_containers[shape_id].surface_patches[i].motion = poses[i];
        }
      }
      for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
        ShapeContainer &shape = (*shapes)[shape_id];
        shape.GenerateCurrentSurface(patch_containers[shape_id], true);
      }
    } else {
      double patchSize = max(min_patch_size_,
        patch_size_ratio_*gridRes)*sceneSize;

      PatchGenerator patch_generator;
      InitOptStruct opt_struct_generator;

      for (int iteration = 0; iteration < non_rigid_num_of_iterations_;
        ++iteration) {
        printf("  inner iteration %d\n", iteration);
        
        // Compute the reference surface from current poses.
        // Setup up the optimization problem which aims at
        // minimizing the distances of these scans to the reference surface
        printf(" Computing the reference surface...\n");
        // Set parameters
        RefSurfPara para;
        para.gridRes = gridRes*sceneSize;
        para.clusterSize = ref_surf_cluster_size_;
        para.halfWindowWidth = ref_surf_half_window_width_;
        para.weightColor = non_rigid_weight_color_;
        para.weightPointPlaneDis = non_rigid_weight_point_2_plane_distance_;

        /************************************************************************/
        // Generate the reference surface
        // Store the target point indices of each scan point
        /************************************************************************/
        vector<vector<int>> foot_point_indices;
        TargetPointGenerator tp_generator;
        tp_generator.Compute(*pDoc->data_pt->GetInputShapes(),
          para,
          pDoc->data_pt->GetReferenceModel()->GetVertexArray(),
          &foot_point_indices);
        pDoc->data_pt->GetReferenceModel()->ComputeBoundingBox();

        t = iteration/(non_rigid_num_of_iterations_-1.0);
        double smoothness_weight = exp((1-t)*log(initial_smoothness_weight_)
          + t*log(final_smoothness_weight_));

        GaussNewtonOptimizer gauss_newton_optimizer;
        gauss_newton_optimizer.SetSmoothnessWeight(smoothness_weight);

        // Align the scans with this reference surface
        for (unsigned shape_id = 0;
          shape_id < pDoc->data_pt->GetInputShapes()->size();
          ++shape_id) {
          ShapeContainer &shape = (*shapes)[shape_id];
          if (shape.surface.Locked())
            continue;

          PatchContainer patch_container;
          OptStructContainer opt_struct_container;
          printf("  Computing patches of shape %d...\n", shape_id);
          patch_generator.Compute((*shapes)[shape_id].surface,
            patchSize,
            patch_support_region_size_,
            &patch_container);
          ShapeContainer &sc = (*shapes)[shape_id];
          // Pre-compute the smoothness term of the objective function
          opt_struct_generator.SmoothnessTerm(sc.surface, patch_container,
            &opt_struct_container);

          opt_struct_generator.DataTerm(
            (*pDoc->data_pt->GetInputShapes())[shape_id],
            patch_container,
            *pDoc->data_pt->GetReferenceModel(),
            foot_point_indices[shape_id],
            para.weightPointPlaneDis,
            &opt_struct_container);

          vector<Affine3d> poses;
          poses.resize(patch_container.surface_patches.size());
          for (unsigned i = 0; i < patch_container.surface_patches.size(); ++i)
            poses[i] = patch_container.surface_patches[i].motion;

          gauss_newton_optimizer.Compute(opt_struct_container, &poses);
          for (unsigned i = 0; i < patch_container.surface_patches.size(); ++i)
            patch_container.surface_patches[i].motion = poses[i];
          shape.GenerateCurrentSurface(patch_container, true);
        }
      }
    }
  }
}

void COptionView::OnBnClickedButtonPerformPairwiseAlignment() {
  // TODO: Add your control notification handler code here
  CNRAlignmentDoc* pDoc = 
    ((CMainFrame *)GetParentFrame())->m_pVGView->GetDocument();

  vector<ShapeContainer> *shapes = pDoc->data_pt->GetInputShapes();
  if (shapes->size() < 2)
    return;

  // All size parameters are related to the size of the input dataset
  double sceneSize =
    sqrt(pDoc->data_pt->GetBoundingBox()->size.getSqrNorm());


  // Get fixed points from locked scans
  unsigned num_points = 0;
  for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
    const ShapeContainer &shape = (*shapes)[shape_id];
    if (shape.surface.Locked()) {
      num_points += shape.surface.GetVertexArray()->size();
    }
  }

  vector<Vertex> fixed_points;
  fixed_points.resize(num_points);

  num_points = 0;
  for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
    const ShapeContainer &shape = (*shapes)[shape_id];
    if (shape.surface.Locked()) {
      for (unsigned v_id = 0; v_id < shape.surface.GetVertexArray()->size(); ++v_id) {
        fixed_points[num_points] =
          (*shape.surface.GetVertexArray())[v_id];
        num_points++;
      }
    }
  }

  if (num_points == 0) {
    printf("You have to fix at least one scan!\n");
    return;
  }

  PWNonRigidAlign pw_non_rigid;
  pw_non_rigid.SetFixedScan(fixed_points);
  
  // Set parameters
  PWNonRigidAlignPara pw_non_rigid_para;
  GetUserInputParameters();
  pw_non_rigid_para.finalAverageCorresDis =
    pw_non_rigid_final_average_corr_distance_;
  pw_non_rigid_para.initialAverageCorresDis =
    pw_non_rigid_initial_average_corr_distance_;
  pw_non_rigid_para.finalSmoothnessWeight =
    pw_non_rigid_final_regu_weight_;
  pw_non_rigid_para.initialSmoothnessWeight =
    pw_non_rigid_initial_regu_weight_;
  pw_non_rigid_para.numIterations =
    pw_non_rigid_num_of_iterations_;
  pw_non_rigid_para.numLevels =
    pw_non_rigid_num_of_levels_;
  pw_non_rigid_para.numSamples =
    pw_non_rigid_num_of_samples_;
  pw_non_rigid_para.patchSize =
    pw_non_rigid_patch_size_;
  pw_non_rigid_para.weightColor =
    pw_non_rigid_weight_color_;
  pw_non_rigid_para.weightNormal =
    pw_non_rigid_weight_normal_;
  pw_non_rigid_para.weightPoint2PlaneDis =
    pw_non_rigid_weight_point_2_plane_distance_;

  // Perform alignment per each scan
  for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
    ShapeContainer &shape = (*shapes)[shape_id];
    if (!shape.surface.Locked() && shape.surface.Render())
      pw_non_rigid.Compute(pw_non_rigid_para, &shape);
  }
}
