#include"stdafx.h"

void AsynchronousClientClass::processPacket(Packet *buf)
{
	switch (buf[1])
	{
	case CHANGED_POSITION: {
		sc_move *p = reinterpret_cast<sc_move*>(buf);

		m_other_players[p->id].pos = p->pos;

		/*unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(*reinterpret_cast<UINT*>(&buf[sizeof(position) + 2]));
		if (m_other_players.end() == ptr) {
			player_data temp;
			temp.id = *reinterpret_cast<UINT*>(&buf[sizeof(position) + 2]);
			temp.pos = *reinterpret_cast<position*>(&buf[2]);
			m_other_players.insert(make_pair(temp.id, temp));
		}
		else { ptr->second.pos = *reinterpret_cast<position*>(&buf[2]); }*/
	}
		break;

	case CHANGED_DIRECTION: {
		sc_dir *p = reinterpret_cast<sc_dir*>(buf);

		m_other_players[p->id].dir = p->dir;

		//unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(*(reinterpret_cast<UINT*>(&buf[sizeof(char) + 2])));
		////if(m_other_players.end() != ptr){ ptr->second.dir = *(reinterpret_cast<char*>(&buf[2])); }	// ptr �� nullptr �̸� �̻��� ��Ȳ�̴�...
		//ptr->second.dir = *(reinterpret_cast<char*>(&buf[2]));
	}
		break;
	case SERVER_MESSAGE_HP_CHANGED: {
		sc_hp *p = reinterpret_cast<sc_hp*>(buf);
		
		// ���� hp �� ����Ǿ��ٸ�...
		if (m_player.id == p->id) {
			m_player.state.hp = p->hp;
			break;
		}

		// ���� �ƴ� �ٸ� �༮�̶��
		m_other_players[p->id].state.hp = p->hp;

	}
		break;

	case KEYINPUT_ATTACK: {

		sc_atk *p = reinterpret_cast<sc_atk*>(buf);

		// ���� �¾Ҵٸ�.. ���� hp �� ����.
		if (m_player.id == p->under_attack_id) {
			m_player.state.hp = p->hp;
			break;
		}

		// ���� �ƴ� ������ �����Ŷ��, �ٸ� ���� hp �� �𵵷� ����.
		m_other_players[p->under_attack_id].state.hp = p->hp;
		if (0 >= p->hp) {
			// 0 ���϶�� ��������
			m_other_players.erase(p->under_attack_id);
		}

		// ���� ���ظ� ���� ���̶��, �� hp �� ��� break;
		//if (m_player.id == *(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2]))) {
		//	m_player.state.hp = *(reinterpret_cast<int*>(&buf[2]));
		//	break;
		//}
		//
		//// ���� �ƴ϶�� �ٸ��� hp ���
		//unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(*(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2])));
		//ptr->second.state.hp = *(reinterpret_cast<int*>(&buf[2]));
		//
		//// ���� hp �� 0 ���϶��, Ŭ���̾�Ʈ���� �����־�� �Ѵ�.
		//if (0 >= *(reinterpret_cast<int*>(&buf[2]))) {
		//	// �������� ���ٸ�!!
		//	if (1 != m_other_players.erase(*(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2])))) {
		//
		//	}			
		//}
	}
		break;

	default:	// �� �Ⱦ��̴� ��Ŷ��

		switch (buf[1])
		{
		case INIT_OTHER_CLIENT: {
			sc_other_init_info *p = reinterpret_cast<sc_other_init_info *>(buf);
			m_other_players[p->playerData.id] = p->playerData;
			break;
		}
		case PLAYER_DISCONNECTED: {
			sc_disconnect *p = reinterpret_cast<sc_disconnect*>(buf);
			m_other_players.erase(p->id);
		}
			break;
		case INIT_CLIENT: {
			sc_client_init_info *p = reinterpret_cast<sc_client_init_info *>(buf);
			m_player = p->player_info;
		}
			break;

			break;
		case TEST:
#ifdef _DEBUG
			cout << "Server is Running. TEST Packet Recived Successfully.\n";
#endif
			break;
		default:
			break;
		}

		// default break;
		break;
	}

	InvalidateRect(m_hWnd, NULL, TRUE);
}
