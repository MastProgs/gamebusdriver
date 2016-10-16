#pragma once
#include<boost\bind.hpp>
#include<boost\asio.hpp>

class boostAsioClient
{
public:
	boostAsioClient();
	~boostAsioClient();

	void Init(const HWND& hwnd);
	void ProcessWinMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void sendPacket_TEST();

private:
	// member Function
	void inputServerIP();
	void inputServerIP_ReadtxtFile();
	void inputServerIP_cin();

	void processPacket();

	// member ����
	/// ������ â ���� ��� ����
	WNDCLASS m_wndclass;
	HWND m_hWnd;
	MSG m_msg;

	/// ��Ű��� ��� ����
	boost::asio::io_service m_io_service;

	boost::asio::ip::tcp::resolver *m_resolver{ nullptr };
	boost::asio::ip::tcp::resolver::query *m_query{ nullptr };
	boost::asio::ip::tcp::resolver::iterator m_endpoint_iterator;

	boost::asio::ip::tcp::socket *m_socket{ nullptr };

	CHAR m_serverIP[32]{ 0 };

	Packet m_recvBuf[MAX_BUF_SIZE]{ 0 };
	Packet m_sendBuf[MAX_BUF_SIZE]{ 0 };
};