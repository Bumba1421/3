//Автор(с): Кудуштеев Алексей
#if !defined(_UTIL_KUDUSHTEEV_H_)
#define _UTIL_KUDUSHTEEV_H_
#ifdef _MSC_VER
#pragma once
#endif

typedef struct scanvas {
	HDC     mdc;
	HBITMAP hbm;
	int     width;
	int     height;
} canvas_t;

typedef struct {
	int x;
	int y;
	int size;
	COLORREF colorA;
	COLORREF colorB;
	COLORREF colorC;
} square_t;

extern _Bool canvas_create(canvas_t* dc, int width, int height);
extern void canvas_destroy(canvas_t* dc);
extern canvas_t* canvas_get(void);
extern void setDelay(DWORD delay);
extern DWORD getTickCount(void);

extern HFONT create_font(LPCWSTR face, int size, int* height);
extern void draw_dword(HDC hDC, int x, int y, DWORD num);

extern void put_square(const canvas_t* can, const square_t* sq);
extern void fill_background(const canvas_t* can, COLORREF colorA, COLORREF colorB);

extern COLORREF color_lerp(COLORREF colorA, COLORREF colorB, float fn);
extern HBRUSH gradient_brush(int width, int height, COLORREF colorA, COLORREF colorB);
extern int    count_len(const char* s, int c);

extern HWND  window_create(HINSTANCE, LPCWSTR, int, int);
extern int   window_run(HWND);
extern void __fastcall onPaint(const canvas_t* hdc);
extern void __fastcall onCreate(HWND);
extern void __fastcall onUpdateGame(const canvas_t* hdc, DWORD tick, LPRECT prc);
extern void __fastcall onKeyDown(UINT, LPRECT prc);
extern void onDestroy(void);

extern BOOL save_level(LPCWSTR filename, DWORD level);
extern BOOL load_level(LPCWSTR filename, DWORD* level);

#endif
