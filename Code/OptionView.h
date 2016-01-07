#ifndef OptionView_h
#define OptionView_h

#include "resource.h"

#pragma once
#include "afxcmn.h"
#include "edittreectrlex.h"
#include "afxwin.h"

#define ID_TREECTRL	2001

// COptionView form view
class COptionView : public CFormView {
	DECLARE_DYNCREATE(COptionView)

protected:
	COptionView();           // protected constructor used by dynamic creation
	virtual ~COptionView();
public:
  void UpdateUI();
public:
	enum { IDD = IDD_FORMVIEW };
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
  // Rigid alignment
  double rigid_weight_color_;
  double rigid_initial_average_corr_distance_;
  double rigid_final_average_corr_distance_;
  double rigid_weight_point_2_plane_distance_;
  int rigid_num_of_samples_;
  int rigid_num_of_levels_;
  int rigid_num_of_iterations_;
  bool loop_closure_;

  // Pair-wise Rigid Alignment
  double pw_non_rigid_weight_point_2_plane_distance_;
  double pw_non_rigid_initial_average_corr_distance_;
  double pw_non_rigid_final_average_corr_distance_;
  double pw_non_rigid_weight_color_;
  double pw_non_rigid_weight_normal_;
  double pw_non_rigid_patch_size_;// Relative to the bounding box of the shape
  int pw_non_rigid_num_of_samples_;
  double pw_non_rigid_initial_regu_weight_;
  double pw_non_rigid_final_regu_weight_;
  int pw_non_rigid_num_of_levels_;
  int pw_non_rigid_num_of_iterations_;


  // (Multiple) Non-Rigid alignment
  double non_rigid_initial_grid_res_;
  double non_rigid_final_grid_res_;
  int non_rigid_num_of_levels_;
  int non_rigid_num_of_iterations_;
  double initial_smoothness_weight_;
  double final_smoothness_weight_;
  double patch_size_ratio_; // [1,2] Relative to the current grid resolution
  double min_patch_size_; // relative to the length of bounding box diagonal
  double patch_support_region_size_; // relative to the size of each patch

  // Parameters used in generating the reference surface 
  double non_rigid_weight_color_; // Controls the contribution
  // of the color information when performing clustering
  double ref_surf_cluster_size_; // Controls the size of each point cluster
  // in the space of normal + color
  int ref_surf_half_window_width_; // Determines the size of the neighbor
  // when performing clustering, the bigger the window size, the smoother
  // the reference surface
  
  // The parameter that controls of the type of distance function being used
  double non_rigid_weight_point_2_plane_distance_;

  // Memory efficient
  bool memory_efficient_;

  void GetUserInputParameters();
 
public:

  CEditTreeCtrlEx m_ctrlTree;
  HICON	m_hIcon;
  CImageList m_listTreeImages;

  HTREEITEM root_shapes;
  HTREEITEM root_reference_model;

  virtual void OnInitialUpdate();
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnTvnSelchangedTree1(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

  afx_msg void OnBnClickedButtonPerformRigidAlignment();
  CEdit m_edit_rigid_weight_color;
  CEdit m_edit_rigid_initial_average_corr_distance;
  CEdit m_edit_rigid_final_average_corr_distance;
  CEdit m_edit_rigid_weight_point_2_plane_distance;
  CEdit m_edit_rigid_num_of_samples;
  CEdit m_edit_rigid_num_of_levels;
  CEdit m_edit_rigid_num_of_iterations;
  CButton m_check_loop_closure;
  afx_msg void OnBnClickedButtonPerformNonRigidAlignment();
  CEdit m_edit_non_rigid_initial_grid_res;
  CEdit m_edit_non_rigid_final_grid_res;
  CEdit m_edit_non_rigid_num_grid_levels;
  CEdit m_edit_non_rigid_patch_size;
  CEdit m_edit_non_rigid_weight_point_2_plane_distance;
  CEdit m_edit_non_rigid_initial_regu_weight;
  CEdit m_edit_non_rigid_final_regu_weight;
  CEdit m_edit_non_rigid_num_of_iterations;
  CEdit m_edit_ref_surf_half_window_width;
  CEdit m_edit_ref_surf_weight_color;
  CEdit m_edit_ref_surf_cluster_size;
  afx_msg void OnBnClickedButtonPerformPairwiseAlignment();
  CEdit m_edit_pw_non_rigid_weight_point_2_plane_distance;
  CEdit m_edit_pw_non_rigid_initial_average_corr_distance;
  CEdit m_edit_pw_non_rigid_final_average_corr_distance;
  CEdit m_edit_pw_non_rigid_weight_color;
  CEdit m_edit_pw_non_rigid_weight_normal;
  CEdit m_edit_pw_non_rigid_patch_size;
  CEdit m_edit_pw_non_rigid_num_of_samples;
  CEdit m_edit_pw_non_rigid_initial_regu_weight;
  CEdit m_edit_pw_non_rigid_final_regu_weight;
  CEdit m_edit_pw_non_rigid_num_of_levels;
  CEdit m_edit_pw_non_rigid_num_of_iterations;
  CButton m_check_memory_efficient;
  CEdit m_edit_non_rigid_patch_support_region_size;
};


#endif