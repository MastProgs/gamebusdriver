// serverBoostModel.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"


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