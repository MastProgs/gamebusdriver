// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include <iostream>
#include <cstdio>
#include <vector>
#include <thread>
using namespace std;

// �þ߸���Ʈ ������ STL
#include <unordered_set>
#include <mutex>
#include <queue>
#include <string>

// boost asio Ŭ��� �����Ǵ� protocol.h
#include "protocol.h"

// LUA script ���� ���
extern "C" {
#include "lualib.h"
#include "lauxlib.h"
#include "lua.h"
}
#include "readLUAScript.h"

// boost asio ���� ���
#include <memory>
#include <boost\asio.hpp>

extern boost::asio::io_service g_io_service;

#include "DB.h"
#include "TimerQueue.h"
#include "player_session.h"
#include "boostAsioServer.h"

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

// �÷��̾ ��� ����
extern mutex g_clients_lock;
extern vector<player_session*> g_clients;
//static AI_session g_AIs[MAX_AI_NUM];

// DB ��ſ� ����
extern DB g_database;
extern TimerQueue g_time_queue;