// boost asio ��ġ�� ���� ���� --->	http://cattool.tistory.com/94
// C/C++, �Ϲ�, �߰� ���� ���丮 ���� ��ġ	..\boost_1_63_0\boost_1_63_0;				%(AdditionalIncludeDirectories)
// ��Ŀ, �Ϲ�, �߰� ���̺귯�� ���丮			..\boost_1_63_0\boost_1_63_0\stage\lib;		%(AdditionalLibraryDirectories)

#pragma once
#include<boost\asio.hpp>

// �þ߸���Ʈ ������ STL ( ���� CAS �� ���� �ڷᱸ���� ���� ��� �ϹǷ� ����� ���� )
#include <unordered_set>
#include <mutex>
#include <queue>

static boost::asio::io_service g_io_service;

using boost::asio::ip::tcp;

// DB ���ٿ� class
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <tchar.h>

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

#define TRYODBC(h, ht, x)   {   RETCODE rc = x;\
                                if (rc != SQL_SUCCESS)	{ HandleDiagnosticRecord (h, ht, rc); } \
                                if (rc == SQL_ERROR)	{ fwprintf(stderr, L"Error in " L#x L"\n"); }} //\
                                    //goto Exit;  \
                                }  \
                            }
#define SQLSERVERADDR	L"Driver={ODBC Driver 13 for SQL Server};Server=tcp:mygame.database.windows.net,1433;Database=myDatabase;Uid=uzchowall@mygame;Pwd={rlagudwns1!};Encrypt=yes;TrustServerCertificate=no;Connection Timeout=30;"

// ������ �߰�
/// L"INSERT INTO dbo.user_data ( ID, Password, Nickname, PlayerLevel, AdminLevel ) VALUES ( '2013180056', 'ghdtmdvlf', N'ȫ����', 1, 0 )"
// ������ ���� ( http://blog.naver.com/khjkhj2804/220725944795 )
/// if (SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT ID, Password, Nickname, PlayerLevel, PositionX, PositionY FROM dbo.user_data", SQL_NTS) != SQL_ERROR)

class DB {
public:
	DB() { /*Init();*/ };
	~DB() { Release(); };

	void SQLcmd(SQLWCHAR* str);
	bool DB_Login(wchar_t* id, wchar_t* pw);

	void Init();
private:
	void Release();

	SQLHENV     hEnv = NULL;
	SQLHDBC     hDbc = NULL;
	SQLHSTMT    hStmt = NULL;
};

class player_session //: public std::enable_shared_from_this<player_session>
{
public:
	player_session(tcp::socket s, const unsigned int& index) : m_socket(std::move(s)), m_id(index) {};
	~player_session() {};
	
	void Init();

	unsigned int get_id() { return m_id; }
	bool get_current_connect_state() { return m_connect_state; }
	player_data* get_player_data() { return &m_player_data; }
	void send_packet(Packet *packet);
	bool check_login();
private:
	// Function
	void m_recv_packet();
	void m_process_packet(Packet buf[]);
	void m_passive_hp_adding();

	// ��ſ� ����
	tcp::socket		m_socket;

	// �÷��̾� ���� ����
	bool m_connect_state{ false };
	unsigned int m_id{ 0 };

	// ���� ����
	Packet m_recv_buf[MAX_BUF_SIZE]{ 0 };
	Packet m_data_buf[MAX_BUF_SIZE]{ 0 };
	unsigned int m_packet_size_current{ 0 };
	unsigned int m_packet_size_previous{ 0 };

	// DB ���� id �� pw
	wchar_t m_login_id[MAX_BUF_SIZE / 4]{ 0 };
	wchar_t m_login_pw[MAX_BUF_SIZE / 4]{ 0 };

	// hp passive �� ���� bool ����
	bool m_is_hp_can_add{ false };

	// �÷��̾� ĳ���� ���� ����
	player_data m_player_data;

	// ���� �÷��̾��� view list
	/// ��� 1. �⺻ unorderd_set �ڷ����� �� ���ΰ� ?
	unordered_set<unsigned int> m_view_list;
	/// ��� 2. ���� ��Ƽ ������ ���� �ڷᱸ���� ����� �� ���ΰ� ? CAS... 5000�� ���ؼ��� ��� �Ұ��̴�...
};

class boostAsioServer
{
public:
	boostAsioServer();
	~boostAsioServer();
	
private:
	// Function
	void getMyServerIP();
	void CheckThisCPUcoreCount();

	void start_io_service();
	void acceptThread();

	// ��ſ� ����
	tcp::acceptor		m_acceptor;
	tcp::socket			m_socket;

	// �÷��̾� ���� id ����
	unsigned int	m_playerIndex{ UINT_MAX };

	// ���� �⺻ ���� ���� ����
	bool			m_ServerShutdown{ false };
	int				m_cpuCore{ 0 };
	vector<thread*>	m_worker_threads;
};

// Timer class ---------------------------------------------------------------------------------------------------------------
using event_type = struct Event_type
{
	unsigned int obj_id;
	unsigned int wakeup_time;
	int event_id;
};

//auto wake_time_cmp = [](const event_type lhs, const event_type rhs) -> const bool { return (lhs.wakeup_time > rhs.wakeup_time); };
class wake_time_cmp { public: bool operator() (const event_type lhs, const event_type rhs) const { return (lhs.wakeup_time > rhs.wakeup_time); }};

class TimerQueue
{
public:
	TimerQueue() {};
	~TimerQueue() {};

	void TimerThread();
private:

	// timer thread queue
	priority_queue < event_type, vector<event_type>, wake_time_cmp/*decltype(wake_time_cmp)*/ > timer_queue;
};


// �÷��̾ ��� ����
static mutex g_clients_lock;
static vector<player_session*> g_clients;

// DB ��ſ� ����
static DB database;
static TimerQueue time_queue;