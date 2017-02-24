
// MapToolView.cpp : CMapToolView Ŭ������ ����
//

#include "stdafx.h"
// SHARED_HANDLERS�� �̸� ����, ����� �׸� �� �˻� ���� ó���⸦ �����ϴ� ATL ������Ʈ���� ������ �� ������
// �ش� ������Ʈ�� ���� �ڵ带 �����ϵ��� �� �ݴϴ�.
#ifndef SHARED_HANDLERS
#include "MapTool.h"
#endif

#include "MapToolDoc.h"
#include "MapToolView.h"
#include "Device.h"
#include "Include.h"
#include "MainFrm.h"
#include "TimeMgr.h"
#include "Camera.h"
#include "ResourcesMgr.h"
#include "ShaderMgr.h"
#include "Input.h"
#include "Scene.h"
#include "SceneMgr.h"
#include "Back.h"
#include "RenderMgr.h"
#include "ObjMgr.h"
#include "ParsingDevice9.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMapToolView

IMPLEMENT_DYNCREATE(CMapToolView, CView)

BEGIN_MESSAGE_MAP(CMapToolView, CView)
	// ǥ�� �μ� ����Դϴ�.
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// CMapToolView ����/�Ҹ�

CMapToolView::CMapToolView()
{
	// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.

}

CMapToolView::~CMapToolView()
{
}

BOOL CMapToolView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs�� �����Ͽ� ���⿡��
	//  Window Ŭ���� �Ǵ� ��Ÿ���� �����մϴ�.

	return CView::PreCreateWindow(cs);
}

// CMapToolView �׸���

void CMapToolView::OnDraw(CDC* /*pDC*/)
{
	CMapToolDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: ���⿡ ���� �����Ϳ� ���� �׸��� �ڵ带 �߰��մϴ�.
}


// CMapToolView �μ�

BOOL CMapToolView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// �⺻���� �غ�
	return DoPreparePrinting(pInfo);
}

void CMapToolView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ��ϱ� ���� �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
}

void CMapToolView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: �μ� �� ���� �۾��� �߰��մϴ�.
}


// CMapToolView ����

#ifdef _DEBUG
void CMapToolView::AssertValid() const
{
	CView::AssertValid();
}

void CMapToolView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

void CMapToolView::SetCurToolIndex(int& iIndex)
{
	m_iCurToolIndex = iIndex;
}

CMapToolDoc* CMapToolView::GetDocument() const // ����׵��� ���� ������ �ζ������� �����˴ϴ�.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMapToolDoc)));
	return (CMapToolDoc*)m_pDocument;
}
#endif //_DEBUG


// CMapToolView �޽��� ó����


void CMapToolView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CMainFrame*	pFrame = (CMainFrame*)AfxGetMainWnd();

	RECT	rcWindow;
	pFrame->GetWindowRect(&rcWindow);

	SetRect(&rcWindow, 0, 0, rcWindow.right - rcWindow.left,
		rcWindow.bottom - rcWindow.top);

	RECT rcMainView;
	GetClientRect(&rcMainView);

	float fRowFrm = float(rcWindow.right - rcMainView.right);
	float fColFrm = float(rcWindow.bottom - rcMainView.bottom);


	pFrame->SetWindowPos(NULL, 0, 0,
		int(WINCX + fRowFrm),
		int(WINCY + fColFrm), SWP_NOZORDER);

	g_hWnd = m_hWnd;

	CDevice::GetInstance()->CreateDevice();

	CParsingDevice9::GetInstance()->InitGraphicDev(CParsingDevice9::MODE_WIN, g_hWnd, WINCX, WINCY);


	CTimeMgr::GetInstance()->InitTime();

	if (FAILED(CCamera::GetInstance()->Initialize()))
	{
		AfxMessageBox(L"Camera Initialize Failed", MB_OK);
	}



	if (FAILED(CResourcesMgr::GetInstance()->ReserveContainerSize(RESOURCE_END)))
	{
		AfxMessageBox(L"Resource Container Reserve Failed", MB_OK);
		return;
	}

	//�Ϲ�

	if (CShaderMgr::GetInstance()->AddShaderFiles(L"VS", L"Shader.fx", "VS", "vs_5_0", SHADER_VS))
	{
		AfxMessageBox(L"Vertex Shader(Default) Create Failed", MB_OK);
		return;
	}


	if (CShaderMgr::GetInstance()->AddShaderFiles(L"PS", L"Shader.fx", "PS", "ps_5_0", SHADER_PS))
	{
		AfxMessageBox(L"PIXEL Shader(Default) Create Failed", MB_OK);
		return;
	}
	//���̳��� �Ž���
	if (CShaderMgr::GetInstance()->AddShaderFiles(L"VS_ANI", L"Shader.fx", "VS_ANI", "vs_5_0", SHADER_ANI))
	{
		AfxMessageBox(L"Vertex Shader(Ani) Create Failed", MB_OK);
		return;
	}

	//�׺�޽� ���ο�

	/*if (CShaderMgr::GetInstance()->AddShaderFiles(L"VS_LINE", L"LineShader.fx", "VS_LINE", "vs_5_0", SHADER_LINE_VS))
	{
		AfxMessageBox(L"Vertex Shader(Line) Create Failed", MB_OK);
		return;
	}


	if (CShaderMgr::GetInstance()->AddShaderFiles(L"PS_LINE", L"LineShader.fx", "PS_LINE", "ps_5_0", SHADER_LINE_PS))
	{
		AfxMessageBox(L"PIXEL Shader(Line) Create Failed", MB_OK);
		return;
	}*/

	if (CInput::GetInstance()->InitInputDevice(AfxGetInstanceHandle(), g_hWnd))
	{
		AfxMessageBox(L"DInput Create Failed", MB_OK);
		return;
	}

	CScene* pScene = NULL;
	pScene = CBack::Create();
	CSceneMgr::GetInstance()->AddScene(SCENE_BACK, pScene);

	CSceneMgr::GetInstance()->ChangeScene(SCENE_BACK);



	SetTimer(1, 10, NULL);

	m_iCurToolIndex = 0;
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
}


void CMapToolView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	CView::OnTimer(nIDEvent);

	CTimeMgr::GetInstance()->SetTime();
	CInput::GetInstance()->SetInputState();
	CCamera::GetInstance()->Update();

	CSceneMgr::GetInstance()->Update();


	CDevice::GetInstance()->BeginDevice();
	CSceneMgr::GetInstance()->Render();
	CDevice::GetInstance()->EndDevice();
}


void CMapToolView::PostNcDestroy()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	CDevice::GetInstance()->DestroyInstance();
	CParsingDevice9::GetInstance()->DestroyInstance();
	CTimeMgr::GetInstance()->DestroyInstance();
	CCamera::GetInstance()->DestroyInstance();
	CShaderMgr::GetInstance()->DestroyInstance();
	CInput::GetInstance()->DestroyInstance();
	CRenderMgr::GetInstance()->DestroyInstance();
	CObjMgr::GetInstance()->DestroyInstance();
	CSceneMgr::GetInstance()->DestroyInstance();

	CView::PostNcDestroy();
}


void CMapToolView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/o�Ǵ� �⺻���� ȣ���մϴ�.

	switch (m_iCurToolIndex)
	{
	case 0:
		((CBack*)CSceneMgr::GetInstance()->GetScene())->ConstObjectMode();
		break;
	case 1:
		break;
	case 2:
		break;
	}

	CView::OnLButtonDown(nFlags, point);
}
