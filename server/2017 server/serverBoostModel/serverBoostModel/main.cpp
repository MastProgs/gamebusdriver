// serverBoostModel.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"

int main()
{
	cout << "g_clients ADDR : " << (int*)&g_clients << endl;
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