#pragma once
#include"stdafx.h"
#include"../../../../server/serverBoostModel/serverBoostModel/protocol.h"
#include"../Server_Code/ClientClass.h"
#include "ObjMgr.h"
#include "Obj.h"

void AsynchronousClientClass::processPacket()
{
	switch (m_recvbuf[1])
	{
	case TEST:
#ifdef _DEBUG
		// �̰� �׽�Ʈ ���̴ϱ� �׳� ���θ� �ǰ� ����
		cout << "Server is Running. TEST Packet Recived Successfully.\n";
#endif
		break;
	case INIT_CLIENT: {
		//m_player = *(reinterpret_cast<player_data*>(&m_recvbuf[2]));
		// ����� ��ü�� �Ǵ�, ���� ���� ������ �ϴ� �÷��̾��� �����Ͱ� �����ϱ�
		// �������̶���� ��ġ��������� �ϴ� �� ����� ���´���
	}
		break;
	case PLAYER_DISCONNECTED: {
		// ���⵵, id �� ã�Ƽ�, �� ���̵� ã���� �� ��ü�� ���� �ؾ߰���? ��������?
	}
		break;
	case KEYINPUT: {
		player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(reinterpret_cast<player_data*>(&m_recvbuf[2])->id);

		if (nullptr != data) { memcpy(data, &m_recvbuf[2], m_recvbuf[0]); }
		else {
			// ������ �߰� ����� �� ���� ����?
			// ���� data �� ���� end ��ġ���, �����Ͱ� ���µ� �������� �ϴ°��ݾ�
			// �׷� ���ο� �÷��̾ �����Ǿ��ٴ� ���̴ϱ�
			// ���� ��ü �߰��ؼ� �� ���� �־���߰���? ����?
		}
	}
		break;
	default:
		break;
	}
}
