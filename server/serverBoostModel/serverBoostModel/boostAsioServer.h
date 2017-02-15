// boost asio ��ġ�� ���� ���� --->	http://cattool.tistory.com/94
// C/C++, �Ϲ�, �߰� ���� ���丮 ���� ��ġ	..\boost_1_63_0\boost_1_63_0;				%(AdditionalIncludeDirectories)
// ��Ŀ, �Ϲ�, �߰� ���̺귯�� ���丮			..\boost_1_63_0\boost_1_63_0\stage\lib;		%(AdditionalLibraryDirectories)

#pragma once
#include<boost\asio.hpp>

// �þ߸���Ʈ ������ STL ( ���� CAS �� ���� �ڷᱸ���� ���� ��� �ϹǷ� ����� ���� )
#include <unordered_set>
#include <mutex>

static boost::asio::io_service g_io_service;

using boost::asio::ip::tcp;

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

// �÷��̾ ��� ����
static mutex g_clients_lock;
static vector<player_session*> g_clients;







// DB ���ٿ� class
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <tchar.h>

void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	WCHAR       wszMessage[1000];
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}

	while (SQLGetDiagRec(hType,	hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT *)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))	{ fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);	}
	}

}

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
	DB() { Init(); };
	~DB() { Release(); };

	auto SQLcmd(SQLWCHAR* str);

private:
	void Init();
	void Release();

	SQLHENV     hEnv = NULL;
	SQLHDBC     hDbc = NULL;
	SQLHSTMT    hStmt = NULL;
};

void DB::Init() {
	// Allocate an environment

	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) == SQL_ERROR) {
		fwprintf(stderr, L"DBacess class :: Unable to allocate an environment handle\n");
		exit(-1);
	}
	
	TRYODBC(hEnv, SQL_HANDLE_ENV, SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0));
	TRYODBC(hEnv, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc));
	TRYODBC(hDbc, SQL_HANDLE_DBC, SQLDriverConnect(hDbc, GetDesktopWindow(), SQLSERVERADDR, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE));
	TRYODBC(hDbc, SQL_HANDLE_DBC, SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt));
	
	fwprintf(stderr, L"Azure SQL Database Server Connected!\n");
	// Loop to get input and execute queries -- ������ ������� �Ϸ� �Ǿ���, ���⼭ ���� ��ɾ ������ �����ϰ� �ȴ�.
}

void DB::Release() {
	// Free ODBC handles and exit

	if (hStmt) { SQLFreeHandle(SQL_HANDLE_STMT, hStmt); }

	if (hDbc) {
		SQLDisconnect(hDbc);
		SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	}

	if (hEnv) {	SQLFreeHandle(SQL_HANDLE_ENV, hEnv); }

	wprintf(L"\nAzure SQL DataBase Server Disconnected.\n");
}

auto DB::SQLcmd(SQLWCHAR* str) {

	RETCODE     RetCode;
	SQLSMALLINT sNumResults;

	RetCode = SQLExecDirect(hStmt, str, SQL_NTS);
	
	switch (RetCode)
	{
	case SQL_SUCCESS_WITH_INFO:	// ���� ���д� �ߴµ�, �ϴ� ��� ���� �� �� ���� ���...
	{
		HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
	}
	case SQL_SUCCESS:
	{
		// �������� ��... �ڵ� ����
		// SQLNumResultCols() �Լ��� ��� ��Ʈ�� �ش� �� ����, ������ ���� �޴´�. ( �� = -> )
		TRYODBC(hStmt, SQL_HANDLE_STMT, SQLNumResultCols(hStmt, &sNumResults));

		// ����� 0�� �̻��̸� ���� ó���Ѵ�. ��߲��...
		if (sNumResults > 0) {

		}
		else
		{
			SQLINTEGER cRowCount;

			// �ݴ�� SQLRowCount() �Լ��� �� ����, ������ ���� �޴´�. ( �� = V )
			TRYODBC(hStmt, SQL_HANDLE_STMT, SQLRowCount(hStmt, &cRowCount));

			// ���� 0 �̻��̸� �����͸� ����ϴ� ����...
			if (cRowCount >= 0) {
				wprintf(L"%Id %s affected\n", cRowCount, cRowCount == 1 ? L"row" : L"rows");
			}
		}
		break;
	}
	case SQL_ERROR: // ���� ���� ���, �Ʒ� �Լ��� ���ؼ� ���� ����� ȭ�鿡 ������ش�.
	{		
		HandleDiagnosticRecord(hStmt, SQL_HANDLE_STMT, RetCode);
	}
	default:
		fwprintf(stderr, L"Unexpected return code %hd!\n", RetCode);
	}


}