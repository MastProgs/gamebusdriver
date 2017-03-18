// �����Ҹ��� boost asio http://neive.tistory.com/22

#pragma once
#include"stdafx.h"

boostAsioServer::boostAsioServer() : m_acceptor(g_io_service, tcp::endpoint(tcp::v4(), SERVERPORT)), m_socket(g_io_service)
{
	getMyServerIP();
	CheckThisCPUcoreCount();

	// DB SQL ������ ����
	database.Init();

	// ai �� �ʱ�ȭ
	g_client_init();

	acceptThread();
	start_io_service();
}

boostAsioServer::~boostAsioServer()
{
	// make_shared �� ����, ������ �ʿ䰡 ������, ��Ƽ������ �浹�� ���� ���� ���� �Ϲ� vector �� �����.. ( ���� ������ �ڷᱸ���� ������ �Ѵ� )
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
	// DB ������ ���� -1
	// Timer Thread �� ���� -1
	m_worker_threads.reserve(m_cpuCore - 2);
	for (int i = 0; i < m_cpuCore - 2; ++i) { m_worker_threads.emplace_back(new thread{ [&]() -> void { g_io_service.run(); } }); }
	m_worker_threads.emplace_back(new thread{ [&]() -> void { time_queue.TimerThread(); } });
	
	while (m_ServerShutdown) { Sleep(1000); }
	
	// workerThread �ߵ�
	for (auto thread : m_worker_threads) {
		thread->join();
		delete thread;
	}
}

void boostAsioServer::g_client_init() {
	
	g_clients.reserve(MAX_AI_NUM + 1000);
	
	for (auto i = 0; i < MAX_AI_NUM; ++i) {
		g_clients.emplace_back(new player_session(boost::asio::ip::tcp::socket (g_io_service), ++m_playerIndex));
		g_clients[i]->get_player_data()->id = m_playerIndex;
		g_clients[i]->get_player_data()->is_ai = true;
		g_clients[i]->get_player_data()->dir = KEYINPUT_UP;
		g_clients[i]->get_player_data()->pos.x = rand() % 500;
		g_clients[i]->get_player_data()->pos.y = rand() % 500;
		g_clients[i]->get_player_data()->state.hp = MAX_HP;
	}

	cout << "\nAI bots created number of " << MAX_AI_NUM << ", Compelete\n";
}

void boostAsioServer::acceptThread()
{
	m_acceptor.async_accept(m_socket, [&](boost::system::error_code error_code) {
		if (true == (!error_code)) {
			cout << "Client No. [ " << ++m_playerIndex << " ] Connected \t:: IP = " << m_socket.remote_endpoint().address().to_string() << ", Port = " << m_socket.remote_endpoint().port() << "\n";
			g_clients.emplace_back(new player_session (std::move(m_socket), m_playerIndex));
			if (false != g_clients[m_playerIndex]->check_login()) { g_clients[m_playerIndex]->Init(); }
			
		}
		if (false == m_ServerShutdown) { acceptThread(); }		
	});
}


// player_session class ------------------------------------------------------------------------------------------------------------------------

bool player_session::check_login() {

	m_connect_state = true;

	int login_cnt{ 6 };
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
	Packet init_this_player_buf[MAX_BUF_SIZE];

	init_this_player_buf[0] = sizeof(player_data) + 2;
	init_this_player_buf[1] = INIT_CLIENT;

	if (0 == wcscmp(L"guest", m_login_id)) {
		// guest �����̶��, �ʱ�ȭ�� ���⿡�� �����Ѵ�.
		m_player_data.id = m_id;
		m_player_data.pos.x = 100;
		m_player_data.pos.y = 100;
		m_player_data.dir = 0;
		m_player_data.state.hp = MAX_HP;
		m_player_data.is_ai = false;
	}
	m_player_data.id = m_id;
	m_player_data.pos.x = 100;
	m_player_data.pos.y = 100;
	m_player_data.dir = 0;
	m_player_data.state.hp = MAX_HP;
	m_player_data.is_ai = false;

	*(reinterpret_cast<player_data*>(&init_this_player_buf[2])) = m_player_data;
	g_clients[m_id]->send_packet(init_this_player_buf);
	
	// �ʱ�ȭ ���� ������ 2 - �� ������ �ٸ� �ֵ����� ������, �ٸ� �ֵ� ������ ������ ������  *****************>>>> player_data ���� �߰��Ǵ� ������ ���� ��, �������־�� �Ѵ�.
	Packet other_info_to_me_buf[MAX_BUF_SIZE];
	Packet my_info_to_other_buf[MAX_BUF_SIZE];

	other_info_to_me_buf[0] = my_info_to_other_buf[0] = sizeof(player_data) + 2;
	other_info_to_me_buf[1] = my_info_to_other_buf[1] = INIT_OTHER_CLIENT;

	// ���� ������ ������ �ٸ� �÷��̾� ���� ������
	*(reinterpret_cast<player_data*>(&my_info_to_other_buf[2])) = m_player_data;

	for (auto players : g_clients)
	{
		if (DISCONNECTED == players->get_current_connect_state()) { continue; }
		if (m_id == players->get_id()) { continue; }

		// �ٸ� �ֵ� ������ �����ؼ� �ְ�, ������ ���� ������...
		*(reinterpret_cast<player_data*>(&other_info_to_me_buf[2])) = *(players->get_player_data());
		send_packet(other_info_to_me_buf);

		if (true == players->get_player_data()->is_ai) { continue; }
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
			int att_x = 3, att_y = 3;		// �׽�Ʈ�� Ŭ�� ���� ��ġ�� ������
			int x = m_player_data.pos.x, y = m_player_data.pos.y;
			int player_size = 50;	// �׽�Ʈ�� Ŭ�� �� �������� ũ�� 5...
			char *dir = &m_player_data.dir;

			if ((*dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT))	{ x += att_x; }
			if ((*dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT))		{ x -= att_x; }
			if ((*dir & KEYINPUT_UP) == (KEYINPUT_UP))			{ y -= att_y; }
			if ((*dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN))		{ y += att_y; }

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

// DB class ---------------------------------------------------------------------------------------------------------------

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT *)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) { fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError); }
	}

}

void DB::Init() {
	// Allocate an environment

	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR) {
		fwprintf(stderr, L"DBacess class :: Unable to allocate an environment handle\n");
		exit(-1);
	}

	TRYODBC(hEnv, SQL_HANDLE_ENV, SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0));
	TRYODBC(hEnv, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc));
	TRYODBC(hDbc, SQL_HANDLE_DBC, SQLDriverConnect(hDbc, GetDesktopWindow(), SQLSERVERADDR, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE));
	TRYODBC(hDbc, SQL_HANDLE_DBC, SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt));

	fwprintf(stderr, L"Azure SQL Database Server Connected!\n");
	// Loop to get input and execute queries -- ������ ������� �Ϸ� �Ǿ���, ���⼭ ���� ��ɾ ������ �����ϰ� �ȴ�.
}

void DB::Release() {
	// Free ODBC handles and exit

	if (hStmt) { SQLFreeHandle(SQL_HANDLE_STMT, hStmt); }

	if (hDbc) {
		SQLDisconnect(hDbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	}

	if (hEnv) { SQLFreeHandle(SQL_HANDLE_ENV, hEnv); }

	wprintf(L"\nAzure SQL DataBase Server Disconnected.\n");
}

bool DB::DB_Login(wchar_t* id, wchar_t* pw) {
	wchar_t input_id[MAX_BUF_SIZE / 4]{ 0 };
	wchar_t input_pw[MAX_BUF_SIZE / 4]{ 0 };
	wchar_t Nickname[MAX_BUF_SIZE / 4]{ 0 };
	int PlayerLevel{ 0 };
	bool Admin{ false };

	SQLINTEGER pIndicators[5];		// DB �� �ȿ� �ִ� ���� �� ��

	if (SQLExecDirect(hStmt, (SQLWCHAR*)L"SELECT RTRIM(ID), RTRIM(Password), RTRIM(Nickname), RTRIM(PlayerLevel), RTRIM(Admin) FROM dbo.user_data", SQL_NTS) != SQL_ERROR) {
		// ���� 1 ��, ù��° �Ķ���� ID
		// �ڿ� �迭�� �� ����Ʈ �޾Ҵ��� ����
		SQLBindCol(hStmt, 1, SQL_C_WCHAR, (SQLPOINTER)&input_id, MAX_BUF_SIZE / 4, &pIndicators[0]);
		SQLBindCol(hStmt, 2, SQL_C_WCHAR, (SQLPOINTER)&input_pw, MAX_BUF_SIZE / 4, &pIndicators[1]);
		SQLBindCol(hStmt, 3, SQL_C_WCHAR, (SQLPOINTER)&Nickname, MAX_BUF_SIZE / 4, &pIndicators[2]);
		SQLBindCol(hStmt, 4, SQL_C_LONG, (SQLPOINTER)&PlayerLevel, sizeof(PlayerLevel), &pIndicators[3]);
		SQLBindCol(hStmt, 5, SQL_C_BIT, (SQLPOINTER)&Admin, sizeof(Admin), &pIndicators[4]);

		while (SQLFetch(hStmt) == SQL_SUCCESS)
		{
			if ((wcscmp(input_id, id) == 0) && (wcscmp(input_pw, pw) == 0)) {

				// Ȯ���� �Ǿ��ٸ�, ���⼭ �����͸� �����ؾ� �ϴµ�..

				return true;
			}
		}
	}

	return false;
}

void DB::SQLcmd(SQLWCHAR* str) {

	RETCODE     RetCode;
	SQLSMALLINT sNumResults;

	RetCode = SQLExecDirect(hStmt, str, SQL_NTS);

	switch (RetCode)
	{
	case SQL_SUCCESS_WITH_INFO:	// ���� ���д� �ߴµ�, �ϴ� ��� ���� �� �� ���� ���...
	{
		HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
	}
	case SQL_SUCCESS:
	{
		// �������� ��... �ڵ� ����
		// SQLNumResultCols() �Լ��� ��� ��Ʈ�� �ش� �� ����, ������ ���� �޴´�. ( �� = -> )
		TRYODBC(hStmt, SQL_HANDLE_STMT, SQLNumResultCols(hStmt, &sNumResults));

		// ����� 0�� �̻��̸� ���� ó���Ѵ�. ��߲��...
		if (sNumResults > 0) {

		}
		else
		{
			SQLINTEGER cRowCount;

			// �ݴ�� SQLRowCount() �Լ��� �� ����, ������ ���� �޴´�. ( �� = V )
			TRYODBC(hStmt, SQL_HANDLE_STMT, SQLRowCount(hStmt, reinterpret_cast<SQLLEN *>(&cRowCount)));

			// ���� 0 �̻��̸� �����͸� ����ϴ� ����...
			if (cRowCount >= 0) {
				wprintf(L"%Id %s affected\n", cRowCount, cRowCount == 1 ? L"row" : L"rows");
			}
		}
		break;
	}
	case SQL_ERROR: // ���� ���� ���, �Ʒ� �Լ��� ���ؼ� ���� ����� ȭ�鿡 ������ش�.
	{
		HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
	}
	default:
		fwprintf(stderr, L"Unexpected return code %hd!\n", RetCode);
	}


}


// Timer class ---------------------------------------------------------------------------------------------------------------

void TimerQueue::TimerThread() {
	while (true) {
		Sleep(1);
		time_lock.lock();
		while (false == timer_queue.empty()) {
			if (timer_queue.top()->wakeup_time > GetTickCount()) { break; }
			
			event_type *event_ptr = timer_queue.top();
			
			timer_queue.pop();
			time_lock.unlock();

			processPacket(event_ptr);
			if (event_ptr != nullptr) { delete event_ptr; }

			time_lock.lock();
		}
		time_lock.unlock();
	}
}

void TimerQueue::add_event(const unsigned int& id, const int& sec, time_queue_event type, bool is_ai) {

	event_type *ptr = new event_type;

	ptr->obj_id = id;
	ptr->wakeup_time = GetTickCount() + (sec * 1000);
	ptr->event_id = type;
	ptr->is_ai = is_ai;

	time_lock.lock();
	timer_queue.push(ptr);
	time_lock.unlock();
}

void TimerQueue::processPacket(event_type *p) {

	switch (p->event_id)
	{
	case HP_ADD: {	// 1�ʸ��� hp 5�� ä���

		// �̹� ����� ����ų� ( ai �� ���� �༮�̸� pass )
		if (DISCONNECTED == g_clients[p->obj_id]->get_current_connect_state()) { break;	}

		int adding_hp_size = 5;
		
		// hp�� 100 �̻��� �ƴϸ�, �Ʒ� ����
		if (false == (g_clients[p->obj_id]->get_player_data()->state.hp > (MAX_HP - 1))) {
			g_clients[p->obj_id]->get_player_data()->state.hp += adding_hp_size;

			// ���ǰ� �Ǿ��ٸ�, ��� hp �����ִ� ��� ����
			if (MAX_HP == g_clients[p->obj_id]->get_player_data()->state.hp) { *g_clients[p->obj_id]->get_hp_adding() = false; }
			add_event(p->obj_id, 1, HP_ADD, false);

			Packet buf[MAX_BUF_SIZE]{ 0 };
			buf[0] = (sizeof(int) * 2) + 2;	// ��Ŷ size
			buf[1] = SERVER_MESSAGE_HP_CHANGED;
			*reinterpret_cast<int *>(&buf[2]) = g_clients[p->obj_id]->get_player_data()->state.hp;	// hp �Է�
			*reinterpret_cast<int *>(&buf[6]) = p->obj_id;	// id �Է�
			
			for (auto players : g_clients) {
				if (DISCONNECTED == players->get_current_connect_state()) { continue; }
				//if (players->m_id == other_players->m_id) { continue; }	// �ڱ� hp �� ���ص� �ش� ��Ŷ�� �޾ƾ� �Ѵ�.
				if (true == players->get_player_data()->is_ai) { continue; }

				players->send_packet(buf);
			}
		}
	}
		break;
	default:
		break;
	}
}