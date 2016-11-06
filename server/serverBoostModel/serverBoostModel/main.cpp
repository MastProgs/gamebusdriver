// serverBoostModel.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

/*// Session Ŭ������ ������ ������ Ŭ���̾�Ʈ
class Session {
public:
	Session(boost::asio::io_service &io_service) : m_Socket(io_service) {}

	boost::asio::ip::tcp::socket& Socket() { return m_Socket; }

	void PostReceive() {
		m_Socket.async_read_some(boost::asio::buffer(m_ReceiveBuffer), boost::bind(&Session::handle_receive, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}

private:

	void handle_write(const boost::system::error_code&, size_t) {}
	void handle_receive(const boost::system::error_code& error, size_t bytes_transferred) {
		if (error) {
			if (error == boost::asio::error::eof) {	cout << "Ŭ���̾�Ʈ�� ������ ������ϴ�.\n"; }
			else { cout << "���� ��ȣ : " << error.value() << " ���� �޼��� : " << error.message() << endl;	}
		}
		else {
			cout << "Ŭ���̾�Ʈ���� ���� �޼��� : " << m_ReceiveBuffer << endl;

			// async_write ( �����͸� ���� Ŭ���̾�Ʈ ����, ���� �����Ͱ� ��� ����, ���� �� ȣ���� �Լ� )
			// bind ( �Ϸ��ϸ� ȣ���� �Լ� (���⼱ handle_write), �Լ��� ����� ������ Ŭ������ �ν��Ͻ� (handle_write �ڱ��ڽ�), write �Լ��� �ѱ� ù ����, write �Լ��� �ѱ� �ι�° ����
			/// void handle_write(const boost::system::error_code&, size_t) = ù ���� Eroor �ڵ� , �ι�° ���� ������ ũ��
			boost::asio::async_write(m_Socket, boost::asio::buffer(m_WritreMessage), boost::bind(&Session::handle_write, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
			
			PostReceive();
		}
	}

	// async_write ������ ���۴�, �����Ͱ� �� �������� �� ( �Ϸ� �Լ��� �ҷ����� �� ) ������ ���� �����Ǿ�� �Ѵ�. ( �񵿱� �̱� ������ )
	char m_WritreMessage[MAX_BUF_SIZE]{ 0 };
	char m_ReceiveBuffer[MAX_BUF_SIZE]{ 0 };
	boost::asio::ip::tcp::socket m_Socket;
};

class TCP_Server {
public:
	TCP_Server(boost::asio::io_service& io_service) : m_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), SERVERPORT)) {
		m_pSession = nullptr;
		StartAccept();
	}

	~TCP_Server() {	if (m_pSession != nullptr) { delete m_pSession; }}

private:
	void StartAccept() {
		cout << "Ŭ���̾�Ʈ ���� ���....\n";

		m_pSession = new Session(m_acceptor.get_io_service());
		m_acceptor.async_accept(m_pSession->Socket(), boost::bind(&TCP_Server::handle_accept, this, m_pSession, boost::asio::placeholders::error));
	}

	void handle_accept(Session* pSession, const boost::system::error_code& error) {
		if (!error) { cout << "Ŭ���̾�Ʈ ���� ����\n"; pSession->PostReceive(); }
	}

	int m_nSeqNumber;
	boost::asio::ip::tcp::acceptor m_acceptor;
	Session* m_pSession;
};*/

int main()
{
	//// io_service �� OS Ŀ�ΰ� ���α׷��� �߰� �Ű�ü ������ �Ѵ�.
	//boost::asio::io_service io_service;
	//
	//// endpoint �� ��Ʈ��ũ �ּҸ� �����Ѵ�.
	//boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), SERVERPORT);
	//// acceptor Ŭ������ Ŭ���̾�Ʈ�� ������ �޾Ƶ��̴� ������ �Ѵ�. ���ڴ� io_service �� endpoint
	//boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
	//
	//// socket �� ���ؼ�, Ŭ���̾�Ʈ�� ���� �޽����� �ְ� �޾ƾ� �ϹǷ�, io_service �Ҵ�
	//boost::asio::ip::tcp::socket socket(io_service);
	//// accept ��� �Լ���, Ŭ���̾�Ʈ�� socket ��ü�� �Ѱ��ָ�, ������ Ŭ���̾�Ʈ�� ���� ���� �غ�� ������.
	///// ������ ����̶�, �Ϸ�� �� ���� ���
	//acceptor.accept(socket);
	//
	//std::cout << "Ŭ���̾�Ʈ ����\n";
	//
	//while (true)
	//{
	//	char buf[MAX_BUF_SIZE]{ 0 };
	//
	//	// error_code Ŭ������ �ý��ۿ� �߻��� �����ڵ带 ������ Ŭ����, ���� �߻� �� ���� �ڵ�� ���� �޼��� ȹ�� ����
	//	boost::system::error_code error;
	//	// read_some ��� �Լ��� Ŭ���̾�Ʈ�� ���� �����͸� ���� ( ������ ������ �� )
	//	/// ������ ����̶�, �����͸� �� ���� �� ���� ����Ѵ�.
	//	size_t len = socket.read_some(boost::asio::buffer(buf), error);
	//
	//	if (error) {
	//		if (error == boost::asio::error::eof) {
	//			// Ŭ���̾�Ʈ�� ������ �������� ���, read_some ��� �Լ��� ���� �� �� �ִ�. �̷� �� eof �� ������ ��.
	//			cout << "Ŭ���̾�Ʈ�� ������ ����\n";
	//		}
	//		else {
	//			cout << "error No : " << error.value() << " error Messgage : " << error.message() << "\n";
	//		}
	//		break;
	//	}
	//	cout << "Ŭ���̾�Ʈ���� ���� �޼��� : " << buf << "\n";
	//
	//	char szMessage[MAX_BUF_SIZE]{ 0 };
	//	// ���ۿ� ���� ��� ( �Ǵ� ���� ) �ϴ� �Լ�
	//	sprintf_s(szMessage, MAX_BUF_SIZE - 1, "Re : %s", buf);
	//
	//	int nMsgLen = strnlen_s(szMessage, MAX_BUF_SIZE - 1);
	//
	//	boost::system::error_code ignored_error;
	//	socket.write_some(boost::asio::buffer(szMessage, nMsgLen), ignored_error);
	//	cout << "Ŭ���̾�Ʈ�� ���� �޼��� : " << szMessage << "\n";
	//} // ������� ���� ������ ���� ����
	
	//boost::asio::io_service io_service;
	//TCP_Server server(io_service);
	//// io_service �� run �Լ��� �񵿱� ��û�� ���� ���, ��û�� ���� �� ���� ������ ��� -> �񵿱� ��û�� �ϱ� ���� run() �� �Ѵٸ�, �׳� ����Ǿ� ������.
	///// ���� ��û�� ���ٸ�, �������� ���� �۾��� ����
	//io_service.run();
	//cout << "��Ʈ��ũ ���� ����\n";

	try
	{
		_wsetlocale(LC_ALL, L"korean");
		boostAsioServer server;
	}
	catch (const std::exception& e)
	{
		std::cout << "Main Function's Exception = " << e.what() << "\n";
	}
}