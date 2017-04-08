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

// AI index
#define MAX_AI_NUM 50

// process protocol
enum PacketProtocolType {
	TEST = 1,		// ���� ��Ŷ �״�� �����ֱ��. ( ���� ������ �״�� ���ƿ��� �ʴ´ٸ�, Ŭ�� ������ ������ �ִٴ� �� )
	DB_LOGIN,		// DB ���� ���� �ڵ�

	INIT_CLIENT,
	INIT_OTHER_CLIENT,
	PLAYER_DISCONNECTED,

	// ĳ���� ��ǥ �� ���� ����
	CHANGED_POSITION,
	CHANGED_DIRECTION,

	// ĳ���� ���
	CHANGED_INVENTORY,

	// Ű �Է°��� ��ȣ�ۿ�
	KEYINPUT_ATTACK,

	// �нú� ȿ���� ���� ������ �뺸
	SERVER_MESSAGE_HP_CHANGED,

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
	int maxhp{ 5 };
	int hp{ 5 };
	unsigned short mp{ 10 };
	unsigned short level{ 2 };
	unsigned short exp{ 0 };
	short def{ 1 };
	short str{ 5 };
	short agi{ 2 };
	short intel{ 1 };
	short health{ 3 };
	short gauge{ 0 };
};

/// �÷��̾� ��ü ���� 64 ( dir -3 ) ( nickname - 2 ) bytes
using player_data = struct Player_data {
	unsigned int	id{ 0 };	// 4
	position		pos;		// 8
	status			state;		// 8

	char			dir{ KEYINPUT_DOWN };	// 1
	bool			is_ai{ false };			// 1

	char			nickname[42]{ 0 };	// 42
};