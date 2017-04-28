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

		if (true == g_database.DB_Login(m_login_id, m_login_pw)) {
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

	// �⺻ ���� �ʱ�ȭ ���� ������

	m_player_data.id = m_id;
	m_player_data.pos.x = 160;
	m_player_data.pos.y = 400;
	m_player_data.dir = 0;
	m_player_data.state.maxhp = 100;
	m_player_data.state.mp = 10;
	m_player_data.state.level = 1;
	m_player_data.state.exp = 0;
	m_player_data.state.maxgauge = 400;
	m_player_data.state.gauge = 0;
	m_player_data.state.hp = m_player_data.state.maxhp;
	m_player_data.is_ai = false;

	m_sub_status.critical = 20;	// const
	m_sub_status.def = 1 + 10;
	m_sub_status.str = 5 + 10;
	m_sub_status.agi = 2 + 10;
	m_sub_status.intel = 1 + 10;
	m_sub_status.health = 3 + 10;

	if (0 == wcscmp(L"guest", m_login_id)) {
		// guest �����̶��, �ʱ�ȭ�� ���⿡�� �����Ѵ�.
		m_player_data.id = m_id;
		m_player_data.pos.x = 160;
		m_player_data.pos.y = 400;
		m_player_data.dir = 0;
		m_player_data.state.maxhp = 100;
		m_player_data.state.mp = 10;
		m_player_data.state.level = 1;
		m_player_data.state.exp = 0;
		m_player_data.state.maxgauge = 400;
		m_player_data.state.gauge = 0;
		m_player_data.state.hp = m_player_data.state.maxhp;
		m_player_data.is_ai = false;

		m_sub_status.critical = 20;	// const
		m_sub_status.def = 1 + 10;
		m_sub_status.str = 5 + 10;
		m_sub_status.agi = 2 + 10;
		m_sub_status.intel = 1 + 10;
		m_sub_status.health = 3 + 10;
	}
	
	sc_client_init_info init_player;
	init_player.size = sizeof(sc_client_init_info);
	init_player.type = INIT_CLIENT;
	init_player.player_info = m_player_data;
	g_clients[m_id]->send_packet(reinterpret_cast<Packet*>(&init_player));

	// �ʱ�ȭ ���� ������ 2 - �� ������ �ٸ� �ֵ����� ������, �ٸ� �ֵ� ������ ������ ������
	sc_other_init_info my_info_to_other;
	sc_other_init_info other_info_to_me;

	my_info_to_other.playerData = m_player_data;

	for (auto players : g_clients)
	{
		if (DISCONNECTED == players->m_connect_state) { continue; }
		if (m_id == players->get_id()) { continue; }
		if (false == is_in_view_range(players->get_id())) { continue; }

		// view list �� �־��ֱ�
		vl_add(players->get_id());
		players->vl_add(m_id);
	}

	for (auto id : m_view_list) {
		// �ٸ� �ֵ� ������ �����ؼ� �ְ�, ������ ���� ������...
		other_info_to_me.playerData = *(g_clients[id]->get_player_data());
		send_packet(reinterpret_cast<Packet*>(&other_info_to_me));

		if (true == g_clients[id]->get_player_data()->is_ai) { continue; }	// ai �� pass
		// �� ������ ���� �ٸ� �ֵ����� ������ �Ǵµ�..
		g_clients[id]->send_packet(reinterpret_cast<Packet*>(&my_info_to_other));
	}

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
				DB ���嵵 ����
			*/

			sc_disconnect p;
			p.id = m_id;

			for (auto id : m_view_list) {				
				g_clients[id]->vl_remove(id);

				if (DISCONNECTED == g_clients[id]->m_connect_state) { continue; }
				if (true == g_clients[id]->get_player_data()->is_ai) { continue; }
				g_clients[id]->send_packet(reinterpret_cast<Packet*>(&p));
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

bool player_session::is_in_view_range(unsigned int id) {
	float x = g_clients[id]->m_player_data.pos.x;
	float y = g_clients[id]->m_player_data.pos.y;
	float my_x = m_player_data.pos.x;
	float my_y = m_player_data.pos.y;

	if ((VIEW_RANGE * VIEW_RANGE) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) { return true; }
	return false;
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

		case CHANGED_POSITION: {
			m_state = mov;
			
			m_player_data.pos = *(reinterpret_cast<position*>(&buf[2]));
			
			sc_move p;
			p.id = m_id;
			p.pos = m_player_data.pos;

			// �ʿ��� �ֵ����� �̵� ������ �ѷ�����
			for (auto players : g_clients)
			{
				if (DISCONNECTED == players->m_connect_state) { continue; }
				if (m_id == players->get_id()) { continue; }
				if (false == is_in_view_range(players->get_id())) {
					if (true == vl_find(players->get_id())) {
						players->vl_remove(m_id);
						vl_remove(players->get_id());

						sc_disconnect send_to_me;
						send_to_me.id = players->get_id();
						send_packet(reinterpret_cast<Packet*>(&send_to_me));

						if (true == players->get_player_data()->is_ai) { continue; }
						sc_disconnect send_to_other;
						send_to_other.id = m_id;
						players->send_packet(reinterpret_cast<Packet*>(&send_to_other));
					}
					continue;
				}

				// view list �� ������ skip
				if (true == vl_find(players->get_id())) { continue;	}

				vl_add(players->get_id());
				players->vl_add(m_id);

				sc_other_init_info other_player_info_to_me;
				other_player_info_to_me.playerData = *(players->get_player_data());
				send_packet(reinterpret_cast<Packet*>(&other_player_info_to_me));

				if (true == players->get_player_data()->is_ai) { continue; }
				sc_other_init_info my_info_to_other_player;
				my_info_to_other_player.playerData = m_player_data;
				players->send_packet(reinterpret_cast<Packet*>(&my_info_to_other_player));
			}

			for (auto id : m_view_list) {
				if (true == g_clients[id]->get_player_data()->is_ai) { continue; }

				g_clients[id]->send_packet(reinterpret_cast<Packet*>(&p));
			}
			break;
		}

		case CHANGED_DIRECTION: {
			
			m_player_data.dir = *(reinterpret_cast<char*>(&buf[2]));
			
			sc_dir p;
			p.id = m_id;
			p.dir = m_player_data.dir;

			for (auto id : m_view_list) {
				if (true == g_clients[id]->get_player_data()->is_ai) { continue; }

				g_clients[id]->send_packet(reinterpret_cast<Packet*>(&p));
			}
			break;
		}

		case KEYINPUT_ATTACK:		// �⺻ ���� ( ������ ���, hit box ���� ����, ���� ���⼭ �� �����ؾ� �Ѵ�. )
		{
			// ���� Ű = ���� + �Ʒ�
			// ���� Ű = ���� + ��
			// �� Ű = ���� + ��
			// �Ʒ� Ű = ���� + �Ʒ�

			// �浹üũ �˻��ϰ� �� �ڿ�..
			float att_x = 0.5, att_y = 0.5;		// �׽�Ʈ�� Ŭ�� ���� ��ġ�� ������
			float my_x = m_player_data.pos.x, my_y = m_player_data.pos.y;
			float player_size = 0.7;	// ��ü �浹 ũ�� ������
			char *dir = &m_player_data.dir;
			bool is_gauge_on = false;

			if ((*dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT)) { my_x -= att_x; my_y -= att_y; }
			if ((*dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT)) { my_x += att_x; my_y += att_y; }
			if ((*dir & KEYINPUT_UP) == (KEYINPUT_UP)) { my_x += att_x; my_y -= att_y; }
			if ((*dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN)) { my_x -= att_x; my_y += att_y; }

			for (auto id : m_view_list) {
				if (DISCONNECTED == g_clients[id]->m_connect_state) { continue; }
				//if (m_id == g_clients[id]->m_id) { continue; }
				//if (true == players->get_player_data()->is_ai) { continue; }

				float x = g_clients[id]->m_player_data.pos.x;
				float y = g_clients[id]->m_player_data.pos.y;
				if((player_size * player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
					m_state = att;
					g_clients[id]->m_player_data.state.hp -= m_sub_status.str;
					is_gauge_on = true; // �߿� �������� ������ üũ �� �÷�����

					if (false == g_clients[id]->get_hp_adding()) {
						g_clients[id]->set_hp_adding(true);
						g_time_queue.add_event(id, 1, HP_ADD, false);	// AI Ÿ�� �϶�, ���� hp �߰��� �ִ� �Լ��� ���� !!! -> �Ϲ� �÷��̾�� �����ϰ� ó����
					}

					// ���� ���� ai ��, �ݰ��� ����.
					if (MAX_AI_NUM > id) {
						if (att != g_clients[id]->m_state) {
							g_clients[id]->m_target_id = m_id;
							g_time_queue.add_event(id, 1, AI_STATE_ATT, true);
						}
						g_clients[id]->m_state = att;
					}
					
					sc_atk p;
					p.attacking_id = m_id;		// ������ id
					p.under_attack_id = id;		// �´� ���� id
					p.hp = g_clients[id]->m_player_data.state.hp;	// ���� ���� hp

					// hp �� 0 �� �Ǹ� ���ó���� �Ѵ�. -> ������ Ŭ���̾�Ʈ���� hp �� 0 �� �༮�� ������
					if (0 >= g_clients[id]->m_player_data.state.hp) {
						
						// ���� �ְ� ai �� �׳� ���� ��� ���̱�
						if (MAX_AI_NUM > g_clients[id]->get_id()) {
							g_clients[id]->m_connect_state = DISCONNECTED;

							sc_disconnect dis_p;
							dis_p.id = id;

							for (auto v_id : g_clients[id]->m_view_list) {
								if (true == g_clients[v_id]->get_player_data()->is_ai) { continue; }
								g_clients[v_id]->send_packet(reinterpret_cast<Packet*>(&dis_p));
							}
							g_clients[id]->m_view_list.clear();
							g_time_queue.add_event(g_clients[id]->m_player_data.id, 10, DEAD_TO_ALIVE, true);	// bot ����
							
							// ���� �༮���� ����ġ�� �ֵ��� ����.
						}
						else {
							// ���� �ְ� player �� ���..

						}
					}

					send_packet(reinterpret_cast<Packet*>(&p));
					if (true == g_clients[id]->get_player_data()->is_ai) { continue; }
					g_clients[id]->send_packet(reinterpret_cast<Packet*>(&p));
				}
			}

			if (true == m_player_data.is_ai) { break; }
			// �߿� �������� �ö󰡾� �Ѵٸ� ---- ( AI �� �ʿ� ������ skip )
			if (true == is_gauge_on) {
				// �߿� ������ ���� �ø���
				m_player_data.state.gauge += 10;
				if (m_player_data.state.maxgauge < m_player_data.state.gauge) { m_player_data.state.gauge = m_player_data.state.maxgauge; }

				/// ������ ������ 3�� ���� �������� �����ϵ��� �Ѵ�.
				g_time_queue.add_event(m_id, 2, CHANGE_PLAYER_STATE, false);

				// ��Ŷ�� ����ڿ��� �ϳ� ��������.
				sc_fever p;
				p.gauge = m_player_data.state.gauge;
				send_packet(reinterpret_cast<Packet*>(&p));
			}
			break;
		}
		default:
			// �� �ȳ��� ���� ��Ŷ
			switch (buf[1])
			{
			case TEST:
				// ���� ��Ŷ�� �״�� �����ش�.
				cout << "Client No. [ " << m_id << " ] TEST Packet Recived !!\n";
				printf("buf[0] = %d, buf[1] = %d, buf[2] = %d\n\n", buf[0], buf[1], buf[2]);
				send_packet(buf);
				break;

			default:
				break;
			}

			// default end
			break;
		}
	}
}
