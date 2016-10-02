#include"stdafx.h"

AsynchronousClientClass::AsynchronousClientClass()
{

}

AsynchronousClientClass::~AsynchronousClientClass()
{
	closesocket(sock);
	WSACleanup();
}

void AsynchronousClientClass::Init()
{
	ConnectingServer();

	// init Winsock
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		int err_no = WSAGetLastError();
		error_quit(L"WSAStartup ERROR", err_no);
	}

	sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	if (INVALID_SOCKET == sock) {
		int err_no = WSAGetLastError();
		error_quit(L"socket()", err_no);
	}
		
	// �ͺ��ŷ ���� ��ȯ
	/*DWORD on = 1;
	retval = ioctlsocket(sock, FIONBIO, &on);
	if (SOCKET_ERROR == retval) {
		int err_no = WSAGetLastError();
		error_quit(L"ioctlsocket()", err_no);
	}*/

	// connect
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(serverIP);
	serveraddr.sin_port = htons(SERVERPORT);

	retval = WSAConnect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR) {
		// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			int err_no = WSAGetLastError();
			error_quit(L"connect()", err_no);
		}
	}

	// WSAAsyncSelect - �ͺ��ŷ ���� �ڵ� ��ȯ
	hWnd = GetConsoleWindow();

	if (NULL == hWnd) {
		printf("Cannot find Consol Window, ERROR : %d\n", __LINE__);
		exit(-1);
	}
	retval = WSAAsyncSelect(sock, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
	if (SOCKET_ERROR == retval) {
		int err_no = WSAGetLastError();
		error_quit(L"ioctlsocket()", err_no);
	}

	wsa_recvbuf.buf = reinterpret_cast<CHAR*>(&recvbuf);
	wsa_recvbuf.len = MAX_BUF_SIZE;
	wsa_sendbuf.buf = reinterpret_cast<CHAR*>(&sendbuf);
	wsa_sendbuf.len = MAX_BUF_SIZE;
}

void AsynchronousClientClass::inputServerIP_ReadtxtFile()
{
	ifstream getServerIP;
	getServerIP.open("ServerIP.txt", ios::in);
	getServerIP.getline(serverIP,32);
	getServerIP.close();
}

void AsynchronousClientClass::inputServerIP_cin()
{
	cout << "\nexample 127.0.0.1\nInput Server's IP : ";
	cin >> serverIP;
}

void AsynchronousClientClass::ConnectingServer()
{
	cout << "choose Server to connect.\n\n0. Read Server IP in .txt File\n1. Input Server IP, directly\n\nchoice method : ";
	short cmd{ 0 };
	cin >> cmd;

	if (cmd & 1) { inputServerIP_cin();	}
	else { inputServerIP_ReadtxtFile();	}

	cout << "\n\n\t--==** Connecting Server, Please Wait **==--\n";
}

SOCKET* AsynchronousClientClass::getServerSocket()
{
	return &sock;
}

int AsynchronousClientClass::recvn()
{
	int received;
	Packet *ptr = recvbuf;
	int left = MAX_BUF_SIZE;

	while (left > 0) {
		received = recv(sock, reinterpret_cast<char*>(&ptr), left, 0);
		if (received == SOCKET_ERROR)
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
	printf("[ %s - %d ]", msg, line);
	wprintf(L"���� %s\n", lpMsgBuf);
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

void AsynchronousClientClass::processPacket()
{
	switch (recvbuf[1])
	{
	case TEST:
		cout << "Server is Running\n";
		break;
	case KEYINPUT:
		break;
	default:
		break;
	}
}

void AsynchronousClientClass::sendPacket(const BYTE data_size, const BYTE type, BYTE* data_start_pointer) {
	// ���� �ִ� ���� ������ ���� ������ ���̰� Ŀ���� �ȵȴ�.
	if (MAX_BUF_SIZE < (data_size + 2)) {
		// �Ʒ��� ���� ������ �߻��ϰ� �ȴٸ�, ���� ����� �ǵ帮�� ���� ���� ������ ũ�⸦ ������ �ٿ� ���� �� �ֵ��� ����
		printf("[ code LINE %d ] [ code FUNCTION %s ] SendPacket class ERROR :: data size overed MAX_BUF_SIZE\n", __LINE__, __FUNCTION__);
	}
	else {		
		// ��Ŷ ���� ���� ���� ����
		sendbuf[0] = data_size + 2;
		sendbuf[1] = type;
		memcpy(&sendbuf[2], data_start_pointer, sendbuf[0]);

		wsa_sendbuf.len = sendbuf[0];
		DWORD ioByteSize;
		retval = WSASend(sock, &wsa_sendbuf, 1, &ioByteSize, 0, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				int err_no = WSAGetLastError();
				error_quit(L"connect()", err_no);
			}
		}
	}
}

// ������ ����ִ���, ������ �ϴ����� ���� �⺻ �׽�Ʈ �Լ�. 3 Bytes �� ������, �⺻ Ÿ���� TEST, �����ʹ� 1�� ������.
void AsynchronousClientClass::sendPacket_TEST() {
	sendbuf[0] = 3;
	sendbuf[1] = TEST;
	sendbuf[2] = 1;		// true

	wsa_sendbuf.len = sendbuf[0];

	DWORD ioByteSize;
	retval = WSASend(sock, &wsa_sendbuf, 1, &ioByteSize, 0, NULL, NULL);
	//int retval = send(sock, reinterpret_cast<char*>(&sendbuf), sendbuf[0], 0);
	if (retval == SOCKET_ERROR) {
		// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			int err_no = WSAGetLastError();
			error_quit(L"connect()", err_no);
		}
	}
}