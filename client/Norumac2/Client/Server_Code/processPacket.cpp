#pragma once
#include"stdafx.h"
#include"../Server_Code/ClientClass.h"
#include "ObjMgr.h"
#include "Obj.h"
#include "RenderMgr.h"
#include "OtherPlayer.h"
#include "SceneMgr.h"
#include "Player.h"
#include "Monster.h"
#include "DamageFont.h"
#include "Info.h"
#include "MobHpBasic.h"
#include "../Include/MobHpBar.h"

void AsynchronousClientClass::processPacket(Packet* buf)
{

	switch (buf[1])
	{
	case CHANGED_POSITION: {
		sc_move *p = reinterpret_cast<sc_move*>(buf);
		//cout << "moved ID : " << p->id << ", position " << p->pos.x << ", " << p->pos.y << endl;;
		
		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = NULL;

			if (p->id >= MAX_AI_NUM)
				data = CObjMgr::GetInstance()->Get_PlayerServerData(p->id);
			else
				data = CObjMgr::GetInstance()->Get_MonsterServerData(p->id);

			if (nullptr != data)
			{
				data->pos = p->pos;

				if (p->id < MAX_AI_NUM)
				{
					for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"Monster"))
					{
						if (iter->GetPacketData()->id == p->id) {
							if (((CMonster*)iter)->GetAniState() == MONSTER_IDLE) {
								((CMonster*)iter)->m_bKey = true;
								((CMonster*)iter)->SetAniState(MONSTER_MOVE);
							}
							break;
						}
					}
				}

				else
				{
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

			}
			else { break; }
		}
		break;
	}
	case CHANGED_DIRECTION: {
		sc_dir *p = reinterpret_cast<sc_dir*>(buf);
		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			player_data *data = nullptr;
			if (p->id >= MAX_AI_NUM)
				data = CObjMgr::GetInstance()->Get_PlayerServerData(p->id);
			else
				data = CObjMgr::GetInstance()->Get_MonsterServerData(p->id);

			if (nullptr != data) { data->dir = p->dir; }
			else { break; }
		}
		break;
	}

	case CHANGED_FEVER: {
		sc_fever *p = reinterpret_cast<sc_fever*>(buf);
		//cout << "�� Fever Gauge ���� �� = " << p->gauge << "\n";
		m_player.state.gauge = p->gauge;
		(*(CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin()))->SetPacketFever(&p->gauge);

		break;
	}

	case SERVER_MESSAGE_HP_CHANGED: {
		sc_hp *p = reinterpret_cast<sc_hp*>(buf);

		cout << "[ " << p->id << " ] ����� HP = " << p->hp << "\n";

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			if (m_player.id == p->id) {
				m_player.state.hp = p->hp;
				(*(CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin()))->SetPacketHp(&p->hp);
				break;
			}

			// ���� �ƴ϶�� �ٸ��� hp ���
			if (NULL != (CObjMgr::GetInstance()->Get_PlayerServerData(p->id))) {
				(CObjMgr::GetInstance()->Get_PlayerServerData(p->id))->state.hp = p->hp;
			}
		}
		break;
	}

	case KEYINPUT_ATTACK: {
		sc_atk *p = reinterpret_cast<sc_atk*>(buf);

		// ���ڱ� �� ���⼭ �ٸ� �÷��̾� ���� ��Ŷ�� �ȳ��� ������ �� ���� ���� ( �ѹ� �� Ȯ���� ���� *************************** )
		cout << "[ " << p->attacking_id << " ] -> [ " << p->under_attack_id << " ] �� ������. ���� HP = " << p->hp << "\n";

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			// ���� � ���� Į���� �ߴ��� ���̵� Ȯ�� ��, �� �༮ �ִϸ��̼� ���� ( �� ���̵� �ƴϸ� �ϴ� �ִϸ��̼� �۵� ���Ѿ� �� )
			/// ���� �� ��ġ�� �� ���� id �� ����ִٸ�, �̹� Ű�� ���� �������� �ִϸ��̼��� ����߱� ������, �׳� if ���� �Ѿ��.
			if (m_player.id != p->attacking_id) {

				if (p->attacking_id < MAX_AI_NUM)
				{
					for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"Monster"))
					{
						if (iter->GetPacketData()->id == p->attacking_id) {
							if ((reinterpret_cast<CMonster*>(iter))->GetAniState() == MONSTER_IDLE)
							{
								(reinterpret_cast<CMonster*>(iter))->m_bKey = true;
								(reinterpret_cast<CMonster*>(iter))->SetAniState(MONSTER_ATT);
							}
							break;
						}
					}
				}

				else
				{
					for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer"))
					{
						if (iter->GetPacketData()->id == p->attacking_id) {
							if ((reinterpret_cast<COtherPlayer*>(iter))->GetAniState() == PLAYER_IDLE)
							{
								(reinterpret_cast<COtherPlayer*>(iter))->m_bKey = true;
								(reinterpret_cast<COtherPlayer*>(iter))->SetAniState(PLAYER_ATT1);
							}
							break;
						}
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

			if (p->under_attack_id >= MAX_AI_NUM) // �÷��̾��Ͻ�
			{
				if (NULL != (CObjMgr::GetInstance()->Get_PlayerServerData(p->under_attack_id))) {
					(CObjMgr::GetInstance()->Get_PlayerServerData(p->under_attack_id))->state.hp = p->hp;

					/*CObj* pObj = CDamageFont::Create();
					pObj->SetPos(D3DXVECTOR3(m_player.pos.x, 1.f, m_player.pos.y));
					pObj->SetPacketData(&p->playerData);
					CObjMgr::GetInstance()->AddObject(L"OtherPlayer", pObj);
					CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);*/
				}
			}
			else // �����Ͻ�
			{
				if (NULL != (CObjMgr::GetInstance()->Get_MonsterServerData(p->under_attack_id))) {
					(CObjMgr::GetInstance()->Get_MonsterServerData(p->under_attack_id))->state.hp = p->hp;


					//////////////������ ��Ʈ////////////////////////
					list<CObj*>* ListObj = CObjMgr::GetInstance()->Get_ObjList(L"Monster");
					list<CObj*>::iterator iter = ListObj->begin();
					list<CObj*>::iterator iter_end = ListObj->end();
					D3DXVECTOR3 vPos;

					for (; iter != iter_end; ++iter)
					{
						if ((*iter)->GetPacketData()->id == p->under_attack_id)
						{
							vPos = (*iter)->GetInfo()->m_vPos;
							break;
						}
					}

					CObj* pObj = CDamageFont::Create(&vPos, 105.f); //���ð� 105. ���߿� �������� �������� �޾ƿͼ� ���.
					CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);
					///////////////////////////////////////////////////

					/////////////////ü�¹�///////////////////////////

					int iSize = 0;
					//���� ü�¹ٰ� ������ �����.
					if (m_bAttackFirst == false)//����
					{
						iSize = 0;
						m_bAttackFirst = true;
					}
					else
						iSize = ((CObjMgr::GetInstance()->Get_ObjList(L"MobHpBaisic")->size()));

					if (iSize == 0)
					{
						CObj * pObj = CMobHpBar::Create();
						dynamic_cast<CMobHpBar*>(pObj)->SetRendHp(p->hp, 100); // �ִ�ü���� ����� �ٲ�����.
						CObjMgr::GetInstance()->AddObject(L"MobHpBar", pObj);

						pObj = CMobHpBasic::Create();
						dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"Monster", p->under_attack_id);
						CObjMgr::GetInstance()->AddObject(L"MobHpBaisic",pObj);			
					}
					//�ִٸ� �����ͼ� �̸��� ü���� �����ϰ� ����Ÿ�ӵ� �ʱ�ȭ�Ѵ�.
					else
					{
						CObj* pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBar")->begin()));
						dynamic_cast<CMobHpBar*>(pObj)->SetRendHp(p->hp, 100);
						dynamic_cast<CMobHpBar*>(pObj)->ResetRendTime();

						pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBaisic")->begin()));
						dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"Monster", p->under_attack_id);
						dynamic_cast<CMobHpBasic*>(pObj)->ResetRendTime();
					}
					//////////////////////////////////////////////////

				}
			}

			if (0 >= p->hp)
			{
				if (p->under_attack_id < MAX_AI_NUM)
				{
					list<CObj*>* ListObj = CObjMgr::GetInstance()->Get_ObjList(L"Monster");
					list<CObj*>::iterator iter = ListObj->begin();
					list<CObj*>::iterator iter_end = ListObj->end();

					for (; iter != iter_end; ++iter)
					{
						if ((*iter)->GetPacketData()->id == p->under_attack_id)
						{
							CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
							(*iter)->m_bDeath = true;
							break;
						}
					}


					//���� ������ ü�¹ٵ� ������.
					int iSize = 0;
					iSize = ((CObjMgr::GetInstance()->Get_ObjList(L"MobHpBaisic")->size()));
					if (iSize != 0)
					{
						//��������� ���� �������� �𸣴� �׳� �����ð��� �ʰ����Ѽ� �ڿ������� ����������.
						CObj* pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBar")->begin()));
						dynamic_cast<CMobHpBar*>(pObj)->m_fRendTime = 6.f;

						pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBaisic")->begin()));
						dynamic_cast<CMobHpBasic*>(pObj)->m_fRendTime = 6.f;
					}

				}

				else
				{
					list<CObj*>* ListObj = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer");
					list<CObj*>::iterator iter = ListObj->begin();
					list<CObj*>::iterator iter_end = ListObj->end();

					for (; iter != iter_end; ++iter)
					{
						if ((*iter)->GetPacketData()->id == p->under_attack_id)
						{
							CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
							(*iter)->m_bDeath = true;
							break;
						}
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
			//�÷��̾� ���� ���� Ȥ�� �׾��� ��Ȱ������

			m_player = reinterpret_cast<sc_client_init_info *>(buf)->player_info;
			cout << "Init ID : " << m_player.id << "\n";
			(*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->SetPos(D3DXVECTOR3(m_player.pos.x, 0.f, m_player.pos.y));
			(*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->GetPacketData()->state.hp = m_player.state.hp;
			break;
		}
		case INIT_OTHER_CLIENT: {
			//�ٸ��÷��̾� or NPC or ���� ���ʰ��� Ȥ�� ��Ȱ
			sc_other_init_info *p = reinterpret_cast<sc_other_init_info *>(buf);
			cout << "init other ID : " << p->playerData.id << endl;

			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(p->playerData.id);

			if (nullptr != data) { break; }
			else {

				if(p->playerData.id < MAX_AI_NUM)
				{ 
					//cout << "���� ����" << endl;

					CObj* pObj = CMonster::Create();
					pObj->SetPos(D3DXVECTOR3(m_player.pos.x, 1.f, m_player.pos.y));
					pObj->SetPacketData(&p->playerData);
					CObjMgr::GetInstance()->AddObject(L"Monster", pObj);
					CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);

					//cout << "���� ������ ���� :" << CObjMgr::GetInstance()->m_mapObj[L"Monster"].size() << endl;
				}
				else
				{
					CObj* pObj = COtherPlayer::Create();
					pObj->SetPos(D3DXVECTOR3(m_player.pos.x, 1.f, m_player.pos.y));
					pObj->SetPacketData(&p->playerData);
					CObjMgr::GetInstance()->AddObject(L"OtherPlayer", pObj);
					CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);
				}
			}
			break;
		}
		case PLAYER_DISCONNECTED: {
			sc_disconnect *p = reinterpret_cast<sc_disconnect*>(buf);
			cout << "disconneced ID : " << p->id << endl;

			if (p->id == (*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->GetPacketData()->id) {
				// �÷��̾ �׾�����
				(*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->m_bDeath = true;


				break;
			}

			if (p->id < MAX_AI_NUM)
			{
				list<CObj*>::iterator iter = CObjMgr::GetInstance()->Get_ObjList(L"Monster")->begin();
				list<CObj*>::iterator iter_end = CObjMgr::GetInstance()->Get_ObjList(L"Monster")->end();
				//cout << "���� ����" << endl;

				for (; iter != iter_end; ++iter)
				{
					//NPC Ȥ�� ���Ͱ� �׾�����
					if ((*iter)->GetPacketData()->id == p->id)
					{
						CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
						::Safe_Delete(*iter);
						iter = CObjMgr::GetInstance()->Get_ObjList(L"Monster")->erase(iter);
						break;
					}
				}

				//cout << "���� ������ ���� :" << CObjMgr::GetInstance()->m_mapObj[L"Monster"].size() << endl;
			}
			
			else
			{
				list<CObj*>::iterator iter = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->begin();
				list<CObj*>::iterator iter_end = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->end();

				for (; iter != iter_end; ++iter)
				{
					//�ٸ��÷��̾ �׾�����
					if ((*iter)->GetPacketData()->id == p->id)
					{
						CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
						::Safe_Delete(*iter);
						iter = CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer")->erase(iter);
						break;
					}
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
