#pragma once

using boost::asio::ip::tcp;

#define VIEW_RANGE	10
#define RANGE_CHECK_AI_ATT	3.25
#define DISTANCE_TRIANGLE(x , y, m_x, m_y) ((((x) - (m_x)) * ((x) - (m_x))) + ((((y) - (m_y)) * ((y) - (m_y)))))
#define SQUARED(x) ((x) * (x))

enum player_state {
	none = 0,
	mov,
	att,
	dead,
};

class player_session //: public std::enable_shared_from_this<player_session>
{
public:
	player_session(tcp::socket s, const unsigned int& index) : m_socket(std::move(s)), m_id(index) {};
	~player_session() {};

	void Init();

	// �����Ƽ�, �׳� ���� ������ ( �÷��̾� ���� ���� )
	bool is_hp_postion{ false };

	void set_hp_adding(bool b) { is_hp_adding = b; }
	bool get_hp_adding() { return is_hp_adding; }

	void set_gauge_reducing(bool b) { is_gauge_reducing = b; }
	bool get_gauge_reducing() { return is_gauge_reducing; }

	void set_state(int s) { m_state = s; }
	int get_state() { return m_state; }

	bool set_hp(int hp) { m_player_data.state.hp = hp; return true; }
	int get_maxhp() { return m_player_data.state.maxhp; }

	unsigned int get_id() { return m_id; }
	bool get_current_connect_state() { return m_connect_state; }
	bool set_connect_state(bool b) { m_connect_state = b; return b; }
	player_data* get_player_data() { return &m_player_data; }
	sub_status* get_sub_data() { return &m_sub_status; }

	void vl_clear() {
		m_view_lock.lock();
		m_view_list.clear();
		m_view_lock.unlock();
	}
	void vl_add(unsigned int id) {
		m_view_lock.lock();
		m_view_list.insert(id);
		m_view_lock.unlock();
	}
	void vl_remove(unsigned int id) {
		m_view_lock.lock();
		m_view_list.erase(id);
		m_view_lock.unlock();
	}
	void vl_lock() { m_view_lock.lock(); }
	void vl_unlock() { m_view_lock.unlock(); }
	bool vl_find(unsigned int id) {
		if (m_view_list.end() != m_view_list.find(id)) { return true; }
		return false;
	}

	void refresh_view_list();

	bool is_in_view_range(unsigned int id);
	bool is_in_att_range(unsigned int id);
	inline unordered_set<unsigned int>* get_view_list() { return &m_view_list; }

	void send_packet(Packet *packet);
	bool check_login();

	// ai ���� �Լ�
	unsigned int ai_rand_mov();
	unsigned int return_nearlest_player(float range);
	void send_packet_other_players(Packet* p, unsigned int except_id);
	void send_packet_other_players_in_view_range(Packet* p, unsigned int mid_id);

	// ai Ȱ�� �뵵 ����
	unsigned int m_target_id{ 0 };
	bool ai_is_rand_mov{ false };
	float ai_mov_speed{ 1 };
	Position origin_pos;
	int radius;

	bool quest_start{ false };
private:
	// Function
	void m_recv_packet();
	void m_process_packet(Packet buf[]);

	// ��ſ� ����
	tcp::socket		m_socket;

	// �÷��̾� ���� ����
	bool m_connect_state{ true };
	unsigned int m_id{ 0 };
	bool is_hp_adding{ false };
	bool is_gauge_reducing{ false };

	// ���� ����
	Packet m_recv_buf[MAX_BUF_SIZE]{ 0 };
	Packet m_data_buf[MAX_BUF_SIZE]{ 0 };
	unsigned int m_packet_size_current{ 0 };
	unsigned int m_packet_size_previous{ 0 };

	// DB ���� id �� pw
	wchar_t m_login_id[MAX_BUF_SIZE / 4]{ 0 };
	wchar_t m_login_pw[MAX_BUF_SIZE / 4]{ 0 };

	// �÷��̾� ĳ���� ���� ����
	player_data m_player_data;
	sub_status m_sub_status;
	atomic_int m_state;
	mutex state_lock;

	// ���� �÷��̾��� view list
	/// ��� 1. �⺻ unorderd_set �ڷ����� �� ���ΰ� ?
	mutex m_view_lock;
	unordered_set<unsigned int> m_view_list;
	/// ��� 2. ���� ��Ƽ ������ ���� �ڷᱸ���� ����� �� ���ΰ� ? CAS... 5000�� ���ؼ��� ��� �Ұ��̴�...
};