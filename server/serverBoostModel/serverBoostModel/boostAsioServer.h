// ���丮 ���� ��ġ				..\boost_1_62_0\boost_1_62_0;				%(AdditionalIncludeDirectories)
// ��Ŀ �߰� ���̺귯�� ���丮	..\boost_1_62_0\boost_1_62_0\stage\lib;		%(AdditionalLibraryDirectories)

#pragma once
#include<boost\asio.hpp>

// �þ߸���Ʈ ������ STL ( ���� CAS �� ���� �ڷᱸ���� ���� ������ ����� ���� )
#include <unordered_set>

static boost::asio::io_service g_io_service;

using boost::asio::ip::tcp;

class player_session : public std::enable_shared_from_this<player_session>
{
public:
	player_session(tcp::socket s, const unsigned int& index) : m_socket(std::move(s)), m_id(index) {};
	~player_session() {};

	void Init();

	bool get_current_connect_state() { return m_connect_state; }
private:
	// Function
	void m_recv_packet();
	void m_process_packet(const unsigned int& id, Packet buf[]);
	void send_packet(const unsigned int& id, Packet *packet);

	// ��ſ� ����
	tcp::socket		m_socket;

	// �÷��̾� ���� ����
	bool m_connect_state{ false };
	unsigned int m_id{ 0 };

	// ���� ����
	Packet m_recv_buf[MAX_BUF_SIZE]{ 0 };
	Packet m_data_buf[MAX_BUF_SIZE]{ 0 };
	unsigned int m_packet_size_current{ 0 };
	unsigned int m_packet_size_previous{ 0 };


	// �÷��̾� ĳ���� ���� ����
	player_data m_player_data;

	// ���� �÷��̾��� view list
	/// ��� 1. �⺻ unorderd_set �ڷ����� �� ���ΰ� ?
	unordered_set<unsigned int> m_view_list;
	/// ��� 2. ���� ��Ƽ ������ ���� �ڷᱸ���� ����� �� ���ΰ� ? CAS...
};

class boostAsioServer
{
public:
	boostAsioServer();
	~boostAsioServer();
	
private:
	// Function
	void getMyServerIP();
	void CheckThisCPUcoreCount();

	void start_io_service();
	void acceptThread();

	// ��ſ� ����
	tcp::acceptor	m_acceptor;
	tcp::socket		m_socket;

	// �÷��̾� ���� id ����
	unsigned int	m_playerIndex{ UINT_MAX };

	// ���� �⺻ ���� ���� ����
	bool			m_ServerShutdown{ false };
	int				m_cpuCore{ 0 };
	vector<thread*> m_worker_threads;
};

// �÷��̾ ��� ����
static vector<shared_ptr<player_session>> g_clients;