#pragma once
#include"stdafx.h"
#include"../Server_Code/ClientClass.h"
#include<fstream>
#include "../../../../server/2017 server/serverBoostModel/serverBoostModel/readLUAScript.h"

AsynchronousClientClass::AsynchronousClientClass()
{
	m_bAttackFirst = false;
}

AsynchronousClientClass::~AsynchronousClientClass()
{
	closesocket(m_sock);
	WSACleanup();
}

void AsynchronousClientClass::Init(const HWND& hwnd)
{
	// Server IP �Է°� DB Login �� �ϴ� �˾� Dialog
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, AboutDlgProc);
	//inputServerIP();

	// init Winsock
	int number_error = WSAStartup(MAKEWORD(2, 2), &m_wsadata);
	if (number_error != 0) {
		int err_no = WSAGetLastError();
		error_quit(L"WSAStartup ERROR", err_no);
	}

	m_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == m_sock) {
		int err_no = WSAGetLastError();
		error_quit(L"WSASocket()", err_no);
	}

	// connect
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(m_serverIP);
	serveraddr.sin_port = htons(SERVERPORT);

	m_retval = WSAConnect(m_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == m_retval) {
		// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			int err_no = WSAGetLastError();
			error_quit(L"WSAConnect()", err_no);
		}
	}

	// DB �����ϱ� ���� �Լ�
	Login_access();
	printf("\nLogin SUCCESSED\n");

	// WSAAsyncSelect - �ͺ��ŷ ���� �ڵ� ��ȯ
	m_hWnd = hwnd;

	if (NULL == m_hWnd) {
#ifdef _DEBUG
		printf("Cannot find Consol Window, ERROR : %d\n", __LINE__);
#endif
		exit(-1);
	}
	m_retval = WSAAsyncSelect(m_sock, m_hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
	if (SOCKET_ERROR == m_retval) {
		int err_no = WSAGetLastError();
		error_quit(L"ioctlsocket()", err_no);
	}

	m_wsa_recvbuf.buf = reinterpret_cast<CHAR*>(m_recvbuf);
	m_wsa_recvbuf.len = MAX_BUF_SIZE;
	m_wsa_sendbuf.buf = reinterpret_cast<CHAR*>(m_sendbuf);
	m_wsa_sendbuf.len = MAX_BUF_SIZE;

	LuaScript script("script/AI_default_status.lua");

	MAX_AI_SLIME = script.get<int>("ai_status_slime2.howMany");
	MAX_AI_GOBLIN = script.get<int>("ai_status_goblin2.howMany");
	MAX_AI_BOSS = script.get<int>("ai_status_boss.howMany");
	MAX_AI_NUM = MAX_AI_BOSS;

#ifdef _DEBUG
	//system("cls");
	printf("Connected with SERVER\n");
#endif
}

void AsynchronousClientClass::Login_access() {
#if 0
	int login_cnt{ 2 };

	while (--login_cnt)
	{
		printf("input ID & Password ( ex : guest guest )\n");
		wchar_t id[MAX_BUF_SIZE / 4]{ 0 };
		wchar_t pw[MAX_BUF_SIZE / 4]{ 0 };

		wcin >> id >> pw;

		Packet temp_buf[MAX_BUF_SIZE]{ 0 };
		temp_buf[0] = wcslen(L"guest") * 2;
		wcscpy(reinterpret_cast<wchar_t*>(&temp_buf[1]), L"guest");
		temp_buf[temp_buf[0] + 3] = wcslen(L"guest") * 2;
		wcscpy(reinterpret_cast<wchar_t*>(&temp_buf[temp_buf[0] + 4]), L"guest");

		send(m_sock, reinterpret_cast<char*>(&temp_buf), MAX_BUF_SIZE, 0);
		recv(m_sock, reinterpret_cast<char*>(&temp_buf), MAX_BUF_SIZE, 0);

		if (1 == temp_buf[0]) { return; }
		else {
			system("cls");
			printf("Login Failed\n");
			exit(-1);
		}
	}

#else
	Packet temp_buf[MAX_BUF_SIZE]{ 0 };
	temp_buf[0] = wcslen(login_id) * 2;
	wcscpy(reinterpret_cast<wchar_t*>(&temp_buf[1]), login_id);
	temp_buf[temp_buf[0] + 3] = wcslen(login_pw) * 2;
	wcscpy(reinterpret_cast<wchar_t*>(&temp_buf[temp_buf[0] + 4]), login_pw);

	send(m_sock, reinterpret_cast<char*>(&temp_buf), MAX_BUF_SIZE, 0);
	recv(m_sock, reinterpret_cast<char*>(&temp_buf), MAX_BUF_SIZE, 0);

	if (1 == temp_buf[0]) 
	{ 
		return; 
	}
	else {
		system("cls");
		printf("Login Failed\n");
	}
#endif
}

void AsynchronousClientClass::inputServerIP_ReadtxtFile()
{
	ifstream getServerIP;
	getServerIP.open("ServerIP.txt", ios::in);
	getServerIP.getline(m_serverIP, 32);
	getServerIP.close();
}

void AsynchronousClientClass::inputServerIP_cin()
{
	cout << "\nexample 127.0.0.1\nInput Server's IP : ";
	cin >> m_serverIP;
}

void AsynchronousClientClass::inputServerIP()
{
	//#ifdef _DEBUG
	//	cout << "choose Server to connect.\n\n0. Read Server IP in .txt File\n1. Input Server IP, directly\n\nchoice method : ";
	//	short cmd{ 0 };
	//	cin >> cmd;
	//
	//	if (cmd & 1) { inputServerIP_cin(); }
	//	else { inputServerIP_ReadtxtFile(); }
	//
	//	cout << "\n\n\t--==** Connecting Server, Please Wait **==--\n\n\n";
	//#else
	inputServerIP_ReadtxtFile();
	//#endif
}

SOCKET* AsynchronousClientClass::getServerSocket()
{
	return &m_sock;
}

int AsynchronousClientClass::recvn()
{
	int received;
	Packet *ptr = m_recvbuf;
	int left = MAX_BUF_SIZE;

	while (left > 0) {
		received = recv(m_sock, reinterpret_cast<char*>(&ptr), left, 0);
		if (SOCKET_ERROR == received)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}

	return (MAX_BUF_SIZE - left);
}

void AsynchronousClientClass::error_display(char *msg, int err_no, int line)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
#ifdef _DEBUG
	printf("[ %s - %d ]", msg, line);
	wprintf(L"���� %s\n", lpMsgBuf);
#endif
	LocalFree(lpMsgBuf);
}

void AsynchronousClientClass::error_quit(wchar_t *msg, int err_no)
{
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// sendPacket ( sizeof( data ), type �� , data �� �ּҰ� );
void AsynchronousClientClass::sendPacket(const BYTE data_size, const BYTE type, BYTE* data_start_pointer) {
	// ���� �ִ� ���� ������ ���� ������ ���̰� Ŀ���� �ȵȴ�.
	if (MAX_BUF_SIZE < (data_size + 2)) {
#ifdef _DEBUG
		// �Ʒ��� ���� ������ �߻��ϰ� �ȴٸ�, ���� ����� �ǵ帮�� ���� ���� ������ ũ�⸦ ������ �ٿ� ���� �� �ֵ��� ����
		printf("[ code LINE %d ] [ code FUNCTION %s ] SendPacket class ERROR :: data size overed MAX_BUF_SIZE\n", __LINE__, __FUNCTION__);
#endif
	}
	else {
		// ��Ŷ ���� ���� ���� ����
		m_sendbuf[0] = data_size + 2;
		m_sendbuf[1] = type;

		if (nullptr != data_start_pointer) { memcpy(&m_sendbuf[2], data_start_pointer, m_sendbuf[0]); }

		m_wsa_sendbuf.len = m_sendbuf[0];
		DWORD ioByteSize;
		m_retval = WSASend(m_sock, &m_wsa_sendbuf, 1, &ioByteSize, 0, NULL, NULL);
		if (SOCKET_ERROR == m_retval) {
			// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				int err_no = WSAGetLastError();
				error_quit(L"sendPacket()", err_no);
			}
		}
	}
}

// ������ ����ִ���, ������ �ϴ����� ���� �⺻ �׽�Ʈ �Լ�. 3 Bytes �� ������, �⺻ Ÿ���� TEST, �����ʹ� 1�� ������.
void AsynchronousClientClass::sendPacket_TEST() {
	m_sendbuf[0] = 3;
	m_sendbuf[1] = TEST;
	m_sendbuf[2] = 1;		// true

	m_wsa_sendbuf.len = m_sendbuf[0];

	DWORD ioByteSize;
	m_retval = WSASend(m_sock, &m_wsa_sendbuf, 1, &ioByteSize, 0, NULL, NULL);
	//int retval = send(sock, reinterpret_cast<char*>(&sendbuf), sendbuf[0], 0);
	if (SOCKET_ERROR == m_retval) {
		// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			int err_no = WSAGetLastError();
			error_quit(L"sendPacket_TEST()", err_no);
		}
	}
}