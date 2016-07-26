#include"protocol.h"
#include<Windows.h>
#include<stdlib.h>

#include<iostream>
#include<thread>
#include<vector>
using namespace std;

/*
	16.07.06
	���� �⺻ Ʋ. Ŭ���̾�Ʈ�� �����ϴ� clients �� ���� array ������, vector ������ ���·� ó�� ������ ���ұ� ������, ���װ� �߻��� ������ �ִ�.
	Ŭ���̾�Ʈ���� ������ �� �����ϱ� ����, �⺻ ���������� ���� �����Ͽ� ���� ���� ���� �̴�.

	16.07.26
	�Ϻ� �ּ� ���� ���� �� process �Լ��� if ������ switch ������ ����
	��� ���� �κ��� �������� ���̱� ���� TRUE == (!ServerShutdwon) ���� ����
*/

// ������ ���ʰ� �Ǵ� �Լ� �� ���� ( ���⼭ �ǵ帱 �Լ� �� ������ ���� )
void GetServerIpAddress();
void ServerClose();
void error_display(char *, int, int);
void error_quit(wchar_t *, int);
int checkCpuCore();
void Initialize();
void SendPacket(int, Packet *);

HANDLE g_hIocp;
bool ServerShutdown{ false };

void workerThreads();
// ----- ��������� �Լ� �� ������ �ǵ帮�� ���� !! -----

// ó�� ������ Ŭ���̾�Ʈ���� �����͸� �ְų�, �ٸ� Ŭ���̾�Ʈ���� ������ ������ �־�� �ϴ� ���, ���⿡ �ڵ尡 ���� �Ѵ�.
void acceptThread();

// ���ƿ� ������ ������� �����͸� �м��Ͽ� ó���ϴ� �Լ�. ( Ŭ���̾�Ʈ���� ���� ��� ��� �����ʹ� �� �Լ��� ��ģ�� )
void ProcessPacket(unsigned int, const Packet[]);

// �����ϴ� Ŭ���̾�Ʈ�鿡�� ������ ��ȣ�� �ִ� ����
unsigned int playerIndex{ UINT_MAX };

// Ŭ���̾�Ʈ�� �����ϸ�, ���������� �� ���Ϳ� ���� �ȴ�. ( playerIndex �� �� ������ Ŭ���̾�Ʈ id�� �Ǹ�, id�� ���� �ٷ� ���� ���� ���� )
vector<PLAYER_INFO *> clients;

int main() {

	_wsetlocale(LC_ALL, L"korean");
	GetServerIpAddress();
	Initialize();
	int cpuCore = checkCpuCore();

	vector<thread *> worker_threads;
	worker_threads.reserve(cpuCore);

	for (int i = 0; i < cpuCore; ++i) { worker_threads.push_back(new thread{ workerThreads }); }

	thread accept_thread{ acceptThread };

	while (ServerShutdown) { Sleep(1000); }

	for (auto thread : worker_threads) {
		thread->join();
		delete thread;
	}

	accept_thread.join();

	ServerClose();
}

// ������ IP�� �˾Ƴ��� �Լ�
void GetServerIpAddress() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	PHOSTENT hostinfo;
	char hostname[50];
	char ipaddr[50];
	memset(hostname, 0, sizeof(hostname));
	memset(ipaddr, 0, sizeof(ipaddr));

	int nError = gethostname(hostname, sizeof(hostname));
	if (nError == 0)
	{
		hostinfo = gethostbyname(hostname);
		strcpy(ipaddr, inet_ntoa(*reinterpret_cast<struct in_addr*>(hostinfo->h_addr_list[0])));
	}
	WSACleanup();
	printf("This Server's IP address : %s\n", ipaddr);
}

// ���� ���� ���� ����� �Լ�
void ServerClose() {
	WSACleanup();
}

// ���� ��Ȳ�� ����������, ������ �����Ű�� �ʴ´�. �� ��° �ٿ��� ������ �߻��ߴ��� �ڵ� �� ���� ���� ��ġ�� �˾Ƴ� �� �ִ�.
void error_display(char *msg, int err_no, int line) {
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[ %s - %d ]", msg, line);
	wprintf(L"���� %s\n", lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// ���� ��Ȳ�� �����ϰ�, ������ ����
void error_quit(wchar_t *msg, int err_no) {
	WCHAR *lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(-1);
}

// ��Ƽ�����带 ���� cpu �ھ� �� �� üũ �Լ� ( �ִ� ������ ������ ���� )
int checkCpuCore() {
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int cpuCore = static_cast<int>(si.dwNumberOfProcessors) * 2;
	printf("CPU Core Count = %d, threads = %d\n", cpuCore / 2, cpuCore);
	return cpuCore;
}

// ���ϵ� �ʱ�ȭ�� ������, �⺻���� Ŭ���̾�Ʈ�� �ٸ� ��, ó�� ����Ǿ� �޸𸮿� �ö󰡾��� ���׵��� �ʱ�ȭ ���ִ� �Լ�
void Initialize() {

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (g_hIocp == NULL) {
		int err_no = WSAGetLastError();
		error_quit(L"Initialize::CreateIoCompletionPort", err_no);
	}
}

// Ŭ���̾�Ʈ�� ���� ����� ������ �Լ� ( ������ �ϴ� Ŭ���̾�Ʈ id ��, ��Ŷ�� ������ �ϴ� ���� �ּҰ� �ʿ��ϴ� )
void SendPacket(int id, Packet *packet) {
	// packet[0] = packet size
	// packet[1] = type
	// packet[...] = data

	OVLP_EX *over = new OVLP_EX;
	memset(over, 0, sizeof(OVLP_EX));
	over->operation = OP_SERVER_SEND;
	over->wsabuf.buf = reinterpret_cast<char *>(over->iocp_buffer);
	over->wsabuf.len = packet[0];
	memcpy(over->iocp_buffer, packet, packet[0]);

	DWORD flags{ 0 };
	int retval = WSASend(clients[id]->s, &over->wsabuf, 1, NULL, flags, &over->original_overlap, NULL);
	if (SOCKET_ERROR == retval) {
		int err_no = WSAGetLastError();
		if (ERROR_IO_PENDING != err_no) {
			error_display("SendPacket::WSASend", err_no, __LINE__);
			while (true);
		}
	}
}

void workerThreads() {
	while (TRUE == (!ServerShutdown)) {
		DWORD key;
		DWORD iosize;
		OVLP_EX *my_overlap;

		BOOL result = GetQueuedCompletionStatus(g_hIocp, &iosize, &key, reinterpret_cast<LPOVERLAPPED *>(&my_overlap), INFINITE);

		// Ŭ���̾�Ʈ�� ������ ������ ���
		if (FALSE == result || 0 == iosize) {
			if (FALSE == result) {
				int err_no = WSAGetLastError();
				error_display("WorkerThreadStart::GetQueuedCompletionStatus", err_no, __LINE__);
			}
			
			closesocket(clients[key]->s);
			clients[key]->connected = false;

			/* view list ���� ������ */
			/* ��� Ŭ���̾�Ʈ����, ���� Ŭ���̾�Ʈ�� ����ٰ� �˷����� */

			/*Packet p[11];
			p[0] = 11;
			p[1] = DISCONNECTED;
			*((int *)(&p[2])) = key;*/

			//for (int i = 0; i < clients.size(); ++i) {
			//	if (false == clients[i]->connected) { continue; }
			//	//if (i == playerIndex) { continue; }

			//	SendPacket(i, p);
			//}
			continue;
		}
		else if (OP_SERVER_RECV == my_overlap->operation) {
			// Ŭ���̾�Ʈ�� ���� �����͸� �޾��� ���
			Packet *buf_ptr = clients[key]->recv_overlap.iocp_buffer;
			int remained = iosize;
			while (0 < remained) {
				if (0 == clients[key]->packet_size) { clients[key]->packet_size = buf_ptr[0]; }

				int required = clients[key]->packet_size - clients[key]->previous_size;

				if (remained >= required) {
					memcpy(clients[key]->packet_buff + clients[key]->previous_size, buf_ptr, required);

					// �Ʒ� �Լ����� ��Ŷ�� ó���ϰ� �ȴ�.
					ProcessPacket(key, clients[key]->packet_buff);

					buf_ptr += required;
					remained -= required;

					clients[key]->packet_size = 0;
					clients[key]->previous_size = 0;
				}
				else {
					memcpy(clients[key]->packet_buff + clients[key]->previous_size, buf_ptr, remained);
					buf_ptr += remained;
					clients[key]->previous_size += remained;
					remained = 0;
				}
			}
			DWORD flags = 0;
			int retval = WSARecv(clients[key]->s, &clients[key]->recv_overlap.wsabuf, 1, NULL, &flags, &clients[key]->recv_overlap.original_overlap, NULL);
			if (SOCKET_ERROR == retval) {
				int err_no = WSAGetLastError();
				if (ERROR_IO_PENDING != err_no) {
					error_display("WorkerThreadStart::WSARecv", err_no, __LINE__);
				}
				continue;
			}
		}
		else if (OP_SERVER_SEND == my_overlap->operation) {
			// �������� �޼����� ��������, �޸𸮸� ������ �ش�.
			delete my_overlap;
		}
		else {
			cout << "Unknown IOCP event !!\n";
			exit(-1);
		}
	}
}

void acceptThread() {
	int retval{ 0 };

	// socket() - IPv4 ( AF_INET )
	SOCKET listen_sock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listen_sock == INVALID_SOCKET) {
		int err_no = WSAGetLastError();
		error_quit(L"socket()", err_no);
	};

	// bind()
	struct sockaddr_in serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = ::bind(listen_sock, reinterpret_cast<struct sockaddr *>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) {
		int err_no = WSAGetLastError();
		error_quit(L"socket()", err_no);
	}

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) {
		int err_no = WSAGetLastError();
		error_quit(L"socket()", err_no);
	}
	
	while (TRUE == (!ServerShutdown)) {
		// accept()
		struct sockaddr_in clientaddr;
		int addrlen = sizeof(clientaddr);
		SOCKET client_sock = WSAAccept(listen_sock, reinterpret_cast<sockaddr *>(&clientaddr), &addrlen, NULL, NULL);
		if (INVALID_SOCKET == client_sock) {
			int err_no = WSAGetLastError();
			error_display("Accept::WSAAccept", err_no, __LINE__);
			while (true);
		}

		/* DB ���� login ����� ���⿡ �߰��Ǿ�� �Ѵ�. �α����� ��ȣ�� ����� ������ ���, �ƴϸ� Ŭ���̾�Ʈ ������ ���´�. �α����� �ϸ� DB���� ������ �����´� */

		playerIndex += 1;
		printf("[ No. %u ] Client IP = %s, Port = %d is Connected\n", playerIndex, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(client_sock), g_hIocp, playerIndex, 0);

		PLAYER_INFO *user = new PLAYER_INFO;

		user->s = client_sock;
		user->connected = true;
		user->id = playerIndex;
		user->packet_size = 0;
		user->previous_size = 0;
		memset(&user->recv_overlap.original_overlap, 0, sizeof(user->recv_overlap.original_overlap));
		user->recv_overlap.operation = OP_SERVER_RECV;
		user->recv_overlap.wsabuf.buf = reinterpret_cast<char*>(&user->recv_overlap.iocp_buffer);
		user->recv_overlap.wsabuf.len = sizeof(user->recv_overlap.iocp_buffer);

		/* �߰��� �����Ͱ� �ʿ��� �����, PLAYER_INFO ����ü ������ �����ϰ�, �߰��� �ʱ�ȭ ���̳� �ҷ����� ������ �����Ͽ� �־��־�� �Ѵ�. */

		clients.push_back(move(user));

		/* �ֺ� Ŭ���̾�Ʈ�� ���� �Ѹ� ���� �Ѹ���, �þ� ����Ʈ�� ó���ؾ� �� �����鵵 �Բ� �ִ´�. */		

		// Ŭ���̾�Ʈ���� ������� ��ٸ���
		DWORD flags{ 0 };
		retval = WSARecv(client_sock, &clients[playerIndex]->recv_overlap.wsabuf, 1, NULL, &flags, &clients[playerIndex]->recv_overlap.original_overlap, NULL);
		if (SOCKET_ERROR == retval) {
			int err_no = WSAGetLastError();
			if (ERROR_IO_PENDING != err_no) {
				error_display("Accept::WSARecv", err_no, __LINE__);
			}
		}
	}
}

// ��� ��Ŷ�� �� �Լ��� ���İ���.
void ProcessPacket(unsigned int id, const Packet buf[]) {
	// packet[0] = packet size		> 0��° �ڸ����� ������, ��Ŷ�� ũ�Ⱑ ���߸� �Ѵ�.
	// packet[1] = type				> 1��° �ڸ����� ���� ��Ŷ�� ���� ��Ŷ���� �Ӽ��� �����ִ� ���̴�.
	// packet[...] = data			> 2��° ���ʹ� �Ӽ��� �´� ����� ó���� ���ش�.

	// buf[1] ��°�� �Ӽ����� �з��� �� �ڿ�, ���ο��� 2��° ���� �����͸� ó���ϱ� �����Ѵ�.
	switch (buf[1])
	{
	case 1:
	{

	}
		break;
	default:
	{
		// Ŭ���̾�Ʈ�� ���� �˼� ���� �����Ͱ� ���� ���, ��ŷ ������ ���� ������ ���� ����. �ش� Ŭ���̾�Ʈ�� ���� ��ȣ�� Ÿ�� ��ȣ�� �˷��ش�.
		printf("ERROR, Unknown signal -> [ %u ] protocol num = %d\n", id, buf[1]);
		exit(-1);
	}
		break;
	}
}