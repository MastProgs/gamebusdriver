// serverBoostModel.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

boost::asio::io_service g_io_service;

// �÷��̾ ��� ����
mutex g_clients_lock;
vector<player_session*> g_clients;
//static AI_session g_AIs[MAX_AI_NUM];

// DB ��ſ� ����
DB database;
TimerQueue time_queue;

int main()
{
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