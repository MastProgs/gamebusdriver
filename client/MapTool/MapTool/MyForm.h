#pragma once
#include "afxcmn.h"
#include "ObjectTool.h"
#include "Interface.h"
#include "NaviTool.h"


// CMyForm �� ���Դϴ�.
class CMapToolView;
class CMyForm : public CFormView
{
	DECLARE_DYNCREATE(CMyForm)

protected:
	CMyForm();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CMyForm();

public:
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MYFORM };
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CTabCtrl m_Tab;
	CWnd*			m_pWndShow;
	CMapToolView*	m_pView;
	CObjectTool		m_Tab1;
	CNaviTool		m_Tab2;
	CInterface		m_InterFace;
	CNaviTool		m_NaviTool;
	afx_msg void OnToolTabChange(NMHDR *pNMHDR, LRESULT *pResult);
	virtual void OnInitialUpdate();
};


