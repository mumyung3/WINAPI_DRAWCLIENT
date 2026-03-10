// WindowsProject1.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "WindowsProject1.h"
#include "stdio.h"
#include <windowsx.h>  // ← 이거 추가!

#define MAX_LOADSTRING 100
#define GRID_SIZE 16
#define GRID_WIDTH 100
#define GRID_HEIGHT 50

HBRUSH	g_hTileBrush;
HPEN	g_hGridPen;
char 	g_Tile[GRID_HEIGHT][GRID_WIDTH];  		 // 0 장애물 없음 / 1 장애물 있음

bool g_bErase = false;
bool g_bDrag = false;

// 전역
HBITMAP 	g_hMemDCBitmap;
HBITMAP 	g_hMemDCBitmap_old;
HDC		g_hMemDC;
RECT		g_MemDCRect;


// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

BOOL	g_bClick;		// 마우스 클릭여부 판단
HPEN	g_hPen;		// 현재 생성된 펜,  휠버튼 클릭시 새롭게 생성
int	g_iOldX;		// 마우스 드래그 표현을 위한 마우스 이동시 이전 좌표
int	g_iOldY; 		// 마우스 드래그 표현을 위한 마우스 이동시 이전 좌표


// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void RenderObstacle(HDC hdc)
{
	int iX = 0;
	int iY = 0;
	HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, g_hTileBrush);
	SelectObject(hdc, GetStockObject(NULL_PEN));
	// 사각형의 테두리를 안보이도록 하기 위해 Null Pen 을 지정한다.
	// CreatePen 으로 NULL PEN 을 생성해도 되지만, GetStockObject 를 사용하여 
	// 이미 시스템에 만들어져 있는 고정 GDI Object 를 사용해본다.
	// GetStockObject 는 시스템의 고정적인 범용 GDI Object 로서 삭제가 필요 없다.
	// 시스템 전역적인 GDI Object 를 얻어서 사용한다는 개념.
	for (int iCntW = 0; iCntW < GRID_WIDTH; iCntW++)
	{
		for (int iCntH = 0; iCntH < GRID_HEIGHT; iCntH++)
		{
			if (g_Tile[iCntH][iCntW])
			{
				iX = iCntW * GRID_SIZE;
				iY = iCntH * GRID_SIZE;
				// 테두리 크기가 있으므로 + 2 한다
				Rectangle(hdc, iX, iY, iX + GRID_SIZE + 2, iY + GRID_SIZE + 2);
			}
		}
	}
	SelectObject(hdc, hOldBrush);
}

void RenderGrid(HDC hdc)
{
	int iX = 0;
	int iY = 0;
	HPEN hOldPen = (HPEN)SelectObject(hdc, g_hGridPen);
	// 그리드의 마지막 선을 추가로 그리기 위해 <= 의 반복 조건.
	for (int iCntW = 0; iCntW <= GRID_WIDTH; iCntW++)
	{
		MoveToEx(hdc, iX, 0, NULL);
		LineTo(hdc, iX, GRID_HEIGHT * GRID_SIZE);
		iX += GRID_SIZE;
	}
	for (int iCntH = 0; iCntH <= GRID_HEIGHT; iCntH++)
	{
		MoveToEx(hdc, 0, iY, NULL);
		LineTo(hdc, GRID_WIDTH * GRID_SIZE, iY);
		iY += GRID_SIZE;
	}
	SelectObject(hdc, hOldPen);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{


	//// 메시지박스로 확인
	//MessageBox(NULL, lpCmdLine, L"명령줄 인수", MB_OK);

	//// 또는 디버거 출력
	//OutputDebugString(L"CommandLine: ");
	//OutputDebugString(lpCmdLine);
	//OutputDebugString(L"\n");




	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 애플리케이션 초기화를 수행합니다:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

	MSG msg;

	// 기본 메시지 루프입니다:
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



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
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

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

int g_x = 0;
VOID CALLBACK TimerProc(HWND hWnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
	// 타이머마다 실행할 코드
	HDC hdc = GetDC(hWnd);
	TextOut(hdc, 10 + (g_x++), 10, L"zsdasd!", 6);
	ReleaseDC(hWnd, hdc);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	switch (message)
	{
	case WM_LBUTTONDOWN:
		g_bDrag = true;
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			int iTileX = xPos / GRID_SIZE;
			int iTileY = yPos / GRID_SIZE;
			// 첫 선택 타일이 장애물이면 지우기 모드 아니면 장애물 넣기 모드
			if (g_Tile[iTileY][iTileX] == 1)
				g_bErase = true;
			else
				g_bErase = false;
		}
		break;
	case WM_LBUTTONUP:
		g_bDrag = false;
		break;

	case WM_MOUSEMOVE:
	{
		if (g_bDrag)
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);

			int iTileX = xPos / GRID_SIZE;
			int iTileY = yPos / GRID_SIZE;

			g_Tile[iTileY][iTileX] = !g_bErase;
			InvalidateRect(hWnd, NULL, false);

		}
	}
	break;

	case WM_CREATE:
	{
		// 윈도우 생성시 현 윈도우 크기와 동일한 메모리 DC 생성
		HDC hdc = GetDC(hWnd);
		GetClientRect(hWnd, &g_MemDCRect);
		g_hMemDCBitmap = CreateCompatibleBitmap(hdc, g_MemDCRect.right, g_MemDCRect.bottom);
		g_hMemDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
		g_hMemDCBitmap_old = (HBITMAP)SelectObject(g_hMemDC, g_hMemDCBitmap);

		// ★ 추가: GDI 객체 생성
		g_hTileBrush = CreateSolidBrush(RGB(255, 0, 0));  // 빨간색
		g_hGridPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));  // 회색
	}
	break;


	case WM_PAINT:
	{
		// 메모리 DC 를 클리어 하고
		PatBlt(g_hMemDC, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, WHITENESS);

		//, RenderObstacle, RenderGrid 를 메모리 DC 에 출력
		RenderObstacle(g_hMemDC);
		RenderGrid(g_hMemDC);

		// 메모리 DC 에 랜더링이 끝나면, 메모리 DC -> 윈도우 DC 로의 출력
		hdc = BeginPaint(hWnd, &ps);
		BitBlt(hdc, 0, 0, g_MemDCRect.right, g_MemDCRect.bottom, g_hMemDC, 0, 0, SRCCOPY);
		EndPaint(hWnd, &ps);
		break;
	}
	break;

	case WM_DESTROY:
		SelectObject(g_hMemDC, g_hMemDCBitmap_old);
		DeleteObject(g_hMemDC);
		DeleteObject(g_hMemDCBitmap);
		DeleteObject(g_hTileBrush);
		DeleteObject(g_hGridPen);
		PostQuitMessage(0);
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


	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
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
