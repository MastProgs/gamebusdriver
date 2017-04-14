#pragma once
#include "afxwin.h"


// CNaviTool ��ȭ �����Դϴ�.

class CNaviCell;
class CNaviTool : public CDialog
{
	DECLARE_DYNAMIC(CNaviTool)

public:
	CNaviTool(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CNaviTool();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_NAVITOOL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	bool m_bStart;
	CNaviCell* m_pCurCell;
public:
	void SetPickPos(D3DXVECTOR3 vIndex);
	void SetPickCell(CNaviCell* pCell,int iIndex);
public:
	int m_iNaviCnt;
	CButton m_WireFrame;
	CButton m_ObjectWire;
	CButton m_NaviMode[4];
	CButton m_CreateMode[2];
	CButton m_PickingMode[2];
	CButton m_CellType[2];
	CButton m_DeleteMode[2];
	float m_fPosX;
	float m_fPosY;
	float m_fPosZ;
	float m_fNearRange;
	float m_PointAx;
	float m_PointBx;
	float m_PointCx;
	float m_PointAy;
	float m_PointBy;
	float m_PointCy;
	float m_PointAz;
	float m_PointBz;
	float m_PointCz;
	
	CButton m_NaviView;
	afx_msg void OnNaviSave();
	afx_msg void OnNaviLoad();
	afx_msg void OnObjWireCheck();
	int m_iCellNum;
};
