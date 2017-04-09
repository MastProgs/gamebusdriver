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

// boost asio Ŭ��� �����Ǵ� protocol.h
#include "protocol.h"

// boost asio ���� ���
#include <memory>
#include <boost\asio.hpp>

static boost::asio::io_service g_io_service;

#include "DB.h"
#include "player_sesstion.h"
#include "TimerQueue.h"
#include "boostAsioServer.h"

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

// �÷��̾ ��� ����
static mutex g_clients_lock;
static vector<player_session*> g_clients;
//static AI_session g_AIs[MAX_AI_NUM];

// DB ��ſ� ����
static DB database;
static TimerQueue time_queue;