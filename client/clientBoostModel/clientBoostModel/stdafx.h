#pragma once
#pragma comment(lib, "ws2_32")

#include<WinSock2.h>
#include<Windows.h>


#include<iostream>
using namespace std;

// boost asio ������ �����Ǵ� protocol.h - ������ �������� �ٷ� ����ȴ�.
#include"../../../server\serverBoostModel/serverBoostModel/protocol.h"
#define WM_SOCKET (WM_USER + 1)

// client ��� ����
#include<fstream>

// ����� ���� �ܼ�â
#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif