#pragma once
#pragma comment(lib, "ws2_32")
#include<WinSock2.h>

#define SERVERPORT 9000
#define MAX_BUF_SIZE 256
#define MAX_USER 500

// packet[1] operation
#define DISCONNECTED 0

// iocp buf operation
#define OP_SERVER_RECV 1
#define OP_SERVER_SEND 2

using Packet = unsigned char;

using OVLP_EX = struct Overlap_ex {
	OVERLAPPED original_overlap;
	int operation;
	WSABUF wsabuf;
	Packet iocp_buffer[MAX_BUF_SIZE];
};

using PLAYER_INFO = struct Client_INFO {
	SOCKET s;
	unsigned int id;
	bool connected;
	OVLP_EX recv_overlap;
	int packet_size;
	int previous_size;
	Packet packet_buff[MAX_BUF_SIZE];
};

// Ŭ���̾�Ʈ���� �����͸� �������� �� ��, �Ʒ� Ŭ������ ����ϸ� ���ϴ�.
class SendPacket {
private:
	Packet *buf;	// ���� ������ ������ �� ���� ���� ( ��ü ������ + Ÿ�� + ���� ������ ���... )
	BYTE size = 0;						// ������ ���� �� ĭ�� ���� �Ѵ� ( buf �迭�� �Է� �� ���� +2 �� �������� )
	BYTE type = 0;						// ��Ŷ ������ ���� ĭ�� Ÿ���� ��������

public:
	SendPacket() {};
	~SendPacket() { delete[] buf; };

	// ���� ������ ũ��, �������� ó���� �� �з��� Ÿ��, ������ ���� ������ ������ ���� ������ ������ ��ġ
	SendPacket(const BYTE data_size, const BYTE type, BYTE* data_start_pointer) : size(data_size), type(type) {
		
		// ���� �ִ� ���� ������ ���� ������ ���̰� Ŀ���� �ȵȴ�.
		if (MAX_BUF_SIZE < (data_size + 2)) {
			// �Ʒ��� ���� ������ �߻��ϰ� �ȴٸ�, ���� ����� �ǵ��̱� ���� ���� ������ ũ�⸦ ������ �ٿ� ���� �� �ֵ��� ����
			printf("[ code LINE %d ] [ code FUNCTION %s ] SendPacket class ERROR :: data size overed MAX_BUF_SIZE\n", __LINE__, __FUNCTION__);
		}
		else {
			// Ȥ�� �� ������ ��쿡 ����� �ϴ� �Ҵ�����...
			buf = new Packet[MAX_BUF_SIZE];

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
};