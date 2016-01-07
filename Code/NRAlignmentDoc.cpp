// NRAlignmentDoc.cpp : implementation of the CNRAlignmentDoc class
//

#include "stdafx.h"
#include "NRAlignment.h"

#include "NRAlignmentDoc.h"
#include "dynamic_linear_algebra_templcode.h"
#include "dynamic_linear_algebra.h"
#include "data_container.h"
#include "scan_io.h"
#include "MainFrm.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CNRAlignmentDoc

IMPLEMENT_DYNCREATE(CNRAlignmentDoc, CDocument)

BEGIN_MESSAGE_MAP(CNRAlignmentDoc, CDocument)
  ON_COMMAND(ID_FILE_LOADBINARYDATA, &CNRAlignmentDoc::OnFileLoadbinarydata)
  ON_COMMAND(ID_FILE_SAVEBINARYDATA, &CNRAlignmentDoc::OnFileSavebinarydata)
  ON_COMMAND(ID_FILE_LOADSCANS, &CNRAlignmentDoc::OnFileLoadscans)
  ON_COMMAND(ID_FILE_SAVEREFERENCEMODEL, &CNRAlignmentDoc::OnFileSavereferencemodel)
  ON_COMMAND(ID_FILE_SAVEREGISTEREDSCANS, &CNRAlignmentDoc::OnFileSaveregisteredscans)
  ON_COMMAND(ID_FILE_ADDSCANS, &CNRAlignmentDoc::OnFileAddscans)
END_MESSAGE_MAP()


// CNRAlignmentDoc construction/destruction

CNRAlignmentDoc::CNRAlignmentDoc() {
	// TODO: add one-time construction code here
  data_pt = new DataContainer();
}

CNRAlignmentDoc::~CNRAlignmentDoc() {
  delete data_pt;
}

BOOL CNRAlignmentDoc::OnNewDocument() {
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// CNRAlignmentDoc serialization

void CNRAlignmentDoc::Serialize(CArchive& ar) {
	if (ar.IsStoring()) {
		// TODO: add storing code here
	} else {
		// TODO: add loading code here
	}
}

// CNRAlignmentDoc diagnostics

#ifdef _DEBUG
void CNRAlignmentDoc::AssertValid() const {
	CDocument::AssertValid();
}

void CNRAlignmentDoc::Dump(CDumpContext& dc) const {
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CNRAlignmentDoc commands

void CNRAlignmentDoc::OnFileLoadbinarydata() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
    CString("DATA Files(*.)|*.data|All data formats(*.*)|*.*|"), NULL); 

  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      CString filename = dlgFile.GetNextPathName(ps);
      FILE *file_ptr;
	  CT2CA filen(filename);
      fopen_s(&file_ptr, filen, "rb");
      data_pt->Read(file_ptr);
      COptionView* pView = ((CMainFrame*)AfxGetApp()->m_pMainWnd)->m_pOptView;
      data_pt->SetNames(pView);
      fclose(file_ptr);
      data_pt->first_view_ = true;
    }
  }
  UpdateAllViews(NULL);
}

void CNRAlignmentDoc::OnFileSavebinarydata() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
    CString("DATA Files(*.)|*.data|All data formats(*.*)|*.*|"), NULL); 

  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      CString filename = dlgFile.GetNextPathName(ps);
      FILE *file_ptr;
	  CT2CA filen(filename);
      fopen_s(&file_ptr, filen, "wb");
      data_pt->Write(file_ptr);
      fclose(file_ptr);
    }
  }
}

void CNRAlignmentDoc::OnFileLoadscans() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ALLOWMULTISELECT, 
    CString("PLY Files(*.)|*.ply|OBJ Files(*.)|*.obj|All data formats(*.*)|*.*|"), NULL); 

  const DWORD numberOfFileNames = 1000;
  const DWORD fileNameMaxLength = MAX_PATH + 1;
  const DWORD bufferSize = (numberOfFileNames * fileNameMaxLength) + 1;
  TCHAR* filenamesBuffer = new TCHAR[bufferSize];

  // Initialize beginning and end of buffer.
  filenamesBuffer[0] = NULL;
  filenamesBuffer[bufferSize-1] = NULL;

  // Attach buffer to OPENFILENAME member.
  dlgFile.m_ofn.lpstrFile = filenamesBuffer;
  dlgFile.m_ofn.nMaxFile = bufferSize;


  vector<wstring> input_shape_filenames_ply, input_shape_filenames_obj;
  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      CString filename = dlgFile.GetNextPathName(ps);
      if(filename.Right(4) == ".ply") {
        wstring filename_wstring = filename.GetBuffer();
        input_shape_filenames_ply.push_back(filename_wstring);
      }
      if(filename.Right(4) == ".obj") {
        wstring filename_wstring = filename.GetBuffer();
        input_shape_filenames_obj.push_back(filename_wstring);
      }
    }
  }
  delete []filenamesBuffer;

  if (input_shape_filenames_ply.size() > 0) {
    sort(input_shape_filenames_ply.begin(), input_shape_filenames_ply.end());
    vector<ShapeContainer> *shapes = data_pt->GetInputShapes();
    shapes->resize(input_shape_filenames_ply.size());

    for (unsigned shape_id = 0;
      shape_id < input_shape_filenames_ply.size();
      ++shape_id) {
      Scan_IO scan_io;
      scan_io.LoadScanFromPLY(input_shape_filenames_ply[shape_id],
        &(*shapes)[shape_id].surface);
    }
    COptionView* pView = ((CMainFrame*)AfxGetApp()->m_pMainWnd)->m_pOptView;
    data_pt->SetNames(pView);
    data_pt->Normalize();
    data_pt->first_view_ = true;
    UpdateAllViews(NULL);
  }

  if (input_shape_filenames_obj.size() > 0) {
    sort(input_shape_filenames_obj.begin(), input_shape_filenames_obj.end());
    vector<ShapeContainer> *shapes = data_pt->GetInputShapes();
    shapes->resize(input_shape_filenames_obj.size());

    for (unsigned shape_id = 0;
      shape_id < input_shape_filenames_obj.size();
      ++shape_id) {
        Scan_IO scan_io;
        scan_io.LoadScanFromOBJ(input_shape_filenames_obj[shape_id],
          &(*shapes)[shape_id].surface);
    }
    COptionView* pView = ((CMainFrame*)AfxGetApp()->m_pMainWnd)->m_pOptView;
    data_pt->SetNames(pView);
    data_pt->Normalize();
    data_pt->first_view_ = true;
    UpdateAllViews(NULL);
  }
}

void CNRAlignmentDoc::OnFileAddscans() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT |OFN_ALLOWMULTISELECT, 
    CString("PLY Files(*.)|*.ply|All data formats(*.*)|*.*|"), NULL); 

  const DWORD numberOfFileNames = 1000;
  const DWORD fileNameMaxLength = MAX_PATH + 1;
  const DWORD bufferSize = (numberOfFileNames * fileNameMaxLength) + 1;
  TCHAR* filenamesBuffer = new TCHAR[bufferSize];

  // Initialize beginning and end of buffer.
  filenamesBuffer[0] = NULL;
  filenamesBuffer[bufferSize-1] = NULL;

  // Attach buffer to OPENFILENAME member.
  dlgFile.m_ofn.lpstrFile = filenamesBuffer;
  dlgFile.m_ofn.nMaxFile = bufferSize;


  vector<wstring> input_shape_filenames;
  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      CString filename = dlgFile.GetNextPathName(ps);
      if(filename.Right(4) == ".ply") {
        wstring filename_wstring = filename.GetBuffer();
        input_shape_filenames.push_back(filename_wstring);
      }
    }
  }
  delete []filenamesBuffer;

  Scan_IO scan_io;
  if (input_shape_filenames.size() > 0) {
    sort(input_shape_filenames.begin(), input_shape_filenames.end());
    vector<ShapeContainer> *shapes = data_pt->GetInputShapes();
    unsigned num_of_old_shapes = shapes->size();
    shapes->resize(num_of_old_shapes + input_shape_filenames.size());

    unsigned valid_shape_id = 0;
    for (unsigned shape_id = 0;
      shape_id < input_shape_filenames.size();
      ++shape_id) {
      bool exist = false;
      wstring filename;
      scan_io.ExtractFileName(input_shape_filenames[shape_id], filename);
      for (int i = 0; i < int(num_of_old_shapes + shape_id-1); ++i) {
        if ((*shapes)[i].surface.GetFileName() 
          == filename) {
          exist = true;
          break;
        }
      }
      if (exist)
        continue;
      scan_io.LoadScanFromPLY(input_shape_filenames[shape_id],
        &(*shapes)[num_of_old_shapes+valid_shape_id].surface);
      (*shapes)[num_of_old_shapes+valid_shape_id].ApplyGlobalTransform(
        data_pt->GetGlobalTranslation(),
        data_pt->GetGlobalScaling());
      valid_shape_id++;
    }
    shapes->resize(num_of_old_shapes + valid_shape_id);
    COptionView* pView = ((CMainFrame*)AfxGetApp()->m_pMainWnd)->m_pOptView;
    data_pt->SetNames(pView);
    data_pt->ComputeBoundingBox();
    data_pt->first_view_ = true;
    UpdateAllViews(NULL);
  }
}

void CNRAlignmentDoc::OnFileSavereferencemodel() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
    CString("pwn Files(*.)|*.pwn|All data formats(*.*)|*.*|"), NULL); 

  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      CString filename = dlgFile.GetNextPathName(ps);
      wstring filename_wstring = filename.GetBuffer();
      Scan_IO scan_io;
      scan_io.SaveReferenceModel(*data_pt->GetReferenceModel(),
        data_pt->GetGlobalTranslation(),
        data_pt->GetGlobalScaling(),
        filename_wstring);
    }
  }
}

void CNRAlignmentDoc::OnFileSaveregisteredscans() {
  // TODO: Add your command handler code here
  CFileDialog dlgFile(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
    CString("PLY Files(*.)|*.ply|OBJ Files(*.)|*.obj|All data formats(*.*)|*.*|"), NULL); 

  CString foldername;
  if (dlgFile.DoModal() == IDOK) {
    POSITION ps = dlgFile.GetStartPosition();
    while (ps) {
      foldername = dlgFile.GetNextPathName(ps);
    }
  }

  bool save_as_ply = foldername.Right(4) == ".ply";
  for (unsigned pos = foldername.GetLength()-1; pos >= 0; --pos) {
    if (foldername.GetAt(pos) == '\\') {
      foldername = foldername.Left(pos);
      break;
    }
  }

  vector<ShapeContainer> *shapes = data_pt->GetInputShapes();
  for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
    const ShapeContainer &shape = (*shapes)[shape_id];
    CString filename(shape.surface.GetFileName().c_str());
    if (save_as_ply) {
      CString filepath = foldername + L"\\" + filename + L"_aligned.ply";
      wstring wstring_filepath = filepath.GetBuffer();
      Scan_IO scan_io;
      scan_io.SaveScanToPLY((*shapes)[shape_id].surface,
        data_pt->GetGlobalTranslation(),
        data_pt->GetGlobalScaling(),
        wstring_filepath);
    } else {
      CString filepath = foldername + L"\\" + filename + L"_aligned.obj";
      wstring wstring_filepath = filepath.GetBuffer();
      Scan_IO scan_io;
      scan_io.SaveScanToOBJ((*shapes)[shape_id].surface,
        data_pt->GetGlobalTranslation(),
        data_pt->GetGlobalScaling(),
        wstring_filepath);
    }
  }
}
