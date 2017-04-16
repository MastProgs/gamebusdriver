#pragma once
#include"stdafx.h"
#include"../Server_Code/ClientClass.h"
#include "ObjMgr.h"
#include "Obj.h"
#include "RenderMgr.h"
#include "OtherPlayer.h"
#include "SceneMgr.h"
#include "Player.h"

void AsynchronousClientClass::processPacket(Packet* buf)
{

	switch (buf[1])
	{
	case CHANGED_POSITION: {
		sc_move *p = reinterpret_cast<sc_move*>(buf);

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(p->id);

			if (nullptr != data)
			{
				data->pos = p->pos;

				for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer"))
				{
					if (iter->GetPacketData()->id == p->id) {
						if (((COtherPlayer*)iter)->GetAniState() == PLAYER_IDLE) {
							((COtherPlayer*)iter)->m_bKey = true;
							((COtherPlayer*)iter)->SetAniState(PLAYER_MOVE);
						}
						break;
					}
				}
			}
			else { break; }
		}
		break;
	}
	case CHANGED_DIRECTION: {
		sc_dir *p = reinterpret_cast<sc_dir*>(buf);

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(p->id);
			if (nullptr != data) { data->dir = p->dir; }
			else { break; }
		}
		break;
	}

	case SERVER_MESSAGE_HP_CHANGED: {
		sc_hp *p = reinterpret_cast<sc_hp*>(buf);

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			if (m_player.id == p->id) {
				m_player.state.hp = p->hp;
				(*(CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin()))->SetPacketHp(&p->hp);
				break;
			}

			// ���� �ƴ϶�� �ٸ��� hp ���
			(CObjMgr::GetInstance()->Get_PlayerServerData(p->id))->state.hp = p->hp;
		}
		break;
	}

	case KEYINPUT_ATTACK: {
		sc_atk *p = reinterpret_cast<sc_atk*>(buf);

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			// ���� � ���� Į���� �ߴ��� ���̵� Ȯ�� ��, �� �༮ �ִϸ��̼� ���� ( �� ���̵� �ƴϸ� �ϴ� �ִϸ��̼� �۵� ���Ѿ� �� )
			/// ���� �� ��ġ�� �� ���� id �� ����ִٸ�, �̹� Ű�� ���� �������� �ִϸ��̼��� ����߱� ������, �׳� if ���� �Ѿ��.
			if (m_player.id != p->attacking_id) {

				for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")) {
					if (iter->GetPacketData()->id == p->attacking_id) {
						if ((reinterpret_cast<COtherPlayer*>(iter))->GetAniState() == PLAYER_IDLE)
						{
							(reinterpret_cast<COtherPlayer*>(iter))->m_bKey = true;
							(reinterpret_cast<COtherPlayer*>(iter))->SetAniState(PLAYER_ATT1);
						}
						break;
					}
				}

				// *(reinterpret_cast<UINT*>(&buf[sizeof(int) + sizeof(int) + 2])) �� ��ȣ Ŭ���̾�Ʈ�� �ִϸ��̼��� �۵����Ѿ� �Ѵ�.
				/// ����� �ϴ� ����, ���� �þ߿��� �����, �����Ϳ��� ������ ��Ȳ�� ���� �� �ֱ⶧����, ���� ����� ���� Ŭ���̾�Ʈ�� ���� ���� ó���� ���־�� �Ѵ�.
				/// ���� �� ȿ�������� �ϱ� ���ؼ���, ���ʿ� ī�޶� �ۿ� ��ġ�� Ŭ���̾�Ʈ�� ���ؼ� ó���� ���ϴ°� ���� ������...?

			}

			// ���� ���ظ� ���� ���̶��, �� hp �� ��� break;
			/// -> ��Ȯ���� hp ��ü �ѷ� ���̴�. �򰥸��� �ʵ��� ����
			/// �ٽø��ϸ�, ���� hp ����, ��Ŷ���� ���Ƶ��� hp ���� ���̸� ���ؼ� �������� �� ������ ǥ�����־�� �Ѵ�.
			/// ���ϸ�, ���̴� �������� �������� ��Ŷ���� ���� �� ���� ����.
			if (m_player.id == p->under_attack_id) {
				m_player.state.hp = p->hp;
				(*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->SetPacketHp(&p->hp);
				break;
			}

			// ���� �ƴ϶�� �ٸ��� hp ���
			CObjMgr::GetInstance()->Get_PlayerServerData(p->under_attack_id)->state.hp = p->hp;

			if (0 >= p->hp)
			{
				list<CObj*>* ListObj = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer");
				list<CObj*>::iterator iter = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->begin();
				list<CObj*>::iterator iter_end = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->end();

				for (iter; iter != iter_end;)
				{
					if ((*iter)->GetPacketData()->id == p->under_attack_id)
					{
						CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
						::Safe_Delete(*iter);
						iter = ListObj->erase(iter);
						break;
					}
				}
			}

		}
		break;
	}

	default:	// �� �Ⱦ��̴� ��Ŷ��

		switch (buf[1])
		{
		case TEST:
#ifdef _DEBUG
			// �̰� �׽�Ʈ ���̴ϱ� �׳� ���θ� �ǰ� ����
			cout << "Server is Running. TEST Packet Recived Successfully.\n";
#endif
			break;
		case INIT_CLIENT: {
			m_player = reinterpret_cast<sc_client_init_info *>(buf)->player_info;
			break;
		}
		case INIT_OTHER_CLIENT: {
			sc_other_init_info *p = reinterpret_cast<sc_other_init_info *>(buf);

			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(p->playerData.id);

			if (nullptr != data) { break; }
			else {
				CObj* pObj = COtherPlayer::Create();
				pObj->SetPos(D3DXVECTOR3(m_player.pos.x, 1.f, m_player.pos.y));
				pObj->SetPacketData(&p->playerData);
				CObjMgr::GetInstance()->AddObject(L"OtherPlayer", pObj);
				CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);
			}
			break;
		}
		case PLAYER_DISCONNECTED: {
			sc_disconnect *p = reinterpret_cast<sc_disconnect*>(buf);
			
			list<CObj*>::iterator iter = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->begin();
			list<CObj*>::iterator iter_end = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->end();

			for (iter; iter != iter_end;)
			{
				if ((*iter)->GetPacketData()->id == p->id)
				{
					CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
					::Safe_Delete(*iter);
					iter = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->erase(iter);
					break;
				}
			}
			break;
		}
		default:
			break;
		}

		// default break;
		break;
	}
}
