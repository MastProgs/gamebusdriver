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

	// ĳ���� ��ǥ �� ����, �߿� ����
	CHANGED_POSITION,
	CHANGED_DIRECTION,
	CHANGED_FEVER,

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

#define KEYINPUT_MASK	0b00001111
#define DIR_XOR(dir)	((~dir) & KEYINPUT_MASK)

using status = struct Status {
	int maxhp{ 100 };
	int hp{ 100 };
	unsigned char mp = 10;
	unsigned char level = 1;
	unsigned short exp{ 0 };
	short maxgauge{ 400 };
	short gauge{ 0 };
};

using sub_status = struct Sub_status
{
	unsigned char critical = { 20 };
	unsigned short def{ 10 };
	unsigned short str{ 10 };
	unsigned short agi{ 10 };
	unsigned short intel{ 10 };
	unsigned short health{ 10 };
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
	unsigned char size = sizeof(player_data) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = INIT_CLIENT;
	player_data player_info;
};

using sc_other_init_info = struct server_to_client_other_clients_info
{
	unsigned char size = sizeof(player_data) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = INIT_OTHER_CLIENT;
	player_data playerData;
};

using sc_disconnect = struct server_to_client_player_disconnect
{
	unsigned char size = sizeof(unsigned int) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = PLAYER_DISCONNECTED;
	unsigned int id;
};

using sc_move = struct server_to_client_player_move
{
	unsigned char size = sizeof(position) + sizeof(unsigned int) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = CHANGED_POSITION;
	unsigned int id;
	position pos;
};

using sc_dir = struct server_to_client_player_direction
{
	unsigned char size = sizeof(char) + sizeof(unsigned int) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = CHANGED_DIRECTION;
	unsigned int id;
	char dir;
};

using sc_atk = struct server_to_client_attack_states
{
	unsigned char size = sizeof(int) + sizeof(unsigned int) + sizeof(unsigned int) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = KEYINPUT_ATTACK;
	unsigned int attacking_id;
	unsigned int under_attack_id;
	int hp;
};

using sc_hp = struct server_to_client_changed_hp
{
	unsigned char size = sizeof(int) + sizeof(unsigned int) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = SERVER_MESSAGE_HP_CHANGED;
	unsigned int id;
	int hp;
};

using sc_fever = struct server_to_client_changed_fever_gauge
{
	unsigned char size = sizeof(short) + sizeof(unsigned char) + sizeof(unsigned char);
	unsigned char type = CHANGED_FEVER;
	short gauge;
};

#pragma pack (pop)