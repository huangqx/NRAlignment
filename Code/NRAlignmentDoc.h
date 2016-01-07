// NRAlignmentDoc.h : interface of the CNRAlignmentDoc class
//


#pragma once

#include <afxwin.h>

class DataContainer;
class CNRAlignmentDoc : public CDocument {
protected: // create from serialization only
	CNRAlignmentDoc();
	DECLARE_DYNCREATE(CNRAlignmentDoc)

// Attributes
public:
  DataContainer *data_pt;
// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CNRAlignmentDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnFileLoadbinarydata();
  afx_msg void OnFileSavebinarydata();
  afx_msg void OnFileLoadscans();
  afx_msg void OnFileSavereferencemodel();
  afx_msg void OnFileSaveregisteredscans();
  afx_msg void OnFileAddscans();
};


