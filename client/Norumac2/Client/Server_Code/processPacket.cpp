#pragma once
#include"stdafx.h"
#include"../../../../server/serverBoostModel/serverBoostModel/protocol.h"
#include"../Server_Code/ClientClass.h"
#include "ObjMgr.h"
#include "Obj.h"
#include "RenderMgr.h"
#include "OtherPlayer.h"
#include "SceneMgr.h"

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
		m_player = *(reinterpret_cast<player_data*>(&m_recvbuf[2]));
		// ����� ��ü�� �Ǵ�, ���� ���� ������ �ϴ� �÷��̾��� �����Ͱ� �����ϱ�
		// �������̶���� ��ġ��������� �ϴ� �� ����� ���´���
	}
		break;
	case PLAYER_DISCONNECTED: {
		// ���⵵, id �� ã�Ƽ�, �� ���̵� ã���� �� ��ü�� ���� �ؾ߰���? ��������?
		player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(reinterpret_cast<player_data*>(&m_recvbuf[2])->id);

		//CObjMgr::GetInstance()->Get_ObjList()
	}
		break;
	case KEYINPUT: {

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(reinterpret_cast<player_data*>(&m_recvbuf[2])->id);

			if (nullptr != data) { memcpy(data, &m_recvbuf[2], m_recvbuf[0]); }
			else {
				// ������ �߰� ����� �� ���� ����?
				// ���� data �� ���� end ��ġ���, �����Ͱ� ���µ� �������� �ϴ°��ݾ�
				// �׷� ���ο� �÷��̾ �����Ǿ��ٴ� ���̴ϱ�
				// ���� ��ü �߰��ؼ� �� ���� �־���߰���? ����?

				CObj* pObj = NULL;
				pObj = COtherPlayer::Create();
				pObj->SetPos(D3DXVECTOR3(m_player.pos.x, 1.f, m_player.pos.y));
				pObj->SetPacketData(reinterpret_cast<player_data*>(&m_recvbuf[2]));
				CObjMgr::GetInstance()->AddObject(L"OtherPlayer", pObj);
				CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);

			}
		}
	}
		break;
	default:
		break;
	}
}
