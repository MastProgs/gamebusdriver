// Interface.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "MapTool.h"
#include "Interface.h"
#include "afxdialogex.h"


// CInterface ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CInterface, CDialog)

CInterface::CInterface(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_INTERFACE, pParent)
{

}

CInterface::~CInterface()
{
}

void CInterface::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInterface, CDialog)
END_MESSAGE_MAP()


// CInterface �޽��� ó�����Դϴ�.


BOOL CInterface::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		pMsg->wParam = NULL;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
