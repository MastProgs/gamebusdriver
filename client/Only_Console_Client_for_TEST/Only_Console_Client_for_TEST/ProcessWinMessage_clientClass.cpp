#include"stdafx.h"

void AsynchronousClientClass::ProcessWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	// ���� �߻� ���� Ȯ��
	if (WSAGETSELECTERROR(lParam)) {
		error_display("ProcessWinMessage::", WSAGETSELECTERROR(lParam), __LINE__);
		return;
	}

	// �޼��� ó��
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ:	// ������ ����
		//retval = recvn();

		DWORD ioByteSize;
		retval = WSARecv(sock, &wsa_recvbuf, 1, NULL, &ioByteSize, 0, NULL);
		if (SOCKET_ERROR == retval) {
			int err_no = WSAGetLastError();
			error_display("FD_READ::", err_no, __LINE__);
			return;
		}
		recvbytes = retval;

		// recvbuf �� ���� ������ ���� ������ ó��
		processPacket();

		break;
	case FD_WRITE:	// ������ �۽�
		break;
	case FD_CLOSE:	// �������� ����� ����
		break;
	default:
		break;
	}
}