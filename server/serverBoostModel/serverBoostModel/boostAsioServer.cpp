#pragma once
#include"stdafx.h"
using boost::asio::ip::tcp;

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
	if (nullptr != m_strand)	delete m_strand;
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

	// strand �� �ڽ��� ���� ����ġ �Ǵ� �ڵ鷯����, �������� �ڵ鷯�� �Ϸ�Ǿ�߸� ���� �ڵ鷯�� ���۵� �� �ֵ��� �ϴ� ���� �������ݴϴ�.
	m_strand = new boost::asio::io_service::strand(m_io_service);

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
		m_worker_threads.emplace_back(new thread{ boost::bind( &boost::asio::io_service::run, &m_io_service), this });
	}
	
	thread acceptThread{ &boostAsioServer::acceptThread, this };
	while (m_ServerShutdown) { Sleep(1000); }

	// io_service �� run() �� �ٸ� ������ �Ϸ��ϸ� �����Ѵٴµ�, ��ǻ� ���� ������ ���� �غ� �Ǿ��ִ� ���¸� ���ϴ� �� �ϴ�.
	m_io_service.run();
	
	// workerThread �ߵ�
	for (auto thread : m_worker_threads) {
		thread->join();
		delete thread;
	}
	
	// acceptThread �ߵ�
	acceptThread.join();

}

void boostAsioServer::acceptThread()
{
	while (true == (!m_ServerShutdown))
	{
		// ������ Ŭ���̾�Ʈ�� �Ҵ��� tcp::socket �� �����. socket �� ���ؼ� Ŭ���̾�Ʈ �޼����� �ְ� �����Ƿ� m_io_serviec �� �Ҵ�
		// ���⿡ �ش��ϴ� iocp �� accept, �� g_hIocp = CreateIoCompletionPort(...) �κ��� ������ �ִ� �Ͱ� ����.
		m_clients.emplace_back(new PLAYER_INFO(m_io_service, ++m_playerIndex));
		
		// ���� �� �κ� ������
		/*m_clients[m_playerIndex]->getSocket()->async_connect(*m_endpoint,
			boost::bind([&](const boost::system::error_code& error) {
			if (error) { cout << "connect ERROR failed : " << m_playerIndex << "player\n"; }
			else { cout << "[ No. " << m_playerIndex << " ] Client IP = " << m_clients[m_playerIndex]->getSocket()->remote_endpoint().address().to_string() << ", Port = " << m_clients[m_playerIndex]->getSocket()->remote_endpoint().port() << " is Connected\n"; }
		}, this, boost::asio::placeholders::error));*/
	}
}

void boostAsioServer::workerThread()
{
	while (true == (!m_ServerShutdown))
	{

	}
}