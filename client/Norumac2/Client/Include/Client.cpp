// Client.cpp : ���� ���α׷��� ���� �������� �����մϴ�.
//



#include "stdafx.h"
#include "Client.h"
#include "MainApp.h"
#include "Include.h"
#include "ChatUI.h"
#include "ObjMgr.h"
#include "Font.h"
//#include <vld.h>

// ���� ����:
HINSTANCE g_hInst; // ���� �ν��Ͻ��Դϴ�.

// ClientClass.cpp - Init �Լ����� ��ũ��Ʈ ���� �о�´�.
int MAX_AI_NUM;
int MAX_AI_SLIME;
int MAX_AI_GOBLIN;
int MAX_AI_BOSS;

TCHAR cText[MAX_CHAT_SIZE / 2];//������ ä�� �޽���
TCHAR cCompText[10];//�������� ����
TCHAR cCanText[200];//Ư������
int iCNum = 0;//Ư������ ��ġ
int iCMax = 0;//Ư������ ��� �ִ밹��

HWND	g_hWnd;
DWORD	g_dwLightIndex = 0;
D3DXVECTOR3 g_vLightDir = D3DXVECTOR3(1.f, -1.f, 1.f);
AsynchronousClientClass g_client;
bool g_bLogin = false;
bool g_bChatMode = false; 
bool g_bChatEnd = true;
float g_fChatCool = 0.f;

// �� �ڵ� ��⿡ ��� �ִ� �Լ��� ������ �����Դϴ�.
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
int GetText(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_wsetlocale(LC_ALL, L"korean");

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

#ifdef _DEBUG
	// �̰� ������ ������ release ��忡�� �������� Ȯ���� �ſ� ����.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG



	HACCEL hAccelTable = LoadAccelerators(hInstance, 0);

	MSG msg;
	msg.message = WM_NULL;

	CMainApp*		pMainApp = CMainApp::Create();

	ZeroMemory(&cText, sizeof(char) * MAX_BUF_SIZE);
	ZeroMemory(&cCompText, sizeof(char) * 10);

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
			if (g_bChatMode == true)
			{
				if (_tcslen(cText) == 0)
				{
					g_bChatEnd = true;

				}
				else
				{
					g_bChatEnd = false;

				}
				CChatUI* pChatUI = dynamic_cast<CChatUI*>(*(CObjMgr::GetInstance()->Get_ObjList(L"ChatUI")->begin()));

				pChatUI->m_pFont->m_wstrText = cText;

				if (GetAsyncKeyState(VK_RETURN) && g_bChatEnd == false)
				{
					//���濡�� �޽����� ������ �κ�

					// ����ٰ� �����͸� ���� �� ����
					g_client.sendPacket(MAX_CHAT_SIZE / 2, CHAT, reinterpret_cast<Packet*>(&cText));

					ZeroMemory(&cText, sizeof(TCHAR) * (MAX_CHAT_SIZE / 2));
					//�������� ���ڿ��� ����ش�.
					g_fChatCool = 0.f;
				}
			}
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

	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	if (g_bChatMode == true)
	{
		if (GetText(hWnd, message, wParam, lParam) == 0)
			return 0;
	}


	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
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
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: ���⿡ �׸��� �ڵ带 �߰��մϴ�.
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SOCKET:
		g_client.ProcessWinMessage(hWnd, message, wParam, lParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int GetText(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int iLen;
	HIMC hIMC = NULL;//ime �ڵ�

	switch (msg)
	{
	case WM_IME_COMPOSITION://�ѱ� ����
		hIMC = ImmGetContext(hWnd);
		if (lParam & GCS_RESULTSTR)
		{
			if ((iLen = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0)) > 0)// ime�� ��Ʈ�� ���̸� �޾ƿ�
			{


				ImmGetCompositionString(hIMC, GCS_RESULTSTR, cCompText, iLen);
				//�������� ���ڿ��� �޾ƿ�.

				cCompText[iLen] = 0;
				//���� 0�� �־� ������.

				wcout << L"�������1:" << cCompText << endl;

				_tcscpy(cText + _tcslen(cText), cCompText);
				//��ü �޽��� �ڿ� �ٿ���.

				ZeroMemory(&cCompText, sizeof(char) * 10);
				//�������� ���ڿ� �ʱ�ȭ
			}
		}
		else if (lParam & GCS_COMPSTR) //�������̸�
		{

			iLen = ImmGetCompositionString(hIMC, GCS_COMPSTR, NULL, 0);
			// �������� ���̸� ��´�.

			ImmGetCompositionString(hIMC, GCS_COMPSTR, cCompText, iLen);
			//�������� ���ڸ� ��´�.

			cCompText[iLen] = 0;
			//���� ������

			wcout << L"�������2:" << *cCompText << endl;

		}
		ImmReleaseContext(hWnd, hIMC);
		return 0;
	case WM_CHAR://����� �齺���̽� ���
		if (wParam == CHAT_BACKSPACE)
		{
			if (_tcslen(cText) > 0) // ������ ������
			{
				if (_tcslen(cText) < 0)//����� Ȯ���ڵ��̸�
				{
					cText[_tcslen(cText) - 1] = 0; //���ڸ� ����
				}

				cText[_tcslen(cText) - 1] = 0;//���ڸ� �����

				ZeroMemory(&cCompText, sizeof(char) * 10); // �������� ���� �ʱ�ȭ
			}
		}
		else if (wParam == CHAT_ENTER)
		{
			//���Ϳ���ó��
		}
		else
		{
			iLen = _tcslen(cText);
			cText[iLen] = wParam & 0xff; // �Ѿ�� ���ڸ� �ֱ�
			//cText[iLen] = 0;//���� ������
		}
		return 0;
	case WM_IME_NOTIFY://����
					   //GetCandi(hWnd, wParam, lParam, 0);
		return 0;
		//case WM_KEYDOWN://Ű�ٿ�
		//return 0;
	default:
		break;
	}
}

//void GetCandi(HWND hWnd, WPARAM wParam, LPARAM lParam, int Number)
//{
//	//Ư���� ���ڴ� ���߿�
//}

