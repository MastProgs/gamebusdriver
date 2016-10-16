#pragma once
#include"stdafx.h"

void boostAsioClient::ProcessWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ���� �߻� ���� Ȯ��
	/*if (WSAGETSELECTERROR(lParam)) {
	error_display("ProcessWinMessage::", WSAGETSELECTERROR(lParam), __LINE__);
	return;
	}*/

	// �޼��� ó��
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_READ: {	// ������ ����
		boost::system::error_code error = boost::asio::error::host_not_found;
		m_socket->read_some(boost::asio::buffer(m_recvBuf), error);

		// recvbuf �� ���� ������ ���� ������ ó��
		processPacket();
	}
				  break;
	case FD_WRITE:	// ������ �۽�
		break;
	case FD_CLOSE:	// �������� ����� ����
		break;
	default:
		break;
	}
}