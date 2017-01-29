#pragma once

#define SERVERPORT 9000
#define MAX_BUF_SIZE 256
#define MAX_USER 500

#define WM_SOCKET (WM_USER + 1)

// packet[1] operation
#define DISCONNECTED 0
#define CONNECTED 1

// iocp buf operation
#define OP_SERVER_RECV 1
#define OP_SERVER_SEND 2

// process protocol
enum PacketProtocolType {
	TEST = 1,		// ���� ��Ŷ �״�� �����ֱ��. ( ���� ������ �״�� ���ƿ��� �ʴ´ٸ�, Ŭ�� ������ ������ �ִٴ� �� )
	//DUMMY_CLIENT,	// dummy client �� ��������
	INIT_CLIENT,
	PLAYER_DISCONNECTED,
	//KEYINPUT,		// Ŭ���̾�Ʈ���� Ű �Է��� �޾��� ���
	CHANGED_POSITION,
	CHANGED_DIRECTION,
	KEYINPUT_ATTACK,
};

using Packet = unsigned char;

// ĳ���� �⺻ ���� struct
/// �⺻ ��ǥ��
using position = struct Position {
	float x{ 100 };
	float y{ 100 };
};
/// ���� ��ǥ��
#define KEYINPUT_UP		0b00000001
#define KEYINPUT_DOWN	0b00000010
#define KEYINPUT_LEFT	0b00000100
#define KEYINPUT_RIGHT	0b00001000

using status = struct Status {
	int hp{ 100 };
};
/// �÷��̾� ��ü ����
using player_data = struct Player_data {
	unsigned int	id{ 0 };
	position		pos;
	char			dir;
	status			sta;
};