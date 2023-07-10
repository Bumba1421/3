//Автор(с): Кудуштеев Алексей
#include "headers.h"
#define TIMER_ID_GAME     777
#define TIMER_DELAY_GAME  10
static const WCHAR g_name[] = L"WinTetris";
static canvas_t    g_memdc  = {0};
static DWORD       g_delay  = 10UL;
static DWORD       g_tick   = 0UL;


//обработчик сообщений
static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	RECT rc;
	static DWORD last_time = 0UL;

	switch(msg){
	case WM_KEYDOWN:
		onKeyDown((UINT)wParam, &rc);
		InvalidateRect(hwnd, &rc, TRUE);
		break;
	case WM_TIMER:
		if(LOWORD(wParam) == TIMER_ID_GAME){
			DWORD cur = GetTickCount();
			g_tick    = cur;

			if(cur > last_time){
				last_time = cur + g_delay;
				onUpdateGame(&g_memdc, cur, &rc);
				InvalidateRect(hwnd, &rc, TRUE);
			}
		}
		break;
	case WM_ERASEBKGND:
		onPaint(&g_memdc);
		BitBlt((HDC)wParam, 0, 0, g_memdc.width, g_memdc.height, g_memdc.mdc, 0, 0, SRCCOPY);
		return TRUE;
	case WM_DESTROY:
		KillTimer(hwnd, TIMER_ID_GAME);
		onDestroy();
		canvas_destroy(&g_memdc);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}
	return 0;
}


// функция создаёт окно...
HWND window_create(HINSTANCE inst, LPCWSTR caption, int width, int height){
	WNDCLASSEXW cls = {
		.cbSize        = sizeof(WNDCLASSEXW),
		.style         = 0,
		.lpfnWndProc   = (WNDPROC)window_proc,
		.hInstance     = inst,
		.hIcon         = LoadIconW(NULL, MAKEINTRESOURCEW(IDI_APPLICATION)),
		.hCursor       = LoadCursorW(NULL, MAKEINTRESOURCEW(IDC_ARROW)),
		.lpszClassName = g_name,
		.hIconSm       = NULL,
		.hbrBackground = NULL,
		.lpszMenuName  = NULL,
		.cbWndExtra    = 0,
		.cbClsExtra    = 0
	};

	if(!RegisterClassExW(&cls))
		return NULL;

	DWORD style = WS_OVERLAPPEDWINDOW & ~(WS_SIZEBOX | WS_MAXIMIZEBOX);
	RECT rc;
	SetRect(&rc, 0, 0, width, height);
	AdjustWindowRectEx(&rc, style, FALSE, WS_EX_APPWINDOW);
	rc.right  -= rc.left;
	rc.bottom -= rc.top;

	int sx = GetSystemMetrics(SM_CXSCREEN);
	int sy = GetSystemMetrics(SM_CYSCREEN);

	HWND hwnd = CreateWindowExW(WS_EX_APPWINDOW, g_name, caption, style, (sx - rc.right)/2,
	                            (sy - rc.bottom)/2, rc.right, rc.bottom, NULL, NULL, inst, NULL);
	if(hwnd == NULL){
		UnregisterClassW(g_name, inst);
		return NULL;
	}

	canvas_create(&g_memdc, width, height);
	onCreate(hwnd);
	SetTimer(hwnd, TIMER_ID_GAME, TIMER_DELAY_GAME, NULL);
	return hwnd;
}


//цикл ожидание сообщений
int window_run(HWND hwnd){
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	MSG msg = {0};
	while(GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	UnregisterClassW(g_name, GetModuleHandleW(NULL));
	return (int)msg.wParam;
}


//---------------------------------------------------------------------------------------------

//создание холста
_Bool canvas_create(canvas_t* dc, int width, int height){
	canvas_destroy(dc);
	HDC hdc = GetDC(NULL);
	dc->mdc = CreateCompatibleDC(hdc);
	if(dc->mdc == NULL)
		return false;

	dc->hbm = CreateCompatibleBitmap(hdc, width, height);
	if(dc->hbm == NULL){
		canvas_destroy(dc);
		return false;
	}
	SelectObject(dc->mdc, dc->hbm);
	PatBlt(dc->mdc, 0, 0, width, height, BLACKNESS);
	dc->width  = width;
	dc->height = height;
	return true;
}


//уничтожение холста
void canvas_destroy(canvas_t* dc){
	if(dc->hbm != NULL)
		DeleteObject(dc->hbm);
	dc->hbm = NULL;

	if(dc->mdc != NULL)
		DeleteDC(dc->mdc);
	dc->mdc = NULL;

	dc->width = dc->height = 0;
}


canvas_t* canvas_get(void){
	return &g_memdc;
}


//задание для задержки(для перерисовки)
void setDelay(DWORD delay){
	g_delay = delay;
}


DWORD getTickCount(void){
	return g_tick;
}


//выбор шрифта
HFONT create_font(LPCWSTR face, int fsize, int* height){
	HDC   hdc  = GetDC(NULL);
	int   size = -MulDiv(fsize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	HFONT font = CreateFontW(size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, RUSSIAN_CHARSET,
	                         OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, face);
	if(font != NULL){
		HGDIOBJ old = SelectObject(hdc, font);
		TEXTMETRICW tm;
		if(GetTextMetricsW(hdc, &tm))
			*height = tm.tmHeight;
		SelectObject(hdc, old);
	}
	ReleaseDC(NULL, hdc);
	return font;
}


//вывод безнаковых чисел
void draw_dword(HDC hDC, int x, int y, DWORD num){
	WCHAR sb[16];
	int   cnt = 0;
	do {
		sb[cnt++] = (WCHAR)(num % 10) + L'0';
	} while((num /= 10) != 0);

	WCHAR t, *e = &sb[cnt - 1];
	for(WCHAR* p = &sb[0]; p < e; ++p, --e){
		t  = *p;
		*p = *e;
		*e = t;
	}
	TextOutW(hDC, x, y, sb, cnt);
}


//интерполяция
COLORREF color_lerp(COLORREF colorA, COLORREF colorB, float fn){
	fn    = max(min(fn, 1.0f), 0.0f);
	int r = (int)GetRValue(colorA);
	int g = (int)GetGValue(colorA);
	int b = (int)GetBValue(colorA);
	r = r + (int)((float)(GetRValue(colorB) - r) * fn);
	g = g + (int)((float)(GetGValue(colorB) - g) * fn);
	b = b + (int)((float)(GetBValue(colorB) - b) * fn);
	return RGB(r, g, b);
}

#define line(hdc, x1, y1, x2, y2)  MoveToEx(hdc, (x1), (y1), NULL); LineTo(hdc, (x2), (y2))

//рисование фигуры
void put_square(const canvas_t* can, const square_t* sq){
	int       x = sq->x;
	int       y = sq->y;
	int    size = sq->size;
	HDC    hdc  = can->mdc;
	int    edge = (int)((float)size * 0.15f);
	HBRUSH  hbr = CreateSolidBrush(sq->colorA);

	RECT rc = { x, y, x + size, y + size };
	FillRect(hdc, &rc, hbr);
	DeleteObject(hbr);

	HPEN    pen = CreatePen(PS_SOLID, 1, sq->colorB);
	HGDIOBJ old = SelectObject(hdc, pen);
	for(int i = 0; i < edge; ++i){
		line(hdc, x + i, y + i, x + i, y + size - i);
		line(hdc, x, y + i, x + size, y + i);
	}
	DeleteObject(pen);

	pen = CreatePen(PS_SOLID, 1, sq->colorC);
	SelectObject(hdc, pen);
	for(int i = 0; i < edge; ++i){
		line(hdc, x + i, y + size - i, x + size - i, y + size - i);
		line(hdc, x + size - i, y + i, x + size - i, y + size - i);
	}
	DeleteObject(pen);
	SelectObject(hdc, old);

	BitBlt(hdc, x, y + size, size, size, hdc, x, y, SRCCOPY);
	const int row = y + size * 2;
	const int col = x + size;
	const COLORREF black = RGB(0, 0, 0);
	for(int i = y + size; i < row; ++i){
		for(int j = x; j < col; ++j){
			COLORREF c = GetPixel(hdc, j, i);
			SetPixelV(hdc, j, i, color_lerp(black, c, 0.7f) );
		}
	}
}


//градиентная заливка
void fill_background(const canvas_t* can, COLORREF colorA, COLORREF colorB){
	const float inc = 1.0f / (can->height - 1);
	HGDIOBJ old = SelectObject(can->mdc, (HPEN)GetStockObject(DC_PEN));
	float     p = 0.0f;
	for(int i = 0; i < can->height; ++i, p += inc){
		SetDCPenColor(can->mdc, color_lerp(colorA, colorB, p));
		line(can->mdc, 0, i, can->width, i);
	}
	SelectObject(can->mdc, old);
}


//создание градиентной кисти
HBRUSH gradient_brush(int width, int height, COLORREF colorA, COLORREF colorB){
	canvas_t can;
	if(!canvas_create(&can, width, height))
		return NULL;

	fill_background(&can, colorA, colorB);

	HBRUSH hbr = CreatePatternBrush(can.hbm);
	canvas_destroy(&can);
	return hbr;
}


//кол-во до n
int count_len(const char* s, int c){
	const char* p = s;
	while(*s && (*s != c))
		++s;
	return (int)(s - p);
}


BOOL save_level(LPCWSTR filename, DWORD level){
	HANDLE fp = CreateFileW(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fp == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD   num = 0;
	DWORD32 lvl = (DWORD32)level;
	BOOL    res = WriteFile(fp, (LPCVOID)&lvl, sizeof(lvl), &num, NULL);
	FlushFileBuffers(fp);
	CloseHandle(fp);
	return (res && (num == sizeof(lvl)));
}


BOOL load_level(LPCWSTR filename, DWORD* level){
	*level    = 0;
	HANDLE fp = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(fp == INVALID_HANDLE_VALUE)
		return FALSE;

	DWORD   num = 0;
	DWORD32 lvl = 0;
	BOOL    res = ReadFile(fp, (LPVOID)&lvl, sizeof(lvl), &num, NULL);
	CloseHandle(fp);

	if(res && (num == sizeof(lvl))){
		*level = (DWORD)lvl;
		return TRUE;
	}
	return FALSE;
}

