// �����Ҹ��� boost asio http://neive.tistory.com/22

#pragma once
#include"stdafx.h"

boostAsioServer::boostAsioServer() : m_acceptor(g_io_service, tcp::endpoint(tcp::v4(), SERVERPORT)), m_socket(g_io_service)
{
	getMyServerIP();
	CheckThisCPUcoreCount();

	acceptThread();
	start_io_service();
}

boostAsioServer::~boostAsioServer()
{
	// make_shared �� ��� ������, ������ �ʿ䰡 ���پ� ?
	for (auto ptr : g_clients) { delete ptr; }
}

void boostAsioServer::getMyServerIP()
{
	tcp::resolver			m_resolver(g_io_service);
	tcp::resolver::query	m_query(boost::asio::ip::host_name(), "");
	tcp::resolver::iterator m_resolver_iterator = m_resolver.resolve(m_query);

	while (m_resolver_iterator != tcp::resolver::iterator()) {
		using boost::asio::ip::address;
		address addr = (m_resolver_iterator++)->endpoint().address();
		if (!addr.is_v6()) { cout << "This Server's IPv4 address: " << addr.to_string() << endl; }
		//else if (addr.is_v6()) { cout << "This Server's IPv6 address: " << addr.to_string() << endl; }
	}
}

void boostAsioServer::CheckThisCPUcoreCount()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_cpuCore = static_cast<int>(si.dwNumberOfProcessors) * 2;
	printf("CPU Core Count = %d, threads = %d\n", m_cpuCore / 2, m_cpuCore);
}

void boostAsioServer::start_io_service()
{	
	m_worker_threads.reserve(m_cpuCore);

	for (int i = 0; i < m_cpuCore; ++i) { m_worker_threads.emplace_back(new thread{ [&]() -> void { g_io_service.run(); } }); }
	
	while (m_ServerShutdown) { Sleep(1000); }
	
	// workerThread �ߵ�
	for (auto thread : m_worker_threads) {
		thread->join();
		delete thread;
	}
}

void boostAsioServer::acceptThread()
{
	m_acceptor.async_accept(m_socket, [&](boost::system::error_code error_code) {
		if (true == (!error_code)) {
			cout << "Client No. [ " << ++m_playerIndex << " ] Connected \t:: IP = " << m_socket.remote_endpoint().address().to_string() << ", Port = " << m_socket.remote_endpoint().port() << "\n";
			g_clients.emplace_back(new player_session (std::move(m_socket), m_playerIndex));
			g_clients[m_playerIndex]->Init();
		}
		if (false == m_ServerShutdown) { acceptThread(); }		
	});
}


// player_session class ------------------------------------------------------------------------------------------------------------------------

void player_session::Init()
{
	m_connect_state = true;

	// �⺻ ���� �ʱ�ȭ ���� ������
	Packet init_this_player_buf[MAX_BUF_SIZE];

	init_this_player_buf[0] = sizeof(player_data) + 2;
	init_this_player_buf[1] = INIT_CLIENT;

	m_player_data.id = m_id;
	m_player_data.pos.x = 400;
	m_player_data.pos.y = 300;

	memcpy(&init_this_player_buf[2], g_clients[m_id]->get_player_data(), init_this_player_buf[0]);
	g_clients[m_id]->send_packet(init_this_player_buf);

	// �ʱ�ȭ ���� ������ 2 - �� ������ �ٸ� �ֵ����� ������, �ٸ� �ֵ� ������ ������ ������
	/*Packet other_info_to_me_buf[MAX_BUF_SIZE];*/
	Packet my_info_to_other_buf[MAX_BUF_SIZE];

	/*other_info_to_me_buf[0] =*/ my_info_to_other_buf[0] = sizeof(player_data) + 2;
	/*other_info_to_me_buf[1] =*/ my_info_to_other_buf[1] = KEYINPUT;
	my_info_to_other_buf[1] = INIT_CLIENT;

	// ���� ������ ������ �ٸ� �÷��̾� ���� ������
	memcpy(&my_info_to_other_buf[2], &m_player_data, my_info_to_other_buf[0]);

	for (auto players : g_clients)
	{
		if (DISCONNECTED == players->get_current_connect_state()) { continue; }
		if (m_id == players->get_id()) { continue; }

		// �ٸ� �ֵ� ������ �����ؼ� �ְ�, ������ ���� ������... ( �� �� ���޴°��� �˼��� ���� ����ü... )
		/*memcpy(&other_info_to_me_buf[2], players->get_player_data(), other_info_to_me_buf[0] - 2);
		send_packet(other_info_to_me_buf);*/

		// �� ������ ���� �ٸ� �ֵ����� ������ �Ǵµ�..
		players->send_packet(my_info_to_other_buf);
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

			Packet temp_buf[MAX_BUF_SIZE] = { sizeof(player_data) + 2, PLAYER_DISCONNECTED };
			memcpy(&temp_buf[2], &m_player_data, temp_buf[0]);

			for (auto players : g_clients)
			{
				if (DISCONNECTED == players->m_connect_state) { continue; }
				if (m_id == players->m_id) { continue; }

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

				m_process_packet(m_id, m_data_buf);

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

void player_session::m_process_packet(const unsigned int& id, Packet buf[])
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
			g_clients[id]->send_packet(buf);
			break;

		case KEYINPUT:
			g_clients[id]->m_player_data.pos = reinterpret_cast<player_data*>(&buf[2])->pos;

			// �ʿ��� �ֵ����� �̵� ������ �ѷ�����
			for (auto players : g_clients)
			{
				if (DISCONNECTED == players->m_connect_state) { continue; }
				if (id == players->m_id) { continue; }

				players->send_packet(buf);
			}

			break;
		default:
			break;
		}
	}
}