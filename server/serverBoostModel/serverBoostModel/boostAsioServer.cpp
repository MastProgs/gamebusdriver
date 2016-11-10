#pragma once
#include"stdafx.h"
using boost::asio::ip::tcp;

// �����Ҹ��� boost asio http://neive.tistory.com/22

boostAsioServer::boostAsioServer()
{
	Init();

	getMyServerIP();
	CheckThisCPUcoreCount();

	makeWorkerThreads_and_AcceptThread();
}

boostAsioServer::~boostAsioServer()
{
	// getMyServerIP
	if (nullptr != m_resolver)	delete m_resolver;
	if (nullptr != m_query)		delete m_query;

	// Init
	if (nullptr != m_endpoint)	delete m_endpoint;
	//if (nullptr != m_strand)	delete m_strand;
	if (nullptr != m_acceptor)	delete m_acceptor;
}

void boostAsioServer::getMyServerIP()
{
	// �Ʒ� while ���� ��� �׸�, ������ ip �� �����ִ� loop ���̴�.
	while (m_resolver_iterator != tcp::resolver::iterator()) {
		using boost::asio::ip::address;
		address addr = (m_resolver_iterator++)->endpoint().address();
		if (!addr.is_v6()) {
			cout << "This Server's IP address: " << addr.to_string() << endl;
		}
	}
}

void boostAsioServer::Init()
{
	// �Է� ���� host�� resolving �Ѵ� - ( ���� �Ʒ� 3���� ���� �� ��, �ʼ������� �ʱ�ȭ ���־�� �ϴ� ������ )
	m_resolver = new tcp::resolver(m_io_service);
	m_query = new tcp::resolver::query(boost::asio::ip::host_name(), "");
	m_resolver_iterator = m_resolver->resolve(*m_query);

	// endpoint �� network address ����
	m_endpoint = new tcp::endpoint(tcp::v4(), SERVERPORT);

	// strand �� �ڽ��� ���� ����ġ �Ǵ� �ڵ鷯����, �������� �ڵ鷯�� �Ϸ�Ǿ�߸� ���� �ڵ鷯�� ���۵� �� �ֵ��� �ϴ� ���� �������ݴϴ�. ( ���� �����带 ����� ���� Ŭ���� ������.. )
	//m_strand = new boost::asio::io_service::strand(m_io_service);

	// listen �� ���� acceptor �� �ʱ�ȭ
	m_acceptor = new tcp::acceptor(m_io_service, *m_endpoint);
}

void boostAsioServer::CheckThisCPUcoreCount()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	m_cpuCore = static_cast<int>(si.dwNumberOfProcessors) * 2;
	printf("CPU Core Count = %d, threads = %d\n", m_cpuCore / 2, m_cpuCore);
}

void boostAsioServer::makeWorkerThreads_and_AcceptThread()
{	
	m_worker_threads.reserve(m_cpuCore);

	for (int i = 0; i < m_cpuCore; ++i)
	{
		m_worker_threads.emplace_back(new thread{ &boostAsioServer::workerThread, this });
	}
	
	thread acceptThread{ &boostAsioServer::acceptThread, this };
	while (m_ServerShutdown) { Sleep(1000); }

	// io_service �� run() �� �ٸ� ������ �Ϸ��ϸ� �����Ѵٴµ�, ��ǻ� ���� ������ ���� �غ� �Ǿ��ִ� ���¸� ���ϴ� �� �ϴ�.
	// worker_threads ���� ���� 8�� �����Ű�� �� ����. ���⼭ �ϸ� ������
	//m_io_service.run();
	
	// workerThread �ߵ�
	for (auto thread : m_worker_threads) {
		thread->join();
		delete thread;
	}
	
	// acceptThread �ߵ�
	acceptThread.join();

}

/*
// ���� �� �Լ��� ��Ŀ������� ���� �Ǿ� ������ �ٽ� ¥���߾�� �� �� �ϴ�. ( ������ Ʋ���� �ϴ� )
void boostAsioServer::handle_accept(PLAYER_INFO* player_ptr, const boost::system::error_code& error)
{
	// error = 0 �� ��� ����, �������� ���� �÷����̴�.
	if (!error) {
		// ���⼭ �÷��̾� ���� �� ����Ǵ� �⺻ �ʱ�ȭ�� ���־�� �Ѵ�.
		cout << "Client No. " << player_ptr->getId() << " Connected :: IP = " << player_ptr->getSocket()->remote_endpoint().address().to_string() << ", Port = " << player_ptr->getSocket()->remote_endpoint().port() << "\n";
		player_ptr->setConnection(true);
		m_clients.emplace_back(player_ptr);

	}
	// �ش� �Լ��� ���� ����, acceptThread �� ���� �����̱� ������, ��� ȣ���� ���� �ʴ��� accept �Լ��� ȣ���� �ǰ� �ȴ�.
}
*/

void boostAsioServer::acceptThread()
{
	// ���⼭ �÷��̾ accept �Ͽ� ������ ���� ������ ( ���Ͽ� �� Ŭ���̾�Ʈ ���� ������ ����ִ� �� �ϴ� )
	PLAYER_INFO *tempPtr = new PLAYER_INFO(m_acceptor->get_io_service(), ++m_playerIndex);

	// ������ ���⼭ �񵿱������� �� ó���� ���� ������, �ٽ� ��� ���·�, start_accept �� �ҷ��´�.
	//m_acceptor->async_accept(*(tempPtr->getSocket()), boost::bind(&boostAsioServer::handle_accept, this, tempPtr, boost::asio::placeholders::error));
	m_acceptor->async_accept(*(tempPtr->getSocket()), [&](const boost::system::error_code& error) -> void {
		// error = 0 �� ��� ����, �������� ���� �÷����̴�.
		if (!error) {
			// ���⼭ �÷��̾� ���� �� ����Ǵ� �⺻ �ʱ�ȭ�� ���־�� �Ѵ�. ******************************************** ���⼭ ��� ������ �߻��Ѵ�... *******************
			//cout << "Client No. " << tempPtr->getId() << " Connected :: IP = " << tempPtr->getSocket()->remote_endpoint().address().to_string() << ", Port = " << tempPtr->getSocket()->remote_endpoint().port() << "\n";
			cout << "client Connected\n";
			tempPtr->setConnection(true);
			g_clients.emplace_back(tempPtr);

			// ������ �־�� �ϴ� �⺻ �ʱ�ȭ�� ���⼭ ������...


			// �񵿱� ��⸦ �ְ�, accept �� ���� ���� �ѱ��.
			tempPtr->packet_recv_from_client();
		}
		boostAsioServer::acceptThread();
	});

	// ������ Ŭ���̾�Ʈ�� �Ҵ��� tcp::socket �� �����. socket �� ���ؼ� Ŭ���̾�Ʈ �޼����� �ְ� �����Ƿ� m_io_serviec �� �Ҵ�
	// ���⿡ �ش��ϴ� iocp �� accept, �� g_hIocp = CreateIoCompletionPort(...) �κ��� ������ �ִ� �Ͱ� ����.
	//m_clients.emplace_back(new PLAYER_INFO(m_io_service, ++m_playerIndex));

	// ���� �� �κ� ������
	/*m_clients[m_playerIndex]->getSocket()->async_connect(*m_endpoint,
		boost::bind([&](const boost::system::error_code& error) {
		if (error) { cout << "connect ERROR failed : " << m_playerIndex << "player\n"; }
		else { cout << "[ No. " << m_playerIndex << " ] Client IP = " << m_clients[m_playerIndex]->getSocket()->remote_endpoint().address().to_string() << ", Port = " << m_clients[m_playerIndex]->getSocket()->remote_endpoint().port() << " is Connected\n"; }
	}, this, boost::asio::placeholders::error));*/
}

void boostAsioServer::workerThread()
{
	m_io_service.run();
}

void PLAYER_INFO::packet_recv_from_client()
{
	while (true == m_connect_state) {
		m_player_socket->async_read_some(boost::asio::buffer(m_recvBuf, MAX_BUF_SIZE), [&](const boost::system::error_code& error_code, size_t length) {
			if (error_code) {
				if (error_code == boost::asio::error::operation_aborted) { return; }
				if (false == m_connect_state) { return; }

				cout << "PLAYER_INFO::packet_recv_from_client() Error - ID [ " << m_id << " ], Error Code = " << error_code << "\n";
				m_player_socket->shutdown(m_player_socket->shutdown_both);
				m_player_socket->close();

				g_clients[m_id]->setConnection(false);
				printf("[ No. %3u ] Disconnected\n", m_id);

				/* view list ���� ������ */
				/* ��� Ŭ���̾�Ʈ����, ���� Ŭ���̾�Ʈ�� ����ٰ� �˷����� */

				/*Packet p[11];
				p[0] = 11;
				p[1] = DISCONNECTED;
				*((int *)(&p[2])) = key;*/

				//for (int i = 0; i < clients.size(); ++i) {
				//	if (false == clients[i]->connected) { continue; }
				//	//if (i == playerIndex) { continue; }

				//	SendPacket(i, p);
				//}

				return;
			}

			// ���⼭ ���� ��Ŷ ������ �Ѵ�
			int data_to_process = static_cast<int>(length);
			Packet *tempBuf = m_recvBuf_temp;
			while (0 < data_to_process) {
				if (0 == packet_size) {
					packet_size = tempBuf[0];
					if (tempBuf[0] > MAX_BUF_SIZE) {
						cout << "PLAYER_INFO::packet_recv_from_client() Error - ID [ " << m_id << " ], Packet size is over then MAX_BUF_SIZE ( " << MAX_BUF_SIZE << " Bytes )\n";
						exit(-1);
					}
				}
				int need_to_build = packet_size - previous_size;
				if (need_to_build <= data_to_process) {
					// ��Ŷ ����
					memcpy(m_recvBuf + previous_size, tempBuf, need_to_build);
					PLAYER_INFO::Process_Packet(m_recvBuf, m_id);
					packet_size = 0;
					previous_size = 0;
					data_to_process -= need_to_build;
					tempBuf += need_to_build;
				}
				else {
					// �ʳ��� ���
					memcpy(m_recvBuf + previous_size, tempBuf, data_to_process);
					previous_size += data_to_process;
					data_to_process = 0;
					tempBuf += data_to_process;
				}
			}
			//PLAYER_INFO::packet_recv_from_client();	// �� �� ���� �����ϰ�, �׳� while ���� loop �� �־���ȴ�.
		});
	}
}

//void PLAYER_INFO::packet_send_for_client(Packet *packet_ptr, size_t length)
//{
//	m_player_socket->async_write_some(boost::asio::buffer(packet_ptr, length), [&](boost::system::error_code error_code, size_t bytes_transferred) {
//		if (!error_code) {
//			if (length != bytes_transferred) {
//				cout << "Incomplete Send occured on session[" << m_id << "]. This session should be closed.\n";
//			}
//			delete[] packet_ptr;
//		}
//	});
//}

void PLAYER_INFO::Send_Packet(const Packet *packet_ptr, unsigned int id)
{
	unsigned short packet_size = packet_ptr[0];
	Packet *tempSendPacket = new Packet[packet_size];
	memset(tempSendPacket, 0, packet_size);
	memcpy(tempSendPacket, packet_ptr, packet_size);

	g_clients[id]->getSocket()->async_write_some(boost::asio::buffer(tempSendPacket, packet_size), [&](const boost::system::error_code& error_code, size_t bytes_transferred) {
		if (!error_code) {
			if (packet_size != bytes_transferred) { cout << "PLAYER_INFO::Send_Packet() Warning - ID [ " << m_id << " ], packet_data_size & bytes_transferred is not same !!\n"; }
			delete[] packet_ptr;
		}
	});
}

void PLAYER_INFO::Process_Packet(const Packet *packet, unsigned int id)
{
	// packet[0] = packet size		> 0��° �ڸ����� ������, ��Ŷ�� ũ�Ⱑ ���߸� �Ѵ�.
	// packet[1] = type				> 1��° �ڸ����� ���� ��Ŷ�� ���� ��Ŷ���� �Ӽ��� �����ִ� ���̴�.
	// packet[...] = data			> 2��° ���ʹ� �Ӽ��� �´� ����� ó���� ���ش�.

	// buf[1] ��°�� �Ӽ����� �з��� �� �ڿ�, ���ο��� 2��° ���� �����͸� ó���ϱ� �����Ѵ�.
	switch (packet[1])
	{
	case TEST:
		// ���� ��Ŷ�� �״�� �����ش�.
		printf("[ No. %3u ] TEST Packet Recived !!\n", id);
		printf("buf[0] = %d, buf[1] = %d, buf[2] = %d\n\n", packet[0], packet[1], packet[2]);
		PLAYER_INFO::Send_Packet(packet, id);
		break;
	case KEYINPUT:
		break;
	default:
		break;
	}
}
