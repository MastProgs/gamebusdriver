// stdafx.cpp : ǥ�� ���� ���ϸ� ��� �ִ� �ҽ� �����Դϴ�.
// Only_Console_Client_for_TEST.pch�� �̸� �����ϵ� ����� �˴ϴ�.
// stdafx.obj���� �̸� �����ϵ� ���� ������ ���Ե˴ϴ�.

#include "stdafx.h"

// TODO: �ʿ��� �߰� �����
// �� ������ �ƴ� STDAFX.H���� �����մϴ�.

LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SOCKET:
		client.ProcessWinMessage(hwnd, uMsg, wParam, lParam);
		return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	default:
		cout << "CALLBACK WndProc ERROR\n";
		exit(-1);
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}