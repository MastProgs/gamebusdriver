// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

// ������ ���õ� ��� ���ϵ�
#pragma comment(lib, "ws2_32")

#include<WinSock2.h>


#include "targetver.h"
#include "resource.h"
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>

// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.

#include <d3d9.h>
#include <d3dx9.h>

#include <d3d11.h>
#include <d3dx11.h>
//#include "..\..\..\DX9\D3DX10Math.h"
//#include "..\..\..\DX9\d3dx9math.h"
#include <d3dcompiler.h>
#include <xnamath.h>

#include <fbxsdk.h>

// #include "d3dx11effect.h"

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#include <vector>
#include <list>
#include <map>
#include <string>
#include <ctime>
#include <iostream>
#include <process.h>
#include <assert.h>
#include <unordered_map>

#include "Include.h"

using namespace std;

// ������ ���õ� ��������
#include"../../../../server/serverBoostModel/serverBoostModel/protocol.h"

// client ��� ���� class
#include<fstream>
#include"../Server_Code/ClientClass.h"

extern AsynchronousClientClass g_client;

#pragma warning(disable : 4005)
#include <crtdbg.h>

#ifdef _DEBUG
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
#endif