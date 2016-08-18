#pragma once
#pragma comment(lib, "ws2_32")
#include<WinSock2.h>
#include<queue>

#define SERVERPORT 9000
#define MAX_BUF_SIZE 256
#define MAX_USER 500

// packet[1] operation
#define DISCONNECTED 0

// iocp buf operation
#define OP_SERVER_RECV 1
#define OP_SERVER_SEND 2

// process protocol
#define TEST 1			// ���� ��Ŷ �״�� �����ֱ��. ( ���� ������ �״�� ���ƿ��� �ʴ´ٸ�, Ŭ�� ������ ������ �ִٴ� �� )

using Packet = unsigned char;

// ���� �˻�� �Լ��� �ǵ帮�� �ʾƵ� �ȴ�
// ���� ��Ȳ�� ����������, ������ �����Ű�� �ʴ´�. �� ��° �ٿ��� ������ �߻��ߴ��� �ڵ� �� ���� ���� ��ġ�� �˾Ƴ� �� �ִ�.
// error_display( "ȭ�鿡 ����� �޼���", ���� ��ȣ, __LINE__ )
void error_display(char *msg, int err_no, int line) {
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[ %s - %d ]", msg, line);
	wprintf(L"���� %s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ���� ��Ȳ�� �����ϰ�, ������ ����
// error_quit( L"ȭ�鿡 ����� �޼���", ���� ��ȣ, __LINE__ ) - �ƿ� ���α׷��� ���� �ȴ�.
void error_quit(wchar_t *msg, int err_no) {
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// ���� ��Ŷ �Ҵ������ �޸� ������ ( �޼����� ���� ��� �� �տ� �ִ� �ش� �ּҸ޸� ���� ��, ť���� ���ش� ) - < ���ͷ� buf_recv �迭�� ��ĥ ���� >
queue<Packet*> free_memory_queue;

// Ŭ���̾�Ʈ ���� �ޱ� & ������ ����
Packet buf_send[MAX_BUF_SIZE] = { 0 };
Packet buf_recv[MAX_BUF_SIZE] = { 0 };

// Ŭ���̾�Ʈ���� �����͸� �������� �� ��, �Ʒ� Ŭ������ ����ϸ� ���ϴ�.
class SendPacket {
private:
	Packet *buf;	// ���� ������ ������ �� ���� ���� ( ��ü ������ + Ÿ�� + ���� ������ ���... )
	BYTE size = 0;	// ������ ���� �� ĭ�� ���� �Ѵ� ( buf �迭�� �Է� �� ���� +2 �� �������� )
	BYTE type = 0;	// ��Ŷ ������ ���� ĭ�� Ÿ���� ��������

public:
	SendPacket() {};
	~SendPacket() { /*delete[] buf;*/ };

	// ���� ������ ũ��, �������� ó���� �� �з��� Ÿ��, ������ ���� ������ ������ ���� ������ ������ ��ġ
	SendPacket(const BYTE data_size, const BYTE type, BYTE* data_start_pointer) : size(data_size), type(type) {
		
		// ���� �ִ� ���� ������ ���� ������ ���̰� Ŀ���� �ȵȴ�.
		if (MAX_BUF_SIZE < (data_size + 2)) {
			// �Ʒ��� ���� ������ �߻��ϰ� �ȴٸ�, ���� ����� �ǵ��̱� ���� ���� ������ ũ�⸦ ������ �ٿ� ���� �� �ֵ��� ����
			printf("[ code LINE %d ] [ code FUNCTION %s ] SendPacket class ERROR :: data size overed MAX_BUF_SIZE\n", __LINE__, __FUNCTION__);
		}
		else {
			// Ȥ�� �� ������ ��쿡 ����� �ϴ� �Ҵ�����...
			//buf = new Packet[MAX_BUF_SIZE];

			// �Ҵ� �� �񵿱� �۵��� �����߻��� Ȯ���� ���Ƽ� �Ϲ� ���� ���� ���۸� Ȱ���Ѵ�.
			buf = buf_send;

			// ��Ŷ ���� ���� ���� ����
			buf[0] = size + 2;
			buf[1] = type;
			memcpy(&buf[2], data_start_pointer, size);
		}
	}

	void Send(const SOCKET s) {
		if (0 >= size) { printf("[ code LINE %d ] SendPacket class ERROR :: there is no data size", __LINE__); }
		if (2 >= buf[0]) { printf("[ code LINE %d ] SendPacket class ERROR :: there is no data size", __LINE__); }

		int retval = send(s, reinterpret_cast<char*>(&buf), buf[0], 0);
		if (retval == SOCKET_ERROR) {
			// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				int err_no = WSAGetLastError();
				error_quit(L"connect()", err_no);
			}
		}
	}

	// ������ ����ִ���, ������ �ϴ����� ���� �⺻ �׽�Ʈ �Լ�. 3 Bytes �� ������, �⺻ Ÿ���� TEST, �����ʹ� 1�� ������.
	void Send_default_test(const SOCKET s) {
		buf[0] = 3;
		buf[1] = TEST;
		buf[2] = 1;		// true

		int retval = send(s, reinterpret_cast<char*>(&buf), buf[0], 0);
		if (retval == SOCKET_ERROR) {
			// �񵿱� �����̶� �׳� ����, �˻� ���־�� ��
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				int err_no = WSAGetLastError();
				error_quit(L"connect()", err_no);
			}
		}
	}

	Packet* getPacketBuf() { return buf; }
};