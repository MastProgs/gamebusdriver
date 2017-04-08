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
	int maxhp{ 100 };
	int hp{ 100 };
	BYTE mp{ 10 };
	BYTE level{ 1 };
	unsigned short exp{ 0 };
	BYTE critical{ 20 };
	unsigned short def{ 5 };
	unsigned short str{ 5 };
	unsigned short agi{ 5 };
	unsigned short intel{ 5 };
	unsigned short health{ 5 };
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

#pragma pack (push, 1)

using sc_client_init_info = struct server_to_client_info
{
	BYTE size = sizeof(player_data) + sizeof(BYTE) + sizeof(BYTE);
	BYTE type = INIT_CLIENT;
	player_data player_info;
};

using sc_other_init_info = struct server_to_client_other_clients_info
{
	BYTE size = sizeof(unsigned int) + sizeof(int) + sizeof(position) + sizeof(BYTE) + sizeof(BYTE);
	BYTE type = INIT_OTHER_CLIENT;
	unsigned int id;
	int hp;
	position pos;
};

using sc_disconnect = struct server_to_client_player_disconnect
{
	BYTE size = sizeof(unsigned int) + sizeof(BYTE) + sizeof(BYTE);
	BYTE type = PLAYER_DISCONNECTED;
	unsigned int id;
};

#pragma pack (pop)