#include <windows.h>
#include <tchar.h>


#define TIMER1 1001
#define PLAYMODE 0
#define MENUMODE 1
#define LOSTMODE 2
#define HEIGHT 554

#define PIPESTARTPOSITIONX 1400
#define PIPESTARTPOSITIONY 100
#define PIPEHEIGHT 650
#define PIPEWIDTH 90

#define PLAYERWIDTH 76
#define PLAYERHEIGHT 48
#define PLAYERSTARTPOSITIONX 100
#define PLAYERSTARTPOSITIONY 100

#define GAPHEIGHT 180
#define MARGIN 50

wchar_t LostText1[20] = L"GAME OVER";
wchar_t ScoreText[30] = L"Your score:";
wchar_t PressSpaceText[30] = L"Press space key to continue";

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex; HWND hWnd; MSG msg;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T("FlappyBirdClass");
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);
	hWnd = CreateWindow(_T("FlappyBirdClass"), _T("Flappy bird"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
HBITMAP PlayerBmp = NULL;
HBITMAP Mask = NULL;
HBITMAP Pipe = NULL;
HBITMAP PipeMask = NULL;
HBITMAP InvPipe = NULL;
HBITMAP InvPipeMask = NULL;
HBITMAP Background = NULL;
RECT pipe;
RECT WindowPosition;
RECT Pipes[4];
RECT Player;
int BkX1=0;
int BkX2=1165;
int IsFirstRun = 1;
int HorSpeed = 8;
int VertSpeed = 10;
int GameMode = MENUMODE;
int Score;
BOOL IsTimed = 0;
wchar_t scoretext[12];

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent)
{
	HDC hdcMem, hdcMem2;
	HBITMAP hbmMask;
	BITMAP bm;

	//делаем одноцветную маску
	GetObject(hbmColour, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	hdcMem = CreateCompatibleDC(0);
	hdcMem2 = CreateCompatibleDC(0);

	SelectObject(hdcMem, hbmColour);
	SelectObject(hdcMem2, hbmMask);

	//фоновым делаем выбранный цвет
	SetBkColor(hdcMem, crTransparent);
	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY); //фоновый делаем белый, остальное - чёрное


	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);//фоновый делаем чёрный
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

	return hbmMask;
}
int IsColliding()
{
	if ((Player.top < 0) || (Player.bottom > HEIGHT)) return 1;
	for (int i = 0;i < 4;i++) {
		if (((Player.left > Pipes[i].left) && (Player.left < Pipes[i].right)) ||
			(Player.right > Pipes[i].left) && (Player.right < Pipes[i].right)) 
		{
			if ((Player.top < Pipes[i].top) ||
				(Player.bottom > Pipes[i].bottom))
			{
				return 1;
			}
		}
	}
	return 0;
}
void InitPipe(LPRECT rect)
{
	int tmp = rand() % (HEIGHT - (GAPHEIGHT+2*MARGIN)) + MARGIN;
	SetRect(rect, PIPESTARTPOSITIONX, tmp, PIPESTARTPOSITIONX + PIPEWIDTH, tmp + GAPHEIGHT);
}
void DrawBmpToBufferDC(HDC BufferDC, int x, int y, int width, int height, HBITMAP bmp, DWORD mode)
{
	HDC BmpContainerDC = CreateCompatibleDC(BufferDC);
	SelectObject(BmpContainerDC, bmp);
	BitBlt(BufferDC, x, y, width, height, BmpContainerDC, 0, 0, mode);
	DeleteObject(BmpContainerDC);
}
void PrepareBackground(HWND hWnd, LPPAINTSTRUCT lpPS, HDC& BufferDC, HBITMAP& BufferBackgroundBmp) 
{

	RECT ClientRectangle;
	GetClientRect(hWnd, &ClientRectangle);
	BufferDC = CreateCompatibleDC(lpPS->hdc);
	BufferBackgroundBmp = CreateCompatibleBitmap(lpPS->hdc, ClientRectangle.right - ClientRectangle.left, ClientRectangle.bottom - ClientRectangle.top);
	SelectObject(BufferDC, BufferBackgroundBmp);
}
void ClearDrawing(HWND hWnd, LPPAINTSTRUCT lpPS, HDC& BufferDC, HBITMAP& BufferBackgroundBmp)
{
	RECT ClientRectangle;
	GetClientRect(hWnd, &ClientRectangle);
	BitBlt(lpPS->hdc, ClientRectangle.left, ClientRectangle.top, ClientRectangle.right - ClientRectangle.left, ClientRectangle.bottom - ClientRectangle.top, BufferDC, 0, 0, SRCCOPY);
	DeleteObject(BufferBackgroundBmp);
	DeleteDC(BufferDC);
}
void DrawPipe(HDC BufferDC,LPRECT pipe)
{
	DrawBmpToBufferDC(BufferDC, pipe->left, pipe->top-PIPEHEIGHT, PIPEWIDTH, PIPEHEIGHT, InvPipeMask, SRCAND);
	DrawBmpToBufferDC(BufferDC, pipe->left, pipe->top-PIPEHEIGHT, PIPEWIDTH, PIPEHEIGHT, InvPipe, SRCPAINT);
	DrawBmpToBufferDC(BufferDC, pipe->left, pipe->bottom, PIPEWIDTH, PIPEHEIGHT, PipeMask, SRCAND);
	DrawBmpToBufferDC(BufferDC, pipe->left, pipe->bottom, PIPEWIDTH, PIPEHEIGHT, Pipe, SRCPAINT);
}
void ResetGame()
{
	Score = 0;
	SetRect(&Player, PLAYERSTARTPOSITIONX, PLAYERSTARTPOSITIONY, PLAYERSTARTPOSITIONX + PLAYERWIDTH, PLAYERSTARTPOSITIONY + PLAYERHEIGHT);
	for (int i = 0;i < 4;i++) {
		InitPipe(&Pipes[i]);
		Pipes[i].left += 350 * i;
		Pipes[i].right += 350 * i;
	}
}
void LoadBitmaps()
{
	PlayerBmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), L"Picture2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	Mask = CreateBitmapMask(PlayerBmp, RGB(0, 255, 0));
	Pipe = (HBITMAP)LoadImage(GetModuleHandle(NULL), L"pipe.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	PipeMask = CreateBitmapMask(Pipe, RGB(246, 246, 246));
	InvPipe = (HBITMAP)LoadImage(GetModuleHandle(NULL), L"pipeinverse.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	InvPipeMask = CreateBitmapMask(InvPipe, RGB(246, 246, 246));
	Background = (HBITMAP)LoadImage(GetModuleHandle(NULL), L"background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hDC,BufferDC=NULL;
	HFONT NewFont=NULL, OldFont=NULL;
	PAINTSTRUCT ps;
	HBITMAP BufferBackgroundBmp=NULL;
	RECT tmp;
	switch (message)
	{
	case WM_PAINT:
		
		if (IsColliding()) {
			IsTimed = FALSE;
			KillTimer(hWnd, TIMER1);
			GameMode = LOSTMODE;
		}

		hDC = BeginPaint(hWnd, &ps);
		PrepareBackground(hWnd, &ps, BufferDC, BufferBackgroundBmp);
		DrawBmpToBufferDC(BufferDC, BkX1, 0, 1165, 557, Background, SRCPAINT);
		DrawBmpToBufferDC(BufferDC, BkX2, 0, 1165, 557, Background, SRCPAINT);
		DrawBmpToBufferDC(BufferDC, Player.left, Player.top, PLAYERWIDTH, PLAYERHEIGHT, Mask, SRCAND);
		DrawBmpToBufferDC(BufferDC, Player.left, Player.top, PLAYERWIDTH, PLAYERHEIGHT, PlayerBmp, SRCPAINT);
		for (int i = 0;i < 4;i++) {
			DrawPipe(BufferDC, &Pipes[i]);
		}
		NewFont = CreateFont(40, 20, 0, 0, 700, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, (L"Arial"));
		OldFont = (HFONT)SelectObject(BufferDC, NewFont);
		SetBkMode(BufferDC, TRANSPARENT);
		SetTextColor(BufferDC, RGB(255, 0, 0));
		SetRect(&tmp, 1000, 20, 1100, 60);
		_itow_s(Score, scoretext, sizeof(scoretext) / 2, 10);
		DrawText(BufferDC, scoretext, -1, &tmp, DT_WORDBREAK);


		if (GameMode == MENUMODE) {
			SetRect(&tmp, 300, 250, 900, 290);
			DrawText(BufferDC, PressSpaceText, -1, &tmp, DT_WORDBREAK);
		}
		if (GameMode == LOSTMODE) 
		{
			SetRect(&tmp, 400, 200, 700, 240);
			DrawText(BufferDC, LostText1, -1, &tmp, DT_WORDBREAK);
			wchar_t tmp1[42]=L"";
			wcscat_s(tmp1, ScoreText);
			wcscat_s(tmp1, scoretext);
			SetRect(&tmp, 400, 240, 700, 280);
			DrawText(BufferDC, tmp1, -1, &tmp, DT_WORDBREAK);
			SetRect(&tmp, 300, 280, 900, 320);
			DrawText(BufferDC, PressSpaceText, -1, &tmp, DT_WORDBREAK);

		}
		SelectObject(BufferDC, OldFont);
		DeleteObject(NewFont);
		ClearDrawing(hWnd, &ps, BufferDC, BufferBackgroundBmp);
		EndPaint(hWnd, &ps);
		break;
	case WM_CHAR:
		GetClientRect(hWnd, &tmp);
		InvalidateRect(hWnd, &tmp, FALSE);
		switch (wParam)
		{
		case ' ':
			if (GameMode == PLAYMODE)
			{
				VertSpeed = -10;
			}
			if (GameMode == MENUMODE) {
				GameMode = PLAYMODE;
				IsTimed = TRUE;
				SetTimer(hWnd, TIMER1, 30, (TIMERPROC)NULL);
			}
			if (GameMode == LOSTMODE)
			{
				GameMode = PLAYMODE;
				ResetGame();
				IsTimed = TRUE;
				SetTimer(hWnd, TIMER1, 30, (TIMERPROC)NULL);
			}
			break;
		case 'p':
			if (!IsTimed) {
				GameMode = PLAYMODE;
				IsTimed = TRUE;
				SetTimer(hWnd, TIMER1, 30, (TIMERPROC)NULL);
			}
			else {
				GameMode = MENUMODE;
				IsTimed = FALSE;
				KillTimer(hWnd, TIMER1);
			}
		}
		break;
	case WM_TIMER:
		if (wParam == TIMER1)
		{
			GetClientRect(hWnd, &tmp);
			InvalidateRect(hWnd, &tmp, FALSE);
			Player.top += VertSpeed;
			Player.bottom += VertSpeed;
			for (int i = 0;i < 4;i++) {
				if (Pipes[i].right < 0) {
					InitPipe(&Pipes[i]);
					Score++;
				}
				Pipes[i].left -= HorSpeed;
				Pipes[i].right -= HorSpeed;
			}
			if (BkX1 < -1165)BkX1 +=1165*2;
			if (BkX2 < -1165)BkX2 +=1165*2;
			BkX1 -= HorSpeed;
			BkX2 -= HorSpeed;
			if (VertSpeed < 10) VertSpeed++;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		LoadBitmaps();
		ResetGame();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
