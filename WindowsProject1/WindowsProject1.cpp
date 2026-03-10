#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "framework.h"
#include "WindowsProject1.h"
#include <windowsx.h>

// 네트워크 코드

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <ws2tcpip.h>
#include "ringbuffer.h"

#define MAX_LOADSTRING 100

// 네트워크 코드
#define SERVERPORT 25000
#define SERVERIP L"127.0.0.1"
#define WM_SOCKET (WM_USER + 1)

struct stHEADER {

	unsigned short Len;
};


//패딩은 맞음.
#pragma pack(push, 1)
struct st_DRAW_PACKET {
	stHEADER iHeader;
	int iStartX;
	int iStartY;
	int iEndX;
	int iEndY;
};
#pragma pack(pop)


// 네트워크 전역 변수
SOCKET g_sock;
RingBuffer recvQ;
RingBuffer sendQ;
bool g_bConnected = false;

// 전역 변수
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// 메모리 DC
HBITMAP g_hMemDCBitmap;
HBITMAP g_hMemDCBitmap_old;
HDC     g_hMemDC;
RECT    g_MemDCRect;

// 드로잉
bool  g_bDrag = false;
// 드로잉
st_DRAW_PACKET g_Lines[100000];
int   g_iLineCount = 0;
int   g_iOldX = 0;
int   g_iOldY = 0;

// 함수 선언
ATOM             MyRegisterClass(HINSTANCE hInstance);
BOOL             InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
		return FALSE;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
	if (!hWnd) return FALSE;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_LBUTTONDOWN:
		g_bDrag = true;
		g_iOldX = GET_X_LPARAM(lParam);
		g_iOldY = GET_Y_LPARAM(lParam);
		break;

	case WM_LBUTTONUP:
		g_bDrag = false;
		break;
	case WM_MOUSEMOVE:
		if (g_bDrag && g_bConnected) {
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);

			stHEADER header = { sizeof(st_DRAW_PACKET) -sizeof(stHEADER)};
			st_DRAW_PACKET packet = { header,g_iOldX, g_iOldY, xPos, yPos };
			
			// 1. 일단 큐에 넣기
			sendQ.Enqueue((char*)&packet, sizeof(packet));


			char localBuf[BUFSIZE];
			int sendSize = sendQ.GetUseSize();
			sendQ.Peek(localBuf, sendSize);

			int retSend = send(g_sock, localBuf, sendSize, 0);

			if (retSend == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					closesocket(g_sock);
				}			
			}
			else {
				sendQ.MoveFront(retSend);
			}

			g_iOldX = xPos;
			g_iOldY = yPos;
		}
		break;

	case WM_CREATE:
	{
		HDC hdc = GetDC(hWnd);
		GetClientRect(hWnd, &g_MemDCRect);
		g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);

		//네트워크 
		// 
		// 윈속 초기화
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);

		// 소켓 생성
		g_sock = socket(AF_INET, SOCK_STREAM, 0);
		
		// 서버 연결
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		InetPton(AF_INET, SERVERIP, &serveraddr.sin_addr);
		serveraddr.sin_port = htons(SERVERPORT);


		// WSAAsyncSelect 등록
		WSAAsyncSelect(g_sock, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);

		// connect()
		connect(g_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	
	}
	break;

	case WM_SOCKET:
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_CONNECT:
			if (WSAGETSELECTERROR(lParam)) {
				// 연결 실패 처리
				closesocket(g_sock);
				return 1;
			}
			g_bConnected = true;

			break;
		case FD_READ:
			// 패킷 수신 → g_Points에 저장 → InvalidateRect
		{
			if (WSAGETSELECTERROR(lParam)) {
				closesocket(g_sock);
				break;
			}

			char localBuf[BUFSIZE];

			int retRecv = recv(g_sock, (char*)localBuf, BUFSIZE, 0);
			if (retRecv == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK)
					closesocket(g_sock);
				break;
			}
			recvQ.Enqueue(localBuf, retRecv);
			while (recvQ.GetUseSize() >= sizeof(st_DRAW_PACKET)) {
				st_DRAW_PACKET packet;
				recvQ.Dequeue((char*)&packet, sizeof(packet));
				
				// 선분 단위로 저장
				g_Lines[g_iLineCount++] = packet;
				InvalidateRect(hWnd, NULL, false);
			}

		}
			break;
		case FD_WRITE:
		{
			if (WSAGETSELECTERROR(lParam)) {
				closesocket(g_sock);
				break;
			}

			// 송신 버퍼 여유 생김 (못 보낸 거 이어서 전송)
			if (sendQ.GetUseSize() == 0) break;
			char localBuf[BUFSIZE];
			int sendSize = sendQ.GetUseSize();
			sendQ.Peek(localBuf, sendSize);

			int retSend = send(g_sock, localBuf, sendSize, 0);

			if (retSend == SOCKET_ERROR) {
				if (WSAGetLastError() != WSAEWOULDBLOCK) {
					closesocket(g_sock);
				}
			}
			else {
				sendQ.MoveFront(retSend);
			}
		}
			break;
		case FD_CLOSE:

			// 서버 종료
			closesocket(g_sock);
			break;
		}
		break;

	case WM_PAINT:
	{
		PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);
	
		//그리기
		for (int i = 0; i < g_iLineCount; i++) {
			MoveToEx(g_hMemDC, g_Lines[i].iStartX, g_Lines[i].iStartY, NULL);
			LineTo(g_hMemDC, g_Lines[i].iEndX, g_Lines[i].iEndY);
		}

		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_SIZE:
	{
		SelectObject(g_hMemDC, g_hMemDCBitmap_old);
		DeleteObject(g_hMemDC);
		DeleteObject(g_hMemDCBitmap);
		HDC hdc = GetDC(hWnd);
		GetClientRect(hWnd, &g_MemDCRect);
		g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);
	}
	break;

	case WM_DESTROY:
		SelectObject(g_hMemDC, g_hMemDCBitmap_old);
		DeleteObject(g_hMemDC);
		DeleteObject(g_hMemDCBitmap);
		closesocket(g_sock);
		WSACleanup();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}