#include"stdafx.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// �׸��� ���� ���� - WM_PAINT
int view_range{ 150 };
int value{ 10 };
int dir_value{ 3 };
int att_value{ 5 };
int hp_value_x{ 10 };
// ------------------------------

// Ű ���� ���� �Լ� - WM_KEYDOWN
void move_up();
void move_down();
void move_left();
void move_right();

bool is_key_s_pushed{ false };

// �ð� üũ ���� Ŭ���� - ����Ű s �Է�
class TimeCheck
{
public:
	TimeCheck() { key_s = system_clock::now(); };
	~TimeCheck() {};
	
	void	start_key_s()	{ key_s = system_clock::now(); }
	UINT	end_key_s()		{ return duration_cast<milliseconds>(system_clock::now() - key_s).count(); }
private:
	system_clock::time_point key_s;
};

TimeCheck timecheck;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	
	// ������ Ŭ���� ���
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = L"MyWndClass";
	if (false == RegisterClass(&wndclass)) { return -1; }

	// ������ ����
	HWND hWnd = CreateWindow(L"MyWndClass", L"WinApp", WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, NULL, hInstance, NULL);
	if (NULL == hWnd) { return -1; }
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	// �������� ��� ���� - ����� ��尡 �ƴ϶�� ServerIP.txt ���Ͽ� ���� ip �ּҷ� �ٷ� ������ �ȴ�.
	g_client.Init(hWnd);

	// �޽��� ����
	MSG msg;
	/*while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/

	HACCEL hAccelTable = LoadAccelerators(hInstance, 0);

	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			
		}
	}

	return msg.wParam;
}

// ������ ���ν���
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	// �׸��� ���� ���� - WM_PAINT
	HDC hdc;
	PAINTSTRUCT ps;
	// ------------------------------

	switch (uMsg)
	{
	case WM_CREATE:
		return 0;
		break;
	case WM_SIZE:
		return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;



		// �Ϲ� ���� key �� �Է� �޾��� ��
	case WM_CHAR:
	{
		switch (wParam)
		{
		case 's':	// attack key
			is_key_s_pushed = false;
			if (600 <= timecheck.end_key_s()) {
				g_client.sendPacket(0, KEYINPUT_ATTACK, nullptr);

				/*hdc = BeginPaint(hWnd, &ps);

				char *dir = &(g_client.getPlayerData()->dir);
				float x = g_client.getPlayerData()->pos.x, y = g_client.getPlayerData()->pos.y;
				
				/// view circle
				Ellipse(hdc, x - view_range, y - view_range, x + view_range, y + view_range);
				/// player circle
				Ellipse(hdc, x - value, y - value, x + value, y + value);

				if ((*dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT)) { x += (value + dir_value); }
				if ((*dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT)) { x -= (value + dir_value); }
				if ((*dir & KEYINPUT_UP) == (KEYINPUT_UP)) { y -= (value + dir_value); }
				if ((*dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN)) { y += (value + dir_value); }
				MoveToEx(hdc, x - att_value, y - att_value, nullptr);
				LineTo(hdc, x + att_value, y + att_value);
				MoveToEx(hdc, x - att_value, y + att_value, nullptr);
				LineTo(hdc, x + att_value, y - att_value);

				/// HP bar - player
				x = g_client.getPlayerData()->pos.x, y = g_client.getPlayerData()->pos.y;
				int hp = g_client.getPlayerData()->state.hp;
				RECT hp_bar;
				HBRUSH hp_color = CreateSolidBrush(RGB(0, 255, 0));
				hp_bar.left = x - hp_value_x;
				if (1 < hp) { hp_bar.right = hp_bar.left + (hp / 5); }
				hp_bar.top = y - 20;
				hp_bar.bottom = y - 17;
				if (hp < 50) { hp_color = CreateSolidBrush(RGB(255, 0, 0)); }
				FillRect(hdc, &hp_bar, hp_color);
				hp_bar.right = x + hp_value_x;
				FrameRect(hdc, &hp_bar, CreateSolidBrush(RGB(0, 0, 0)));

				EndPaint(hWnd, &ps);*/
				timecheck.start_key_s();
				is_key_s_pushed = true;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			break;
		default:
			break;
		}
	}
		break;


		// key �� �Է� �޾��� ��
	case WM_KEYDOWN:

		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
			break;
		case VK_RIGHT:
			move_right();
			break;
		case VK_LEFT:
			move_left();
			break;
		case VK_DOWN:
			move_down();
			break;
		case VK_UP:
			move_up();
			break;
		}
		InvalidateRect(hWnd, NULL, TRUE);

		// 1. �۾��ؾ� �ϴ� �͵�, �� ó�� Ŭ���̾�Ʈ ��� �� �ְ�޾ƾ� �ϴ� �����͵� �ְ� �ޱ�
		// 2. �ٸ� �÷��̾� ������ ���Ϳ� �����ϱ�
		// 3. �þ� ���� ���� �ִ� �÷��̾� �����Ű��
		{
			g_client.sendPacket(sizeof(position), CHANGED_POSITION, reinterpret_cast<BYTE*>(&g_client.getPlayerData()->pos));
			g_client.sendPacket(sizeof(char), CHANGED_DIRECTION, reinterpret_cast<BYTE*>(&g_client.getPlayerData()->dir));
		}
		break;


	// â�� �׸� ��
	case WM_PAINT: {
		hdc = BeginPaint(hWnd, &ps);

		char *dir;
		float x = g_client.getPlayerData()->pos.x, y = g_client.getPlayerData()->pos.y;

		/// this player view circle
		Ellipse(hdc, x - view_range, y - view_range, x + view_range, y + view_range);
		
		/// other player ---------------------------------------------------------------------------------
		for (auto players : *(g_client.getOtherPlayers())) {
			x = players.second.pos.x;
			y = players.second.pos.y;
			Ellipse(hdc, x - value, y - value, x + value, y + value);

			/// player direction
			dir = &players.second.dir;
			if ((*dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT)) { x += (value + dir_value); }
			if ((*dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT)) { x -= (value + dir_value); }
			if ((*dir & KEYINPUT_UP) == (KEYINPUT_UP)) { y -= (value + dir_value); }
			if ((*dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN)) { y += (value + dir_value); }
			Ellipse(hdc, x - dir_value, y - dir_value, x + dir_value, y + dir_value);

			/// HP bar - other_player
			x = players.second.pos.x;
			y = players.second.pos.y;
			int other_player_hp = players.second.state.hp;
			RECT other_player_hp_bar;
			HBRUSH other_player_hp_color = CreateSolidBrush(RGB(0, 100, 255));
			other_player_hp_bar.left = x - hp_value_x;
			if (1 < other_player_hp) { other_player_hp_bar.right = other_player_hp_bar.left + (other_player_hp / 5); }
			other_player_hp_bar.top = y - 20;
			other_player_hp_bar.bottom = y - 17;
			if (other_player_hp < 90) { other_player_hp_color = CreateSolidBrush(RGB(0, 255, 0)); }
			if (other_player_hp <= 50) { other_player_hp_color = CreateSolidBrush(RGB(255, 0, 0)); }
			FillRect(hdc, &other_player_hp_bar, other_player_hp_color);
			other_player_hp_bar.right = x + hp_value_x;
			FrameRect(hdc, &other_player_hp_bar, CreateSolidBrush(RGB(0, 0, 0)));
		}

		/// this player ---------------------------------------------------------------------------------
		dir = &(g_client.getPlayerData()->dir);
		x = g_client.getPlayerData()->pos.x, y = g_client.getPlayerData()->pos.y;
				
		/// player circle
		Ellipse(hdc, x - value, y - value, x + value, y + value);

		/// player direction
		if ((*dir & KEYINPUT_RIGHT)	== (KEYINPUT_RIGHT)){ x += (value + dir_value); }
		if ((*dir & KEYINPUT_LEFT)	== (KEYINPUT_LEFT))	{ x -= (value + dir_value); }
		if ((*dir & KEYINPUT_UP)	== (KEYINPUT_UP))	{ y -= (value + dir_value); }
		if ((*dir & KEYINPUT_DOWN)	== (KEYINPUT_DOWN))	{ y += (value + dir_value); }
		
		/// ������ ���� ��� ���� ���̸�, ���� X ��.. �ƴϸ� ���� ���� ���� �׸���
		if ((100 >= timecheck.end_key_s()) && is_key_s_pushed) {
			MoveToEx(hdc, x - att_value, y - att_value, nullptr);
			LineTo(hdc, x + att_value, y + att_value);
			MoveToEx(hdc, x - att_value, y + att_value, nullptr);
			LineTo(hdc, x + att_value, y - att_value);
		}
		else {
			Ellipse(hdc, x - dir_value, y - dir_value, x + dir_value, y + dir_value);
			is_key_s_pushed = false;
		}
		
		/// HP bar - player
		x = g_client.getPlayerData()->pos.x, y = g_client.getPlayerData()->pos.y;
		int hp = g_client.getPlayerData()->state.hp;
		RECT hp_bar;
		HBRUSH hp_color = CreateSolidBrush(RGB(0, 100, 255));
		hp_bar.left = x - hp_value_x;
		if (1 < hp){ hp_bar.right = hp_bar.left + (hp / 5); }
		hp_bar.top = y - 20;
		hp_bar.bottom = y - 17;
		if (hp < 90) { hp_color = CreateSolidBrush(RGB(0, 255, 0)); }
		if (hp <= 50) { hp_color = CreateSolidBrush(RGB(255, 0, 0)); }
		FillRect(hdc, &hp_bar, hp_color);
		hp_bar.right = x + hp_value_x;
		FrameRect(hdc, &hp_bar, CreateSolidBrush(RGB(0, 0, 0)));

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SOCKET:
		g_client.ProcessWinMessage(hWnd, uMsg, wParam, lParam);
		return 0;
		break;

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



void move_up() {
	g_client.getPlayerData()->pos.y -= value;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir | KEYINPUT_UP;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_DOWN);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_RIGHT);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_LEFT);
}
void move_down() {
	g_client.getPlayerData()->pos.y += value;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir | KEYINPUT_DOWN;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_UP);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_RIGHT);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_LEFT);
}
void move_left() {
	g_client.getPlayerData()->pos.x -= value;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir | KEYINPUT_LEFT;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_UP);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_DOWN);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_RIGHT);
}
void move_right() {
	g_client.getPlayerData()->pos.x += value;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir | KEYINPUT_RIGHT;
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_UP);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_DOWN);
	g_client.getPlayerData()->dir = g_client.getPlayerData()->dir & (~KEYINPUT_LEFT);
}