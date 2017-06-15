// serverBoostModel.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

boost::asio::io_service g_io_service;

// �÷��̾ ��� ����
mutex g_clients_lock;
vector<player_session*> g_clients;

// DB ��ſ� ����
DB g_database;
TimerQueue g_time_queue;

// protocol ��� ������ & boostAsioServer.cpp ���� �� �������ش�.
int MAX_AI_SLIME;
int MAX_AI_GOBLIN;
int MAX_AI_NUM;

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