#include"stdafx.h"

void AsynchronousClientClass::processPacket(Packet *buf)
{
	switch (buf[1])
	{
	case CHANGED_POSITION: {
		unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(*reinterpret_cast<UINT*>(&buf[sizeof(position) + 2]));
		if (m_other_players.end() == ptr) {
			player_data temp;
			temp.id = *reinterpret_cast<UINT*>(&buf[sizeof(position) + 2]);
			temp.pos = *reinterpret_cast<position*>(&buf[2]);
			m_other_players.insert(make_pair(temp.id, temp));
		}
		else { ptr->second.pos = *reinterpret_cast<position*>(&buf[2]); }
	}
		break;

	case CHANGED_DIRECTION: {
		unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(*(reinterpret_cast<UINT*>(&buf[sizeof(char) + 2])));
		//if(m_other_players.end() != ptr){ ptr->second.dir = *(reinterpret_cast<char*>(&buf[2])); }	// ptr �� nullptr �̸� �̻��� ��Ȳ�̴�...
		ptr->second.dir = *(reinterpret_cast<char*>(&buf[2]));
	}
		break;
	case SERVER_MESSAGE_HP_CHANGED: 
	//{
	//
	//	// �� hp�� ����� ���ΰ�? �׷��� ���� �� break;
	//	if (m_player.id == *(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2]))) {
	//		m_player.state.hp = *(reinterpret_cast<int*>(&buf[2]));
	//		break;
	//	}
	//
	//	// ���� �ƴ϶�� �ٸ��� hp ����
	//	unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(*(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2])));
	//	ptr->second.state.hp = *(reinterpret_cast<int*>(&buf[2]));
	//
	//
	//	// ���� hp �� 0 ���϶��, Ŭ���̾�Ʈ���� �����־�� �Ѵ�.
	//	if (0 >= *(reinterpret_cast<int*>(&buf[2]))) {
	//		// �������� ���ٸ�!!
	//		if (1 != m_other_players.erase(*(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2])))) {
	//			
	//		}
	//	}
	//}
	//	break;

	case KEYINPUT_ATTACK: {

		// ���� ���ظ� ���� ���̶��, �� hp �� ��� break;
		if (m_player.id == *(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2]))) {
			m_player.state.hp = *(reinterpret_cast<int*>(&buf[2]));
			break;
		}
		
		// ���� �ƴ϶�� �ٸ��� hp ���
		unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(*(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2])));
		ptr->second.state.hp = *(reinterpret_cast<int*>(&buf[2]));
		
		// ���� hp �� 0 ���϶��, Ŭ���̾�Ʈ���� �����־�� �Ѵ�.
		if (0 >= *(reinterpret_cast<int*>(&buf[2]))) {
			// �������� ���ٸ�!!
			if (1 != m_other_players.erase(*(reinterpret_cast<UINT*>(&buf[sizeof(int) + 2])))) {

			}			
		}
	}
		break;

	case CHANGED_INVENTORY: {
		//m_player.inven = *(reinterpret_cast<inventory*>(&buf[2]));
	}
		break;

	default:	// �� �Ⱦ��̴� ��Ŷ��

		switch (buf[1])
		{
		case INIT_OTHER_CLIENT: {
			unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(reinterpret_cast<player_data*>(&buf[2])->id);
			if (m_other_players.end() == ptr) { m_other_players.insert(make_pair(reinterpret_cast<player_data*>(&buf[2])->id, *reinterpret_cast<player_data*>(&buf[2]))); }
			else { ptr->second.pos = *reinterpret_cast<position*>(&buf[2]); }
		}
			break;
		case INIT_CLIENT: {
			m_player = *(reinterpret_cast<player_data*>(&buf[2]));
		}
			break;

		case PLAYER_DISCONNECTED: {
			unordered_map<UINT, player_data>::iterator ptr = m_other_players.find(reinterpret_cast<player_data*>(&buf[2])->id);
			if (m_other_players.end() != ptr) { m_other_players.erase(ptr->first); }
		}
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
