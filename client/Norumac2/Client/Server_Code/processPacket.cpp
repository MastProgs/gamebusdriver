#pragma once
#include"stdafx.h"
#include"../../../../server/serverBoostModel/serverBoostModel/protocol.h"
#include"../Server_Code/ClientClass.h"
#include "ObjMgr.h"
#include "Obj.h"
#include "RenderMgr.h"
#include "OtherPlayer.h"
#include "SceneMgr.h"

void AsynchronousClientClass::processPacket(Packet* buf)
{
	switch (buf[1])
	{
	case TEST:
#ifdef _DEBUG
		// �̰� �׽�Ʈ ���̴ϱ� �׳� ���θ� �ǰ� ����
		cout << "Server is Running. TEST Packet Recived Successfully.\n";
#endif
		break;
	case INIT_CLIENT: {
		m_player = *(reinterpret_cast<player_data*>(&buf[2]));
		// ����� ��ü�� �Ǵ�, ���� ���� ������ �ϴ� �÷��̾��� �����Ͱ� �����ϱ�
		// �������̶���� ��ġ��������� �ϴ� �� ����� ���´���
	}
		break;
	case INIT_OTHER_CLIENT: {
		/*unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(reinterpret_cast<player_data*>(&buf[2])->id);
		if (m_other_players.end() == ptr) { m_other_players.insert(make_pair(reinterpret_cast<player_data*>(&buf[2])->id, *reinterpret_cast<player_data*>(&buf[2]))); }
		else { ptr->second.pos = *reinterpret_cast<position*>(&buf[2]); }

		InvalidateRect(m_hWnd, NULL, TRUE);*/
		//if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(reinterpret_cast<player_data*>(&buf[2])->id);

			if (nullptr != data) { break; }
			else {
				// ������ �߰� ����� �� ���� ����?
				// ���� data �� ���� end ��ġ���, �����Ͱ� ���µ� �������� �ϴ°��ݾ�
				// �׷� ���ο� �÷��̾ �����Ǿ��ٴ� ���̴ϱ�
				// ���� ��ü �߰��ؼ� �� ���� �־���߰���? ����?

				CObj* pObj = NULL;
				pObj = COtherPlayer::Create();
				pObj->SetPos(D3DXVECTOR3(m_player.pos.x, 1.f, m_player.pos.y));
				pObj->SetPacketData(reinterpret_cast<player_data*>(&buf[2]));
				CObjMgr::GetInstance()->AddObject(L"OtherPlayer", pObj);
				CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);

			}
		}
	}
		break;
	case PLAYER_DISCONNECTED: {
		// ���⵵, id �� ã�Ƽ�, �� ���̵� ã���� �� ��ü�� ���� �ؾ߰���? ��������?
		player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(reinterpret_cast<player_data*>(&buf[2])->id);

		list<CObj*>::iterator iter = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->begin();
		list<CObj*>::iterator iter_end = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->end();

		for (iter; iter != iter_end;)
		{
			if ((*iter)->GetPacketData()->id == reinterpret_cast<player_data*>(data)->id)
			{
				CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
				::Safe_Release(*iter);
				CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->erase(iter++);
			}
			else
				++iter;

		}

	}
		break;
	case CHANGED_POSITION: {

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(*reinterpret_cast<unsigned int*>(&buf[sizeof(position)+2]));

			if (nullptr != data) { memcpy(&data->pos, &buf[2], sizeof(player_data)); }
			else
				break;
		}
	}
		break;
	case CHANGED_DIRECTION: {

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(*reinterpret_cast<unsigned int*>(&buf[sizeof(char)+2]));

			if (nullptr != data) { memcpy(&data->dir, &buf[2], sizeof(char)); }
			else
				break;
		}
	}
		break;
	default:
		break;
	}
}
