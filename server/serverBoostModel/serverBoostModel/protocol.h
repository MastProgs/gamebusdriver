#pragma once

#define SERVERPORT 9000
#define MAX_BUF_SIZE 256
#define MAX_USER 500

// packet[1] operation
#define DISCONNECTED 0
#define CONNECTED 1

// iocp buf operation
#define OP_SERVER_RECV 1
#define OP_SERVER_SEND 2

// process protocol
enum PacketProtocolType {
	TEST = 1,		// ���� ��Ŷ �״�� �����ֱ��. ( ���� ������ �״�� ���ƿ��� �ʴ´ٸ�, Ŭ�� ������ ������ �ִٴ� �� )
	DUMMY_CLIENT,	// dummy client �� ��������
	KEYINPUT,		// Ŭ���̾�Ʈ���� Ű �Է��� �޾��� ���

};	

using Packet = unsigned char;

// ĳ���� �⺻ ���� struct
/// �⺻ ��ǥ��
using position = struct Position {
	float x{ 0 };
	float y{ 0 };
};
/// �÷��̾� ��ü ����
using player_data = struct Player_data {
	unsigned int id{ 0 };
	position pos;
};