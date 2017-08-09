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
#include "Boss.h"
#include "DamageFont.h"
#include "Info.h"
#include "MobHpBasic.h"
#include "../Include/MobHpBar.h"
#include "ChatUI.h"
#include "Font.h"
#include "QuestUI.h"
#include "Effect.h"

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
			{
				if (p->id < MAX_AI_GOBLIN)
					data = CObjMgr::GetInstance()->Get_MonsterServerData(p->id);
				else
					data = CObjMgr::GetInstance()->Get_BossServerData(p->id);
			}

			if (nullptr != data)
			{
				data->pos = p->pos;

				if (p->id < MAX_AI_NUM)
				{
					if (p->id < MAX_AI_GOBLIN)
					{
						for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"Monster"))
						{
							if (iter->GetPacketData()->id == p->id) {
								if (((CMonster*)iter)->GetAniState() == MONSTER_IDLE)
								{

									if ((fabs(((CMonster*)iter)->GetInfo()->m_vPos.x - p->pos.x) < 0.1) && (fabs(((CMonster*)iter)->GetInfo()->m_vPos.z - p->pos.y) < 0.1))
										break;
									else
									{
										((CMonster*)iter)->m_bKey = true;
										((CMonster*)iter)->SetAniState(MONSTER_IDLE);
									}
								}
								break;
							}
						}
					}
					else
					{
						auto iter = *CObjMgr::GetInstance()->Get_ObjList(L"Boss")->begin();

						if (((CBoss*)iter)->GetAniState() == BOSS_IDLE)
						{

							if ((fabs(((CBoss*)iter)->GetInfo()->m_vPos.x - p->pos.x) < 0.1) && (fabs(((CBoss*)iter)->GetInfo()->m_vPos.z - p->pos.y) < 0.1))
								break;
							else
							{
								((CBoss*)iter)->m_bKey = true;
								((CBoss*)iter)->SetAniState(BOSS_MOVE);
							}
						}
					}
				}

				else
				{
					for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer"))
					{
						if (iter->GetPacketData()->id == p->id) {
							if (((COtherPlayer*)iter)->GetAniState() == PLAYER_IDLE) 
							{

								if ((fabs(((COtherPlayer*)iter)->GetInfo()->m_vPos.x - p->pos.x) < 0.1) && (fabs(((COtherPlayer*)iter)->GetInfo()->m_vPos.z - p->pos.y) < 0.1))
									break;
								else
								{
									((COtherPlayer*)iter)->m_bKey = true;
									((COtherPlayer*)iter)->SetAniState(PLAYER_MOVE);
								}
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
			{
				if(p->id < MAX_AI_GOBLIN)
					data = CObjMgr::GetInstance()->Get_MonsterServerData(p->id);
				else
					data = CObjMgr::GetInstance()->Get_BossServerData(p->id);
			}

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

		//cout << "[ " << p->id << " ] ����� HP = " << p->hp << "\n";

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
		//cout << "[ " << p->attacking_id << " ] -> [ " << p->under_attack_id << " ] �� ������. ���� HP = " << p->hp << "\n";

		if (CSceneMgr::GetInstance()->GetScene() != SCENE_LOGO)
		{
			// ���� � ���� Į���� �ߴ��� ���̵� Ȯ�� ��, �� �༮ �ִϸ��̼� ���� ( �� ���̵� �ƴϸ� �ϴ� �ִϸ��̼� �۵� ���Ѿ� �� )
			/// ���� �� ��ġ�� �� ���� id �� ����ִٸ�, �̹� Ű�� ���� �������� �ִϸ��̼��� ����߱� ������, �׳� if ���� �Ѿ��.
			if (m_player.id != p->attacking_id) {

				if (p->attacking_id < MAX_AI_NUM)
				{
					if (p->attacking_id < MAX_AI_GOBLIN)
					{
						if (CObjMgr::GetInstance()->Get_ObjList(L"Monster") != NULL)
						{
							if (CObjMgr::GetInstance()->Get_ObjList(L"Monster")->size() != 0)
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
						}
					}
					else
					{
						if (CObjMgr::GetInstance()->Get_ObjList(L"Boss") != NULL)
						{
							if (CObjMgr::GetInstance()->Get_ObjList(L"Boss")->size() != 0)
							{
								auto iter = *CObjMgr::GetInstance()->Get_ObjList(L"Boss")->begin();
								if ((reinterpret_cast<CBoss*>(iter))->GetAniState() == BOSS_IDLE)
								{
									(reinterpret_cast<CBoss*>(iter))->m_bKey = true;
									(reinterpret_cast<CBoss*>(iter))->SetAniState(BOSS_NORMALATT);
									CObj* pObj = CEffect::Create(L"BossEffect1", L"Texture_White", D3DXVECTOR3((*iter).GetInfo()->m_vPos.x, (*iter).GetInfo()->m_vPos.y, (*iter).GetInfo()->m_vPos.z));
									pObj->GetInfo()->m_fAngle[ANGLE_Y] = 270.f;
									CObjMgr::GetInstance()->AddObject(L"Effect", pObj);
								}
							}
						}

					}
				}

				else
				{
					for (auto iter : *CObjMgr::GetInstance()->Get_ObjList(L"OtherPlayer"))
					{
						if (iter->GetPacketData()->id == p->attacking_id) {
							
								(reinterpret_cast<COtherPlayer*>(iter))->m_bKey = true;
								if (p->comboState == COMBO1)
								{
									(reinterpret_cast<COtherPlayer*>(iter))->SetAniState(PLAYER_ATT1);
								}
								else if (p->comboState == COMBO2)
								{
									(reinterpret_cast<COtherPlayer*>(iter))->SetAniState(PLAYER_ATT2);
								}
								else if (p->comboState == COMBO3)
								{
									(reinterpret_cast<COtherPlayer*>(iter))->SetAniState(PLAYER_ATT3);
								}
								else if (p->comboState == SKILL1) 
								{
									(reinterpret_cast<COtherPlayer*>(iter))->SetAniState(PLAYER_SKILL1);
								}
								else if (p->comboState == SKILL2)
								{
									(reinterpret_cast<COtherPlayer*>(iter))->SetAniState(PLAYER_SKILL2);
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
				if (NULL != (CObjMgr::GetInstance()->Get_MonsterServerData(p->under_attack_id))) 
				{

					if (p->under_attack_id < MAX_AI_GOBLIN)//�����
					{

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

						int damage_Value = (CObjMgr::GetInstance()->Get_MonsterServerData(p->under_attack_id))->state.hp - p->hp;
						if (10 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 20, 0xFFFFD200);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);
						}
						else if (20 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value,25, 0xFF00FFFF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);
						}
						else if (30 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 30, 0xFF00E4FF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);
						}
						else if (40 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 45, 0xFF00A2FF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);
						}
						else
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value,60, 0xFF0000FF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);
						}


						///////////////////////////////////////////////////

						(CObjMgr::GetInstance()->Get_MonsterServerData(p->under_attack_id))->state.hp = p->hp;

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

							if (p->under_attack_id < MAX_AI_SLIME)
								dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"������");
							else
								dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"���");

							CObjMgr::GetInstance()->AddObject(L"MobHpBaisic", pObj);
						}
						//�ִٸ� �����ͼ� �̸��� ü���� �����ϰ� ����Ÿ�ӵ� �ʱ�ȭ�Ѵ�.
						else
						{
							CObj* pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBar")->begin()));
							dynamic_cast<CMobHpBar*>(pObj)->SetRendHp(p->hp, 100);
							dynamic_cast<CMobHpBar*>(pObj)->ResetRendTime();

							pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBaisic")->begin()));

							if(p->under_attack_id < MAX_AI_SLIME)
								dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"������");
							else
								dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"���");

							dynamic_cast<CMobHpBasic*>(pObj)->ResetRendTime();
						}
						//////////////////////////////////////////////////
					}
				}
				else if (NULL != (CObjMgr::GetInstance()->Get_BossServerData(p->under_attack_id))) //�����̸�
				{
					if (p->under_attack_id >= MAX_AI_GOBLIN && p->under_attack_id < MAX_AI_BOSS)
					{
						//////////////������ ��Ʈ////////////////////////
						list<CObj*>* ListObj = CObjMgr::GetInstance()->Get_ObjList(L"Boss");
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


						int damage_Value = (CObjMgr::GetInstance()->Get_BossServerData(p->under_attack_id))->state.hp - p->hp;
						if (10 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 20, 0xFFFFD200);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);

							//pObj = CEffect::Create(L"Effect", L"Texture_Slime", D3DXVECTOR3(vPos.x,vPos.y + 2.f, vPos.z));
							//CObjMgr::GetInstance()->AddObject(L"Effect", pObj);
						}
						else if (20 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 25, 0xFF00FFFF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);

							//pObj = CEffect::Create(L"Effect", L"Texture_Slime", D3DXVECTOR3(vPos.x, vPos.y + 2.f, vPos.z));
							//CObjMgr::GetInstance()->AddObject(L"Effect", pObj);
						}
						else if (30 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 30, 0xFF00E4FF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);

							//pObj = CEffect::Create(L"Effect", L"Texture_Slime", D3DXVECTOR3(vPos.x, vPos.y + 2.f, vPos.z));
							//CObjMgr::GetInstance()->AddObject(L"Effect", pObj);
						}
						else if (40 > damage_Value)
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 45, 0xFF00A2FF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);

							//pObj = CEffect::Create(L"Effect", L"Texture_Slime", D3DXVECTOR3(vPos.x, vPos.y + 2.f, vPos.z));
							//CObjMgr::GetInstance()->AddObject(L"Effect", pObj);
						}
						else
						{
							CObj* pObj = CDamageFont::Create(&vPos, damage_Value, 60, 0xFF0000FF);
							CObjMgr::GetInstance()->AddObject(L"DamageFont", pObj);

							//pObj = CEffect::Create(L"Effect", L"Texture_Slime", D3DXVECTOR3(vPos.x, vPos.y + 2.f, vPos.z));
							//CObjMgr::GetInstance()->AddObject(L"Effect", pObj);
						}
						///////////////////////////////////////////////////

						(CObjMgr::GetInstance()->Get_BossServerData(p->under_attack_id))->state.hp = p->hp;

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
							dynamic_cast<CMobHpBar*>(pObj)->SetRendHp(p->hp, 2000); // �ִ�ü���� ����� �ٲ�����.
							CObjMgr::GetInstance()->AddObject(L"MobHpBar", pObj);

							pObj = CMobHpBasic::Create();
							dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"���� ��");
							CObjMgr::GetInstance()->AddObject(L"MobHpBaisic", pObj);
						}
						//�ִٸ� �����ͼ� �̸��� ü���� �����ϰ� ����Ÿ�ӵ� �ʱ�ȭ�Ѵ�.
						else
						{
							CObj* pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBar")->begin()));
							dynamic_cast<CMobHpBar*>(pObj)->SetRendHp(p->hp, 2000);
							dynamic_cast<CMobHpBar*>(pObj)->ResetRendTime();

							pObj = (*(CObjMgr::GetInstance()->Get_ObjList(L"MobHpBaisic")->begin()));

							dynamic_cast<CMobHpBasic*>(pObj)->SetName(L"���� ��");
							dynamic_cast<CMobHpBasic*>(pObj)->ResetRendTime();
						}
						//////////////////////////////////////////////////
					}
				}
			}

			if (0 >= p->hp)
			{
				if (p->under_attack_id < MAX_AI_NUM)
				{
					if (p->under_attack_id < MAX_AI_GOBLIN)
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
						if (nullptr != CObjMgr::GetInstance()->Get_ObjList(L"MobHpBaisic")) {
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
					}
					else
					{
						list<CObj*>* ListObj = CObjMgr::GetInstance()->Get_ObjList(L"Boss");
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
	case CHAT: {
		sc_chat *p = reinterpret_cast<sc_chat*>(buf);

		CChatUI* pChatUI = dynamic_cast<CChatUI*>(*(CObjMgr::GetInstance()->Get_ObjList(L"ChatUI")->begin()));

		
		wchar_t* str = (wchar_t*)p->msg;
		/*wchar_t *wch = new wchar_t[str.length() + 1];
		wch[str.size()] = L'\0';
		wstring wstr = L"";
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wch, (int)str.length());
		wstr = wch;*/
		
		/*cout << p->msg << endl;

		cout << str << endl;

		wcout << wstr << endl;*/

		CFont* pFont = CFont::Create(L"Font_Clear");


		wchar_t wcNick[20] = L""; 


		if ((unsigned int)(-1) != p->id) { wsprintf(wcNick, L"�÷��̾� [ %d ] : ", p->id); }
		else { wsprintf(wcNick, L"System : "); }


		wchar_t TotalChat[MAX_CHAT_SIZE];
		ZeroMemory(&TotalChat, sizeof(wchar_t) * MAX_CHAT_SIZE);
		_tcscat(TotalChat, wcNick);
		_tcscat(TotalChat, str);

		
		pFont->m_eType = FONT_TYPE_OUTLINE;
		pFont->m_wstrText = TotalChat;
		pFont->m_fSize = 20.f;
		pFont->m_nColor = 0xFFFFFFFF;
		pFont->m_nFlag = FW1_LEFT | FW1_VCENTER | FW1_RESTORESTATE;
		pFont->m_vPos = D3DXVECTOR2(15.f, 620.f);
		pFont->m_fOutlineSize = 1.f;
		pFont->m_nOutlineColor = 0xFFFFFFFF /*0xFFFFFFFF*/;

		for (auto ChatList : pChatUI->m_ChatLogList)
			ChatList->m_vPos.y -= 20.f;

		if (pChatUI->m_ChatLogList.size() == 7)
		{
			pChatUI->m_ChatLogList.pop_front(); // �տ� ��Ʈ��ü����°� �ؾ��ϴ´� �ϴ� �������ϱ� �˸�����. ���߿� ó���ؾ���
		}

		pChatUI->m_ChatLogList.push_back(pFont);


		ZeroMemory(&TotalChat, sizeof(wchar_t) * MAX_CHAT_SIZE);


		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///*
		//	ä�� ������, ���� �ڵ�
		//*/

		//// ����ٰ� �����͸� ���� ��
		//char chatting_data[MAX_CHAT_SIZE];
		//g_client.sendPacket(MAX_CHAT_SIZE, CHAT, reinterpret_cast<Packet>(chatting_data));

		//// wcahr_t �̸�
		//wchar_t chatting_data_w[MAX_CHAT_SIZE / 2];
		//// ������ chat size ���� ���� �ٿ��ߵ�. short �� ó��, 2 byte ó���ϱ� ����

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		break;
	}
	case QUEST_PROGRESS: {
		// ���⼭ ����Ʈ ���� �ؽ�Ʈ ���൵ ���̰� �ϸ��

		sc_quest *p = reinterpret_cast<sc_quest*>(buf);

		CQuestUI* pQuestUI = dynamic_cast<CQuestUI*>(*(CObjMgr::GetInstance()->Get_ObjList(L"QuestUI")->begin()));
		auto player = CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin();
		dynamic_cast<CPlayer*>(*player)->m_iQuestStateMount = p->quest;


		wchar_t wcQuestState[MAX_BUF_SIZE];

		

		if (p->quest < MAX_AI_SLIME)
		{
			pQuestUI->m_QuestScript->m_wstrText = L"�������� ��ƶ�!";
			wsprintf(wcQuestState, L"������ ���� ������ : %d / %d", p->quest, MAX_AI_SLIME);
			pQuestUI->m_QuestState->m_wstrText = wcQuestState;
		}
		else if (p->quest >= MAX_AI_SLIME && p->quest < MAX_AI_GOBLIN)
		{
			pQuestUI->m_QuestScript->m_wstrText = L"����� ��ƶ�!";
			wsprintf(wcQuestState, L"��� ���� ������ : %d / %d", p->quest - MAX_AI_SLIME, MAX_AI_GOBLIN - MAX_AI_SLIME);
			pQuestUI->m_QuestState->m_wstrText = wcQuestState;
		}
		else 
		{
			pQuestUI->m_QuestScript->m_wstrText = L"���� ���� ��ƶ�!";
			wsprintf(wcQuestState, L"������ ��ġ : %d / %d", p->quest - MAX_AI_SLIME - MAX_AI_GOBLIN, MAX_AI_BOSS);
			pQuestUI->m_QuestState->m_wstrText = wcQuestState;
		}


		/*
			�� ���� ������ 1 ���� ����, �׸��� ��� ������

			0					~ MAX_AI_SLIME ( 10 ) => ������ ���� ���� �״����
			MAX_AI_SLIME ( 10 ) ~ MAX_AI_GOBLIN ( 20 ) => ( quest - MAX_AI_SLIME ) �ؾ�, ��� ���� ���� ����
		*/

		break;
	}
	case BOSS_ATT: {
		// ���⼭�� ���� ���� ���� �ִϸ��̼Ǹ� ó���Ұ� !!
		// �⺻���� ������ ����� KEYINPUT_ATTACK �� ��Ŷ�� ���� ó���ϵ��� �صξ���.
		// ���������� ������ �˷��ٰŰ�, �𸣴°� ������, ���뼭 �����ϸ� �ڴ�� �������� �� �����

		sc_boss_atk *p = reinterpret_cast<sc_boss_atk*>(buf);
		auto iter = *CObjMgr::GetInstance()->Get_ObjList(L"Boss")->begin();

		/*BOSS_ATT,
			BOSS_ATT_01,
			BOSS_ATT_02,
			BOSS_ATT_03,
			BOSS_ATT_04,
			BOSS_ATT_05,
			BOSS_ATT_06*/


		//cout << "������Ŷ:" << (int)p->att_type << endl;

		if (p->att_type == BOSS_ATT_01)
		{
		
				(reinterpret_cast<CBoss*>(iter))->m_bKey = true;
				(reinterpret_cast<CBoss*>(iter))->SetAniState(BOSS_ATT1);
				

		}

		else if (p->att_type == BOSS_ATT_02)
		{
	
				(reinterpret_cast<CBoss*>(iter))->m_bKey = true;
				(reinterpret_cast<CBoss*>(iter))->SetAniState(BOSS_ATT2);
				

		}

		else if (p->att_type == BOSS_ATT_03)
		{
		
				(reinterpret_cast<CBoss*>(iter))->m_bKey = true;
				(reinterpret_cast<CBoss*>(iter))->SetAniState(BOSS_ATT3);
				

		}

		else if (p->att_type == BOSS_ATT_04)
		{
			
				(reinterpret_cast<CBoss*>(iter))->m_bKey = true;
				(reinterpret_cast<CBoss*>(iter))->SetAniState(BOSS_ATT4);
				
		}

		else if (p->att_type == BOSS_ATT_05)
		{

				(reinterpret_cast<CBoss*>(iter))->m_bKey = true;
				(reinterpret_cast<CBoss*>(iter))->SetAniState(BOSS_ATT5);
				

		}

		else if (p->att_type == BOSS_ATT_06)
		{
		
				(reinterpret_cast<CBoss*>(iter))->m_bKey = true;
				(reinterpret_cast<CBoss*>(iter))->SetAniState(BOSS_BUFF);

		}
		/*
			att_type �� ����, � ���� �� �ൿ���� Ȯ���� �� �ִ�.
			���� �� Ÿ�Կ� ���� ���� �ִϸ��̼� ����� �ϸ� ��.
		*/

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
			//cout << "Init ID : " << m_player.id << "\n";
			(*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->SetPos(D3DXVECTOR3(m_player.pos.x, 0.f, m_player.pos.y));
			(*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->GetPacketData()->state.hp = m_player.state.hp;
			break;
		}
		case INIT_OTHER_CLIENT: {
			//�ٸ��÷��̾� or NPC or ���� ���ʰ��� Ȥ�� ��Ȱ
			sc_other_init_info *p = reinterpret_cast<sc_other_init_info *>(buf);
			//cout << "init other ID : " << p->playerData.id << endl;

			player_data *data = CObjMgr::GetInstance()->Get_PlayerServerData(p->playerData.id);

			if (nullptr != data) { break; }
			else {

				if(p->playerData.id < MAX_AI_SLIME)
				{ 
					
					CObj* pObj = CMonster::Create(L"Slime", L"Texture_Slime");
					pObj->SetPos(D3DXVECTOR3(p->playerData.pos.x, 1.f, p->playerData.pos.y));
					pObj->SetPacketData(&p->playerData);
					CObjMgr::GetInstance()->AddObject(L"Monster", pObj);
					CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);

				}
				else if (p->playerData.id < MAX_AI_GOBLIN && p->playerData.id >= MAX_AI_SLIME)
				{
					
					CObj* pObj = CMonster::Create(L"Goblin" , L"Texture_Goblin");
					pObj->SetPos(D3DXVECTOR3(p->playerData.pos.x, 1.f, p->playerData.pos.y));
					pObj->SetPacketData(&p->playerData);
					CObjMgr::GetInstance()->AddObject(L"Monster", pObj);
					CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);
				}
				else if (p->playerData.id < MAX_AI_BOSS && p->playerData.id >= MAX_AI_GOBLIN)
				{
					CObj* pObj = CBoss::Create();
					pObj->SetPos(D3DXVECTOR3(p->playerData.pos.x, 1.f, p->playerData.pos.y));
					pObj->SetPacketData(&p->playerData);
					CObjMgr::GetInstance()->AddObject(L"Boss", pObj);
					CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);
				}
				else
				{
					CObj* pObj = COtherPlayer::Create();
					pObj->SetPos(D3DXVECTOR3(p->playerData.pos.x, 1.f, p->playerData.pos.y));
					pObj->SetPacketData(&p->playerData);
					CObjMgr::GetInstance()->AddObject(L"OtherPlayer", pObj);
					CRenderMgr::GetInstance()->AddRenderGroup(TYPE_NONEALPHA, pObj);
				}
			}
			break;
		}
		case PLAYER_DISCONNECTED: {
			sc_disconnect *p = reinterpret_cast<sc_disconnect*>(buf);
			//cout << "disconneced ID : " << p->id << endl;

			if (p->id == (*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->GetPacketData()->id) {
				// �÷��̾ �׾�����
				(*CObjMgr::GetInstance()->Get_ObjList(L"Player")->begin())->m_bDeath = true;


				break;
			}

			if (p->id < MAX_AI_GOBLIN)
			{

				if (CObjMgr::GetInstance()->Get_ObjList(L"Monster") != NULL)
				{
					if (CObjMgr::GetInstance()->Get_ObjList(L"Monster")->size() != 0)
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
					}
				}

				//cout << "���� ������ ���� :" << CObjMgr::GetInstance()->m_mapObj[L"Monster"].size() << endl;
			}
			else if (p->id >= MAX_AI_GOBLIN && p->id < MAX_AI_BOSS)
			{

				if (CObjMgr::GetInstance()->Get_ObjList(L"Boss") != NULL)
				{
					if (CObjMgr::GetInstance()->Get_ObjList(L"Boss")->size() != 0)
					{
						list<CObj*>::iterator iter = CObjMgr::GetInstance()->Get_ObjList(L"Boss")->begin();
						list<CObj*>::iterator iter_end = CObjMgr::GetInstance()->Get_ObjList(L"Boss")->end();
					//cout << p->id << " ���� ���� ����" << endl;

						for (; iter != iter_end; ++iter)
						{
							//NPC Ȥ�� ���Ͱ� �׾�����
							if ((*iter)->GetPacketData()->id == p->id)
							{
								CRenderMgr::GetInstance()->DelRenderGroup(TYPE_NONEALPHA, *iter);
								::Safe_Delete(*iter);
								iter = CObjMgr::GetInstance()->Get_ObjList(L"Boss")->erase(iter);
								break;
							}
						}
					}
				}
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
