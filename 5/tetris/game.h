//Автор(с): Кудуштеев Алексей
#if !defined(_GAME_TETRIS_KUDUSHTEEV_H_)
#define _GAME_TETRIS_KUDUSHTEEV_H_
#ifdef _MSC_VER
#pragma once
#endif
#define MAX_SIZE_BRUSH  4

extern _Bool game_run(HINSTANCE hinst, LPCWSTR caption);


enum state_game {
	GAME_PLAY  = 0,
	GAME_MENU  = 1,
	GAME_LEVEL = 2,
	GAME_PAUSE = 3,
	GAME_OVER  = 4,
	GAME_NONE  = 5
};


typedef struct {
	HWND            hwnd;
	DWORD           lines;
	DWORD           level;
	DWORD           bonus;
	HBRUSH          bgcolor;
	HBRUSH          bgnext;
	HBRUSH          colors[MAX_SIZE_BRUSH];
	HFONT           font;
	int             font_height;
	canvas_t        tiles;
	canvas_t        iback;
	int             cell_size;
	int             figure_cur;
	int             cur_row;
	int             cur_col;
	enum state_game state;
	enum state_game pstate;
	field_t         field;
	figure_t        figure1;
	figure_t        figure2;
	DWORD           rem_time;
	int             rem_row1;
	int             rem_row2;
	_Bool           once;
	_Bool           rem_ok;
} game_t;


enum state_update {
	st_field = 0, st_window = 1
};

#endif
