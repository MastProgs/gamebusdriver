// Client.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//



#include "stdafx.h"
#include "Client.h"
#include "MainApp.h"
#include "Include.h"
//#include <vld.h>

// ���� ����:
HINSTANCE g_hInst;                                // ���� �ν��Ͻ��Դϴ�.

HWND	g_hWnd;
DWORD	g_dwLightIndex = 0;
D3DXVECTOR3 g_vLightDir = D3DXVECTOR3(1.f, -1.f, 1.f);
AsynchronousClientClass g_client;

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// ������ Ŭ���� ���
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = L"��縷��";
	if (false == RegisterClass(&wndclass)) { return -1; }

	// ������ ����
	RECT rc = { 0,0,WINCX, WINCY };
	g_hInst = hInstance; // �ν��Ͻ� �ڵ��� ���� ������ �����մϴ�.
	HWND hWnd = CreateWindow(L"��縷��", L"��縷��", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	g_hWnd = hWnd;
	if (NULL == hWnd) { return -1; }
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// �̰� ������ ������ release ��忡�� �������� Ȯ���� �ſ� ����.
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);


	HACCEL hAccelTable = LoadAccelerators(hInstance, 0);

	MSG msg;
	msg.message = WM_NULL;

	CMainApp*		pMainApp = CMainApp::Create();


	// �̰� �ʰ� �Űܾߵ�, ���� ���ƿ��� ���ķ�
	// g_hWnd �� ������ â �ڵ鰪�̴ϱ�
	// �ϱ� ������ �Ѵ� �۷ι��̴� ��� ���ڴ�
	// ��ư �ű� �� �θ��� �� �̰�, ����� �����ϰ� ����?

	// �⺻ �޽��� �����Դϴ�.
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		else
		{
			pMainApp->Update();
			pMainApp->Render();
		}
	}

	::Safe_Delete(pMainApp);

	return (int)msg.wParam;
}

BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam) {

	switch (iMessage)
	{
	case WM_INITDIALOG: {

		SetDlgItemText(hDlg, IDC_IP, L"127.0.0.1");
		SetDlgItemText(hDlg, IDC_ID, L"guest");
		SetDlgItemText(hDlg, IDC_PW, L"guest");

		return TRUE;
	}
	case WM_COMMAND: {

		switch (LOWORD(wParam))
		{
		case IDOK:
			wchar_t ip[32];

			GetDlgItemText(hDlg, IDC_IP, ip, 32);
			WideCharToMultiByte(CP_ACP, 0, ip, -1, g_client.get_server_ip_array(), 32, 0, 0);

			GetDlgItemText(hDlg, IDC_ID, g_client.get_login_id_array(), MAX_BUF_SIZE / 4);
			GetDlgItemText(hDlg, IDC_PW, g_client.get_login_pw_array(), MAX_BUF_SIZE / 4);

			EndDialog(hDlg, IDOK);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			PostQuitMessage(0);
			return TRUE;
		default:
			break;
		}

		break;
	}
	default:
		break;
	}
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{

	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ hdc�� ����ϴ� �׸��� �ڵ带 �߰��մϴ�.
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SOCKET:
		g_client.ProcessWinMessage(hWnd, message, wParam, lParam);
		return 0;
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}