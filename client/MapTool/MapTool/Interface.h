#pragma once


// CInterface ��ȭ �����Դϴ�.

class CInterface : public CDialog
{
	DECLARE_DYNAMIC(CInterface)

public:
	CInterface(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CInterface();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INTERFACE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
