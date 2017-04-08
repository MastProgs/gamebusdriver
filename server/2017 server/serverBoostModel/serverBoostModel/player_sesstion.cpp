#include "stdafx.h"

bool player_session::check_login() {

	m_connect_state = true;

	int login_cnt{ 2 };
	while (--login_cnt)
	{
		// id size + id + pw size + pw ( id ���ڿ� + pw ���ڿ� + 2 )
		Packet temp_buf[MAX_BUF_SIZE]{ 0 };
		m_socket.receive(boost::asio::buffer(temp_buf, MAX_BUF_SIZE));

		wcscpy(m_login_id, reinterpret_cast<wchar_t*>(&temp_buf[1]));
		wcscpy(m_login_pw, reinterpret_cast<wchar_t*>(&temp_buf[temp_buf[0] + 4]));

		// �׳� guest ���, DB ���� ���� �׳� ok...
		if ((0 == wcscmp(L"guest", m_login_id)) && (0 == wcscmp(L"guest", m_login_pw))) {
			temp_buf[0] = 1;
			m_socket.send(boost::asio::buffer(temp_buf, MAX_BUF_SIZE));
			return true;
		}

		if (true == database.DB_Login(m_login_id, m_login_pw)) {
			// �α��� ���� �� ���⼭ �÷��̾� ������ �ҷ��ͼ� �Է�

			/// ( �����ߴٰ� Ŭ������ �޼��� ���� )
			temp_buf[0] = 1;
			m_socket.send(boost::asio::buffer(temp_buf, MAX_BUF_SIZE));
			return true;
		}
		else {
			// �α��� ���� ��, ���⼭ ó��
			/// ( �����ߴٰ� Ŭ������ �޼��� ���� )
			temp_buf[0] = 0;
			m_socket.send(boost::asio::buffer(temp_buf, MAX_BUF_SIZE));
		}
	}

	cout << "Client No. [ " << m_id << " ] Disonnected \t:: IP = " << m_socket.remote_endpoint().address().to_string() << ", Port = " << m_socket.remote_endpoint().port() << "\n";
	cout << "Reason = Login Failed\n";
	m_socket.shutdown(m_socket.shutdown_both);
	m_socket.close();
	m_connect_state = false;
	return false;
}

void player_session::Init()
{
	m_connect_state = true;

	// �⺻ ���� �ʱ�ȭ ���� ������ *****************>>>> player_data �ʱ�ȭ�� ���� �⺻ �ʿ� ���� ������, ���⼭ �ؾ���.

	m_player_data.id = m_id;
	m_player_data.pos.x = 100;
	m_player_data.pos.y = 100;
	m_player_data.dir = 0;
	m_player_data.state.maxhp = 100;
	m_player_data.state.mp = 10;
	m_player_data.state.level = 1;
	m_player_data.state.exp = 0;
	m_player_data.state.critical = 20;	// const
	m_player_data.state.def = 1;
	m_player_data.state.str = 5;
	m_player_data.state.agi = 2;
	m_player_data.state.intel = 1;
	m_player_data.state.health = 3;
	m_player_data.state.gauge = 0;
	m_player_data.state.hp = m_player_data.state.maxhp;
	m_player_data.is_ai = false;

	if (0 == wcscmp(L"guest", m_login_id)) {
		// guest �����̶��, �ʱ�ȭ�� ���⿡�� �����Ѵ�.
		m_player_data.id = m_id;
		m_player_data.pos.x = 100;
		m_player_data.pos.y = 100;
		m_player_data.dir = 0;
		m_player_data.state.maxhp = 5;
		m_player_data.state.mp = 10;
		m_player_data.state.level = 2;
		m_player_data.state.exp = 0;
		m_player_data.state.critical = 20;	// const
		m_player_data.state.def = 1;
		m_player_data.state.str = 5;
		m_player_data.state.agi = 2;
		m_player_data.state.intel = 1;
		m_player_data.state.health = 3;
		m_player_data.state.gauge = 0;
		m_player_data.state.hp = m_player_data.state.maxhp;
		m_player_data.is_ai = false;
	}

	sc_client_init_info init_player;
	init_player.size = sizeof(sc_client_init_info);
	init_player.type = INIT_CLIENT;
	init_player.player_info = m_player_data;
	g_clients[m_id]->send_packet(reinterpret_cast<Packet*>(&init_player));

	// �ʱ�ȭ ���� ������ 2 - �� ������ �ٸ� �ֵ����� ������, �ٸ� �ֵ� ������ ������ ������  *****************>>>> player_data ���� �߰��Ǵ� ������ ���� ��, �������־�� �Ѵ�.
	sc_other_init_info my_info_to_other;
	sc_other_init_info other_info_to_me;

	my_info_to_other.id = m_player_data.id;
	my_info_to_other.hp = m_player_data.state.hp;
	my_info_to_other.pos = m_player_data.pos;

	for (auto players : g_clients)
	{
		if (DISCONNECTED == players->get_current_connect_state()) { continue; }
		if (m_id == players->get_id()) { continue; }

		// �ٸ� �ֵ� ������ �����ؼ� �ְ�, ������ ���� ������...
		other_info_to_me.id = (players->get_player_data())->id;
		other_info_to_me.hp = (players->get_player_data())->state.hp;
		other_info_to_me.pos = (players->get_player_data())->pos;
		send_packet(reinterpret_cast<Packet*>(&other_info_to_me));

		if (true == players->get_player_data()->is_ai) { continue; }
		// �� ������ ���� �ٸ� �ֵ����� ������ �Ǵµ�..
		players->send_packet(reinterpret_cast<Packet*>(&my_info_to_other));
	}

	/*
	��ó �÷��̾��, ���� �÷��̾��� ������ �˸���
	view list ���� �������� �߰����� ~ !!
	*/

	m_recv_packet();
}

void player_session::m_recv_packet()
{
	//auto self(shared_from_this());
	m_socket.async_read_some(boost::asio::buffer(m_recv_buf, MAX_BUF_SIZE), [&](boost::system::error_code error_code, std::size_t length) -> void {
		if (error_code) {
			if (error_code.value() == boost::asio::error::operation_aborted) { return; }
			// client was disconnected
			if (false == g_clients[m_id]->get_current_connect_state()) { return; }

			cout << "Client No. [ " << m_id << " ] Disonnected \t:: IP = " << m_socket.remote_endpoint().address().to_string() << ", Port = " << m_socket.remote_endpoint().port() << "\n";
			m_socket.shutdown(m_socket.shutdown_both);
			m_socket.close();

			m_connect_state = false;

			/*
			��ó �÷��̾��, ���� �÷��̾��� ������ �˸���
			view list ���� �������� ������ ~ !!
			*/

			sc_disconnect p;
			p.id = m_id;

			for (auto players : g_clients)
			{
				if (DISCONNECTED == players->m_connect_state) { continue; }
				if (m_id == players->m_id) { continue; }
				//if (true == players->get_player_data()->is_ai) { continue; }

				players->send_packet(temp_buf);
			}

			return;
		}

		int current_data_processing = static_cast<int>(length);
		Packet *buf = m_recv_buf;
		while (0 < current_data_processing) {
			if (0 == m_packet_size_current) {
				m_packet_size_current = buf[0];
				if (buf[0] > MAX_BUF_SIZE) {
					cout << "player_session::m_recv_packet() Error, Client No. [ " << m_id << " ] recv buf[0] is out of MAX_BUF_SIZE\n";
					exit(-1);
				}
			}
			int need_to_build = m_packet_size_current - m_packet_size_previous;
			if (need_to_build <= current_data_processing) {
				// Packet building Complete & Process Packet
				memcpy(m_data_buf + m_packet_size_previous, buf, need_to_build);

				m_process_packet(m_data_buf);

				m_packet_size_current = 0;
				m_packet_size_previous = 0;
				current_data_processing -= need_to_build;
				buf += need_to_build;
			}
			else {
				// Packet build continue
				memcpy(m_data_buf + m_packet_size_previous, buf, current_data_processing);
				m_packet_size_previous += current_data_processing;
				current_data_processing = 0;
				buf += current_data_processing;
			}
		}
		m_recv_packet();
	});
}

void player_session::send_packet(Packet *packet)
{
	int packet_size = packet[0];
	Packet *sendBuf = new Packet[packet_size];
	memcpy(sendBuf, packet, packet_size);

	//auto self(shared_from_this());
	m_socket.async_write_some(boost::asio::buffer(sendBuf, packet_size), [=](boost::system::error_code error_code, std::size_t bytes_transferred) -> void {
		if (!error_code) {
			if (packet_size != bytes_transferred) { cout << "Client No. [ " << m_id << " ] async_write_some packet bytes was NOT SAME !!\n"; }
			delete[] sendBuf;
		}
	});
}

void player_session::m_process_packet(Packet buf[])
{
	// packet[0] = packet size		> 0��° �ڸ����� ������, ��Ŷ�� ũ�Ⱑ ���߸� �Ѵ�.
	// packet[1] = type				> 1��° �ڸ����� ���� ��Ŷ�� ���� ��Ŷ���� �Ӽ��� �����ִ� ���̴�.
	// packet[...] = data			> 2��° ���ʹ� �Ӽ��� �´� ����� ó���� ���ش�.

	// buf[1] ��°�� �Ӽ����� �з��� �� �ڿ�, ���ο��� 2��° ���� �����͸� ó���ϱ� �����Ѵ�.

	{
		switch (buf[1])
		{
		case TEST:
			// ���� ��Ŷ�� �״�� �����ش�.
			cout << "Client No. [ " << m_id << " ] TEST Packet Recived !!\n";
			printf("buf[0] = %d, buf[1] = %d, buf[2] = %d\n\n", buf[0], buf[1], buf[2]);
			send_packet(buf);
			break;

		case CHANGED_POSITION: {

			// [ 0] = size
			// [ 1] = CHANGED_POSITION type
			// [ 2] = size of position
			// [10] = user id

			m_player_data.pos = *(reinterpret_cast<position*>(&buf[2]));

			Packet temp_pos_buf[MAX_BUF_SIZE]{ 0 };
			temp_pos_buf[0] = sizeof(position) + sizeof(unsigned int) + 2;
			temp_pos_buf[1] = CHANGED_POSITION;
			*(reinterpret_cast<position*>(&temp_pos_buf[2])) = m_player_data.pos;
			*(reinterpret_cast<unsigned int*>(&temp_pos_buf[sizeof(position) + 2])) = m_id;

			// �ʿ��� �ֵ����� �̵� ������ �ѷ����� - ����� �ֵ� �� �Ѹ���.
			for (auto players : g_clients)
			{
				if (DISCONNECTED == players->m_connect_state) { continue; }
				if (m_id == players->m_id) { continue; }
				if (true == players->get_player_data()->is_ai) { continue; }

				players->send_packet(temp_pos_buf);
			}
		}
							   break;

		case CHANGED_DIRECTION: {

			// [0] = size
			// [1] = CHANGED_DIRECTION type
			// [2] = size of direction
			// [3] = user id

			m_player_data.dir = *(reinterpret_cast<char*>(&buf[2]));

			Packet temp_direction_buf[MAX_BUF_SIZE]{ 0 };
			temp_direction_buf[0] = sizeof(char) + sizeof(unsigned int) + 2;
			temp_direction_buf[1] = CHANGED_DIRECTION;
			*(reinterpret_cast<char*>(&temp_direction_buf[2])) = m_player_data.dir;
			*(reinterpret_cast<unsigned int*>(&temp_direction_buf[sizeof(char) + 2])) = m_id;

			// �ʿ��� �ֵ����� ���� ������ �ѷ����� - ����� �ֵ� �� �Ѹ���.
			for (auto players : g_clients)
			{
				if (DISCONNECTED == players->m_connect_state) { continue; }
				if (m_id == players->m_id) { continue; }
				if (true == players->get_player_data()->is_ai) { continue; }

				players->send_packet(temp_direction_buf);
			}
		}
								break;

		case KEYINPUT_ATTACK:		// �⺻ ���� ( ������ ���, hit box ���� ����, ���� ���⼭ �� �����ؾ� �Ѵ�. )
		{
			// �浹üũ �˻��ϰ� �� �ڿ�.. ( ����� �ӽ� �浹 üũ, ���� Ŭ��� ������ �浹 ���� üũ�ؾ� �Ѵ�. )
			int att_x = 0.1, att_y = 0.1;		// �׽�Ʈ�� Ŭ�� ���� ��ġ�� ������
			int x = m_player_data.pos.x, y = m_player_data.pos.y;
			int player_size = 1;	// �׽�Ʈ�� Ŭ�� �� �������� ũ�� 5...
			char *dir = &m_player_data.dir;

			if ((*dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT)) { x += att_x; }
			if ((*dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT)) { x -= att_x; }
			if ((*dir & KEYINPUT_UP) == (KEYINPUT_UP)) { y -= att_y; }
			if ((*dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN)) { y += att_y; }

			for (auto players : g_clients) {
				if (DISCONNECTED == players->m_connect_state) { continue; }
				if (m_id == players->m_id) { continue; }
				//if (true == players->get_player_data()->is_ai) { continue; }

				int tempx = x - players->m_player_data.pos.x;
				int tempy = y - players->m_player_data.pos.y;
				if (((tempx * tempx) + (tempy * tempy)) <= (player_size * player_size)) {
					players->m_player_data.state.hp -= 10;

					if (false == *players->get_hp_adding()) {
						*players->get_hp_adding() = true;
						time_queue.add_event(players->m_player_data.id, 1, HP_ADD, false);	// AI Ÿ�� �϶�, ���� hp �߰��� �ִ� �Լ��� ���� !!! -> �Ϲ� �÷��̾�� �����ϰ� ó����
					}

					Packet temp_hp_buf[MAX_BUF_SIZE]{ 0 };
					temp_hp_buf[0] = sizeof(int) + sizeof(UINT) + sizeof(UINT) + 2;	// hp + id + packet size addition(2)
					temp_hp_buf[1] = KEYINPUT_ATTACK;
					*(reinterpret_cast<int*>(&temp_hp_buf[2])) = players->m_player_data.state.hp;
					*(reinterpret_cast<int*>(&temp_hp_buf[6])) = players->m_id;		// �´� ����� id
					*(reinterpret_cast<int*>(&temp_hp_buf[10])) = m_id;				// ������ ����� id

																					// hp �� 0 �� �Ǹ� ���ó���� �Ѵ�. -> ������ Ŭ���̾�Ʈ���� hp �� 0 �� �༮�� ��������...
					if (0 >= players->m_player_data.state.hp) {
						//*(reinterpret_cast<int*>(&temp_hp_buf[2])) = players->m_player_data.state.hp = 0; // ���� 0 ���� ������� �ʿ�� ���µ�

						// ���� �ְ� ai �� �׳� ���� ��� ���̱�
						if (MAX_AI_NUM > players->get_id()) {
							players->m_connect_state = DISCONNECTED;

							// 10 ���� ������ �ϴ� Ÿ�̸� ť�� ����ִ°� ��� �ʹ�.

							// ai ���ϰ��, ������ �׳� �Է��ؼ� �ֱ� ( ���� ����Ŷ�, �ٽ� �����ؾ� �Ѵ� ) => ��Ŷ �����°�, �׳� ��ü �κ��丮 �� ��������.. �̰͵� ���� �ʿ�
							/*if (NONE == m_player_data.inven.head) { m_player_data.inven.head = BASIC_HEAD; }
							else if (NONE == m_player_data.inven.body) { m_player_data.inven.body = BASIC_BODY; }
							else if (NONE == m_player_data.inven.arm) { m_player_data.inven.arm = BASIC_ARM; }
							else if (NONE == m_player_data.inven.weapon) { m_player_data.inven.weapon = BASIC_WEAPON; }

							Packet temp_buf_inventory[MAX_BUF_SIZE]{ 0 };
							temp_buf_inventory[0] = sizeof(inventory) + 2;
							temp_buf_inventory[1] = CHANGED_INVENTORY;
							*(reinterpret_cast<inventory*>(&temp_buf_inventory[2])) = m_player_data.inven;

							send_packet(temp_buf_inventory);*/
						}
					}

					for (auto other_players : g_clients) {
						if (DISCONNECTED == other_players->m_connect_state) { continue; }
						//if (players->m_id == other_players->m_id) { continue; }	// �ڱ� hp �� ���� ���, �ڱ� ���׵� ������ �Ѵ�...
						if (true == other_players->get_player_data()->is_ai) { continue; }

						other_players->send_packet(temp_hp_buf);
					}
				}
			}

		}
		break;
		default:
			break;
		}
	}
}
