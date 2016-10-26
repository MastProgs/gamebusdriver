#pragma once
#include<boost\bind.hpp>
#include<boost\asio.hpp>

class PLAYER_INFO
{
public:
	PLAYER_INFO(boost::asio::io_service& io, const unsigned int id) : s(new boost::asio::ip::tcp::socket(io)), id(id) {};
	~PLAYER_INFO() { delete s; };

	// ������� �Ұ�
	PLAYER_INFO(const PLAYER_INFO&) = delete;
	// �̵����� �Ұ�
	PLAYER_INFO& operator=(const PLAYER_INFO&) = delete;

	boost::asio::ip::tcp::socket* getSocket() { return s; }
	Packet* getRecvBuf() { return recvBuf; }

private:
	boost::asio::ip::tcp::socket* s;
	unsigned int id;
	Packet recvBuf[MAX_BUF_SIZE]{ 0 };

};

class boostAsioServer
{
public:
	boostAsioServer();
	~boostAsioServer();
	
private:
	// Function
	void Init();
	void getMyServerIP();
	void CheckThisCPUcoreCount();

	void makeWorkerThreads_and_AcceptThread();
	void acceptThread();
	void workerThread();

	void handle_accept(PLAYER_INFO*, const boost::system::error_code& error);

	// member ����
	boost::asio::io_service m_io_service;

	boost::asio::ip::tcp::resolver				*m_resolver{ nullptr };
	boost::asio::ip::tcp::resolver::query		*m_query{ nullptr };
	boost::asio::ip::tcp::resolver::iterator	m_resolver_iterator;

	//boost::asio::ip::tcp::socket	*m_socket{ nullptr };
	boost::asio::ip::tcp::acceptor	*m_acceptor{ nullptr };
	boost::asio::ip::tcp::endpoint	*m_endpoint{ nullptr };
	boost::asio::io_service::strand *m_strand{ nullptr };


	unsigned int m_playerIndex{ UINT_MAX };
	vector<PLAYER_INFO *> m_clients;

	bool m_ServerShutdown{ false };
	int m_cpuCore{ 0 };
	vector<thread*> m_worker_threads;
};