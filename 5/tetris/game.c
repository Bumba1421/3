//Автор(с): Кудуштеев Алексей
#include "headers.h"
#define WHITE_COLOR    0
#define DKGRAY_COLOR   1
#define LTGRAY_COLOR   2
#define GRAY_COLOR     3
#define BUF_LEN(s)     (sizeof(s)/sizeof(s[0]) - 1)
#define MAX_COLORS     9
#define DELAY_REMOVE   500UL
#define DELAY_LEVEL    1400UL
#define FINISH_LINES   64UL

static void create_squares(void);
static void tetris_initialize(void);
static void __fastcall draw_status(const canvas_t* can);
static void __fastcall tetris_draw(const canvas_t* can);
static void __fastcall tetris_menu(const canvas_t* can);
static void __fastcall test_game_over(DWORD tick);
static enum state_update __fastcall tetris_update(DWORD tick);
static void __fastcall tetris_move(UINT key);
static void __fastcall remove_rows(DWORD tick);
static void remove_rows_real(void);
static void figure_rotate(void);
static void new_figure(void);
static void str_draw(HDC hdc, LPCWSTR str, int len, int x, int y, DWORD num, COLORREF color);
static game_t* g_app    = NULL;
static int     g_width  = 0;
static int     g_height = 0;
static const WCHAR g_file_level[] = L"level.dat";

typedef void (__fastcall *pfn_draw)(const canvas_t* can);
static pfn_draw g_funs[2] = { &tetris_draw, &tetris_menu };


//запуск игры
_Bool game_run(HINSTANCE hinst, LPCWSTR caption){
	g_app = (game_t*)calloc(1, sizeof(game_t));
	if(g_app == NULL)
		return false;

	int w  = 640, h = 480;
	int cx = GetSystemMetrics(SM_CXSCREEN);
	int cy = GetSystemMetrics(SM_CYSCREEN);
	if((cx > 1024) && (cy > 768)){
		w = 800;
		h = 600;
	}

	int    ret  = ERROR_SUCCESS;
	g_app->hwnd = window_create(hinst, caption, w, h);
	if(g_app->hwnd != NULL)
		ret = window_run(g_app->hwnd);
	return (ret == ERROR_SUCCESS);
}


//создание окна
void __fastcall onCreate(HWND hwnd){
	RECT rc;
	GetClientRect(hwnd, &rc);
	int width  = rc.right  - rc.left;
	int height = rc.bottom - rc.top;

	g_app->cell_size = height / FIELD_SIZE;
	g_width  = width;
	g_height = height;

	canvas_create(&g_app->tiles, MAX_COLORS * g_app->cell_size, g_app->cell_size * 2);
	create_squares();

	canvas_create(&g_app->iback, g_app->cell_size * FIELD_SIZE, height);
	fill_background(&g_app->iback, RGB(0x11, 0x33, 0x55), RGB(0x55, 0x33, 0x11));

	g_app->bgnext  = NULL;
	g_app->bgcolor = CreateSolidBrush(RGB(0, 0x5F, 0));
	g_app->font    = create_font(L"Tahoma", (width == 640) ? 10 : 11, &g_app->font_height);
	g_app->level   = 0;
	g_app->lines   = g_app->bonus = 0;

	g_app->colors[GRAY_COLOR]   = (HBRUSH)GetStockObject(GRAY_BRUSH);
	g_app->colors[WHITE_COLOR]  = (HBRUSH)GetStockObject(WHITE_BRUSH);
	g_app->colors[LTGRAY_COLOR] = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	g_app->colors[DKGRAY_COLOR] = (HBRUSH)GetStockObject(DKGRAY_BRUSH);

	g_app->state = GAME_MENU;

	load_level(g_file_level, &g_app->level);
}


//обработчик клавиш клавиатуры
void __fastcall onKeyDown(UINT key, LPRECT prc){
	if(g_app->state == GAME_PLAY){
		tetris_move(key);
		SetRect(prc, 0, 0, g_app->iback.width, g_app->iback.height);

		switch(key){
		case VK_RETURN:
jmp_next:
			g_app->rem_time = getTickCount() + DELAY_LEVEL;
			g_app->pstate   = GAME_NONE;
			g_app->state    = GAME_LEVEL;
			SetRect(prc, 0, 0, g_width, g_height);
			break;
		case VK_SPACE:
			g_app->pstate = GAME_NONE;
			g_app->state  = GAME_PAUSE;
			SetRect(prc, 0, 0, g_width, g_height);
			break;
		case VK_ESCAPE:
			SendMessageW(g_app->hwnd, WM_CLOSE, 0, 0);
			break;
		}

	} else if(g_app->state == GAME_MENU){

		if(key == VK_RETURN){
			g_app->rem_time = getTickCount() + DELAY_LEVEL;
			g_app->state    = GAME_LEVEL;
		} else if(key == VK_ESCAPE)
			SendMessageW(g_app->hwnd, WM_CLOSE, 0, 0);

		SetRect(prc, 0, 0, g_width, g_height);

	} else if(g_app->state == GAME_PAUSE){

		if(key == VK_RETURN){
			g_app->state = GAME_PLAY;
			draw_status(canvas_get());
		}
		SetRect(prc, 0, 0, g_width, g_height);

	} else if(g_app->state == GAME_OVER){
		if(key == VK_RETURN)
			goto jmp_next;
	}
}


//обновление анимации
void __fastcall onUpdateGame(const canvas_t* hdc, DWORD tick, LPRECT prc){
	if(g_app->state == GAME_PLAY){

		enum state_update state = tetris_update(tick);
		if(state == st_field)
			SetRect(prc, 0, 0, g_app->iback.width, g_app->iback.height);
		else {
			SetRect(prc, 0, 0, g_width, g_height);
			draw_status(hdc);
			remove_rows(tick);
			test_game_over(tick);
		}

	} else {
		SetRect(prc, 0, 0, g_width, g_height);

		switch(g_app->state){
		case GAME_LEVEL:
			if(g_app->rem_time < tick){
				g_app->rem_time = 0;
				tetris_initialize();
			}
			break;
		default:
			break;
		}
	}
}


//перерисовка игры
void __fastcall onPaint(const canvas_t* can){
	(*g_funs[g_app->state > GAME_PLAY])(can);
}


//уничтожения окна
void onDestroy(void){
	if(g_app != NULL){
		DeleteObject(g_app->font);
		DeleteObject(g_app->bgcolor);
		DeleteObject(g_app->bgnext);
		canvas_destroy(&g_app->iback);
		canvas_destroy(&g_app->tiles);
		free(g_app);
	}
	g_app = NULL;
}


//----------------------------------------------------------------------------------------------


//иницилиазция
static void tetris_initialize(void){
	g_app->lines  = g_app->bonus = 0;
	g_app->once   = false;
	g_app->state  = GAME_PLAY;
	g_app->pstate = GAME_NONE;
	g_app->rem_ok = false;

	for(int i = 0; i < FIELD_SIZE; ++i)
		memset(g_app->field.field[i], CELL_NONE, FIELD_SIZE * sizeof(BYTE));

	new_figure();
	setDelay(350 - min(g_app->level * 2, 100));

	build_level(&g_app->field, g_app->level);
	draw_status(canvas_get());

	fill_background(&g_app->iback, RGB(rand() % 100, rand() % 100, rand() % 100), RGB(rand() % 100, rand() % 100, rand() % 100));
}


//обновление игры
static enum state_update __fastcall tetris_update(DWORD tick){
	enum state_update state = st_field;

	_Bool ret = is_figure_vert(&g_app->field, &g_app->figure1, g_app->cur_row + 1, g_app->cur_col);
	if(ret)
		++g_app->cur_row;
	else {
		field_put_figure(&g_app->field, &g_app->figure1, g_app->cur_row, g_app->cur_col);
		remove_rows(tick);
		new_figure();
		state = st_window;
	}

	//удаление строк(и)
	if(g_app->rem_ok && (tick > g_app->rem_time)){
		g_app->rem_ok = false;
		remove_rows_real();
		state = st_window;
	}
	return state;
}


//вывод процесса игры
static void __fastcall tetris_draw(const canvas_t* can){
	HDC hdc = can->mdc;
	BitBlt(hdc, 0, 0, g_app->iback.width, g_app->iback.height, g_app->iback.mdc, 0, 0, SRCCOPY);

	int x, y, id;
	const figure_t* const fig = &g_app->figure1;
	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j){
			if(fig->mat[i][j] != CELL_NONE){
				x  = (j + g_app->cur_col) * g_app->cell_size;
				y  = (i + g_app->cur_row) * g_app->cell_size;
				id = fig->mat[i][j];
				BitBlt(hdc, x, y, g_app->cell_size, g_app->cell_size, g_app->tiles.mdc, id * g_app->cell_size, 0, SRCCOPY);
			}
		}
	}

	//вывод поля
	const field_t* const pm = &g_app->field;
	for(int i = 0; i < FIELD_SIZE; ++i){
		for(int j = 0; j < FIELD_SIZE; ++j){
			if(pm->field[i][j] != CELL_NONE){
				x  = j * g_app->cell_size;
				y  = i * g_app->cell_size;
				id = pm->field[i][j];
				BitBlt(hdc, x, y, g_app->cell_size, g_app->cell_size, g_app->tiles.mdc, id * g_app->cell_size, g_app->cell_size, SRCCOPY);
			}
		}
	}

	//выделение для удаления строк
	if(g_app->rem_ok){
		int top  = g_app->rem_row1 * g_app->cell_size;
		BitBlt(hdc, 0, top, g_app->iback.width, (g_app->rem_row2 - g_app->rem_row1) * g_app->cell_size, hdc, 0, top, DSTINVERT);

		RECT rc = { 0, top, g_app->iback.width, g_app->rem_row2 * g_app->cell_size };
		FrameRect(hdc, &rc, g_app->colors[WHITE_COLOR]);
	}
}


//рисование статуса игры
static void __fastcall draw_status(const canvas_t* can){
	HDC hdc = can->mdc;
	const int left  = g_app->iback.width;
	const int width = g_width - left;

	//заливаем фон
	RECT rc = { left, 0, g_width, g_height };
	FillRect(hdc, &rc, g_app->bgcolor);

	//вывод заголовка
	HGDIOBJ  fold = SelectObject(hdc, g_app->font);
	int      mode = SetBkMode(hdc, TRANSPARENT);
	COLORREF pcol = SetTextColor(hdc, RGB(0xFF, 0xAA, 0x33));

	const WCHAR fh[] = L"Следующея фигура";
	SIZE fsz = {0};
	GetTextExtentPoint32W(hdc, fh, BUF_LEN(fh), &fsz);
	TextOutW(hdc, left + (width - fsz.cx)/2, 3, fh, BUF_LEN(fh));

	//размер окна-подсказки для следующей фигуры
	const int w = width * 0.8f;
	const int h = (int)((float)g_height * 0.25f);

	int x = left + (width - w)/2;
	int y = fsz.cy + fsz.cy / 2;

	SetRect(&rc, x, y, x + w, y + h);

	if(g_app->bgnext == NULL)
		g_app->bgnext = gradient_brush(w, h, RGB(0, 0, 0xFF), RGB(0, 0xAA, 0x22));

	POINT pt;
	SetBrushOrgEx(hdc, x, y, &pt);
	FillRect(hdc, &rc, g_app->bgnext);
	SetBrushOrgEx(hdc, pt.x, pt.y, NULL);
	FrameRect(hdc, &rc, g_app->colors[GRAY_COLOR]);

	//вывод следущей фигуры
	HBRUSH     hbr = g_app->colors[GRAY_COLOR];
	HBRUSH     pen = g_app->colors[LTGRAY_COLOR];
	const int size = (int)((float)w * 0.18f);

	SetRectEmpty(&rc);
	figure_size(&g_app->figure2, &rc, size);

	const int offx = x + (w - (rc.right  - rc.left)) / 2 - rc.left;
	const int offy = y + (h - (rc.bottom - rc.top))  / 2 - rc.top;

	const figure_t* const fig = &g_app->figure2;
	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j){
			if(fig->mat[i][j] != CELL_NONE){
				int px = offx + j * size;
				int py = offy + i * size;
				SetRect(&rc, px, py, px + size, py + size);
				FillRect(hdc, &rc, hbr);
				FrameRect(hdc, &rc, pen);
			}
		}
	}
	x  = left + (width - w)/2 + 2;
	y += h + fsz.cy;

	const WCHAR sl[] = L"Линий: ";
	str_draw(hdc, sl, BUF_LEN(sl), x, y, g_app->lines, RGB(0xFF, 0xFF, 0xFF));
	y += fsz.cy * 2;

	const WCHAR sb[] = L"Очки: ";
	str_draw(hdc, sb, BUF_LEN(sb), x, y, g_app->bonus, RGB(0xFF, 0xFF, 0x11));
	y += fsz.cy * 2;

	const WCHAR ss[] = L"Уровень: ";
	str_draw(hdc, ss, BUF_LEN(ss), x, y, g_app->level + 1, RGB(0x22, 0xFF, 0x44));

	HGDIOBJ old = SelectObject(hdc, g_app->colors[DKGRAY_COLOR]);
	HGDIOBJ not = SelectObject(hdc, (HBRUSH)GetStockObject(NULL_PEN));
	RoundRect(hdc, x, g_height - h - 8, x + w, g_height - 8, 30, 30);
	SelectObject(hdc, not);
	SelectObject(hdc, old);

	y = g_height - h + (int)((float)g_height * 0.035f);
	SetTextColor(hdc, RGB(0x33, 0xAA, 0xEE));

	const WCHAR* cs[] = { L"Пауза пробел", L"Заново Enter", L"Выход Esc" };
	for(unsigned i = 0; i < 3; ++i, y += (fsz.cy - 2) * 2)
		TextOutW(hdc, x + (int)((float)w * 0.13f), y, cs[i], wcslen(cs[i]));

	SelectObject(hdc, fold);
	SetBkMode(hdc, mode);
	SetTextColor(hdc, pcol);
}


//новая фигура
static void new_figure(void){
	_Bool rnd = ((rand() % 10) < 5);

	if(!g_app->once){
		g_app->figure_cur = build_figure(&g_app->figure1, rnd, g_app->figure_cur);
		g_app->figure_cur = build_figure(&g_app->figure2, rnd, g_app->figure_cur);
		g_app->once = true;
	} else {
		figure_copy(&g_app->figure1, &g_app->figure2);
		g_app->figure_cur = build_figure(&g_app->figure2, rnd, g_app->figure_cur);
	}
	g_app->cur_row = -FIGURE_SIZE;
	g_app->cur_col = 1 + rand() % (FIELD_SIZE - 5);
}


//движение влево или вправо
static void __fastcall tetris_move(UINT key){
	_Bool ret;
	if((key == VK_UP) || (key == (UINT)'W')){//вращение
		figure_rotate();
	} else if((key == VK_LEFT) || (key == (UINT)'A')){ //движение влево

		ret = is_figure_horz(&g_app->field, &g_app->figure1, g_app->cur_row, g_app->cur_col - 1);
		if(ret)
			--g_app->cur_col;

	} else if((key == VK_RIGHT) || (key == (UINT)'D')){ //движение вправо

		ret = is_figure_horz(&g_app->field, &g_app->figure1, g_app->cur_row, g_app->cur_col + 1);
		if(ret)
			++g_app->cur_col;

	} else if((key == VK_DOWN) || (key == (UINT)'S')){ //движение вниз

		ret = is_figure_vert(&g_app->field, &g_app->figure1, g_app->cur_row + 1, g_app->cur_col);
		if(ret)
			++g_app->cur_row;
	}
}


//вращение фигуры
static void figure_rotate(void){
	figure_t old;
	figure_zero(&old);
	figure_transponse(&old, &g_app->figure1);
	figure_reverse_vert(&old);
	int r, c;
	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j){
			if(old.mat[i][j] != CELL_NONE){
				r = i + g_app->cur_row;
				c = j + g_app->cur_col;
				if((r >= FIELD_SIZE) || (c < 0) || (c >= FIELD_SIZE))
					return;
				else if((r >= 0) && (g_app->field.field[r][c] != CELL_NONE))
					return;
			}
		}
	}
	figure_copy(&g_app->figure1, &old);
}


//удаление строк, то есть проверка на удаление строк
static void __fastcall remove_rows(DWORD tick){
	if(g_app->rem_ok)
		return;
	g_app->rem_row1 = g_app->rem_row2 = -1;

	int row1 = -1;
	for(int i = 0; i < FIELD_SIZE; ++i){
		int j = 0;
		while((j < FIELD_SIZE) && (g_app->field.field[i][j] != CELL_NONE))
			++j;

		if(j == FIELD_SIZE){
			row1 = i;
			break;
		}
	}

	if(row1 == -1)
		return;

	int row2 = row1 + 1;
	for(int i = row2; i < FIELD_SIZE; ++i){
		int j = 0;
		while((j < FIELD_SIZE) && (g_app->field.field[i][j] != CELL_NONE))
			++j;

		if(j == FIELD_SIZE)
			row2 = i + 1;
		else
			break;
	}
	g_app->rem_row1 = row1;
	g_app->rem_row2 = row2;
	g_app->rem_ok   = true;
	g_app->rem_time = tick + DELAY_REMOVE;
}


//удаление строк матрицы
static void remove_rows_real(void){
	field_t* pf = &g_app->field;
	const int n = g_app->rem_row2 - g_app->rem_row1;
	for(int i = g_app->rem_row2 - 1; i > n; --i)
		memcpy(pf->field[i], pf->field[i - n], FIELD_SIZE * sizeof(BYTE));

	for(int i = 0; i < n; ++i)
		memset(pf->field[i], 0, FIELD_SIZE * sizeof(BYTE));

	g_app->rem_ok = false;
	g_app->lines += (DWORD)n;
	g_app->bonus += 10 + (DWORD)(n - 1) * 10;
}


//проверка на проигрыш
static void __fastcall test_game_over(DWORD tick){
	int k = 0;
	for(int col = 0; col < FIELD_SIZE; ++col){
		if(g_app->field.field[0][col] == CELL_NONE)
			++k;
	}

	if(k != FIELD_SIZE){//вы проиграли
		g_app->state = GAME_OVER;
		return;
	}

	//проверка на выигрыш
	_Bool res = true;
	for(int i = 0; i < FIELD_SIZE; ++i){
		const BYTE* e = g_app->field.field[i] + FIELD_SIZE;
		for(const BYTE* p = g_app->field.field[i]; p != e; ++p){
			if(*p != CELL_NONE){
				res = false;
				goto jmp_end;
			}
		}
	}
jmp_end:

	if(res || (g_app->lines > FINISH_LINES)){//вы выиграли!
		save_level(g_file_level, ++g_app->level);
		g_app->rem_time = tick + DELAY_LEVEL;
		g_app->pstate   = GAME_NONE;
		g_app->state    = GAME_LEVEL;
	}
}


//вывод меню(меню игры, уровень, пaуза, проигрыш)
static void __fastcall tetris_menu(const canvas_t* can){
	SIZE fsz;
	RECT rc;
	HDC  hdc = can->mdc;

	if(g_app->state == g_app->pstate)
		return;

	HGDIOBJ  hfd  = SelectObject(hdc, g_app->font);
	int      mode = SetBkMode(hdc, TRANSPARENT);
	COLORREF tcol = GetTextColor(hdc);

	switch(g_app->state){
	case GAME_MENU:
		{
			int oy = g_app->cell_size * 7;
			SetRect(&rc, 0, 0, g_width, oy);
			FillRect(hdc, &rc, g_app->colors[DKGRAY_COLOR]);
			StretchBlt(hdc, 0, oy, g_width, g_height - oy, g_app->iback.mdc, 0, 0, g_app->iback.width, g_app->iback.height, SRCCOPY);

			//вывести надпись тетрис
			const char tetris[] = {
				"### ### ### ### #   # ###\n"\
				" #  #    #  # # #  ## #  \n"\
				" #  ###  #  ### # # # #  \n"\
				" #  #    #  #   ##  # #  \n"\
				" #  ###  #  #   #   # ###"
			};

			int ox   = (g_width - count_len(tetris, '\n') * g_app->cell_size) / 2;
			int left = ox, top = g_app->cell_size;
			for(const char* p = tetris; *p; ++p){
				if(*p == '#'){
					BitBlt(hdc, left, top, g_app->cell_size, g_app->cell_size, g_app->tiles.mdc, g_app->cell_size, 0, SRCCOPY);
					left += g_app->cell_size;
				} else if(*p == ' ')
					left += g_app->cell_size;
				else if(*p == '\n'){
					top += g_app->cell_size;
					left = ox;
				}
			}
/*
			const WCHAR author[] = L"автор игры (С) Кудуштеев Алексей";
			GetTextExtentPoint32W(hdc, author, BUF_LEN(author), &fsz);
			SetTextColor(hdc, RGB(0xFF, 0, 0xAA));
			TextOutW(hdc, (g_width - fsz.cx)/2, oy + fsz.cy, author, BUF_LEN(author));
*/
			HGDIOBJ old = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN));
			HGDIOBJ hbr = SelectObject(hdc, g_app->bgcolor);
			SetTextColor(hdc, RGB(0xFF, 0xFF, 0));

			MoveToEx(hdc, 0, oy, NULL);
			LineTo(hdc, g_width, oy);

			const WCHAR  cmd1[] = L"НАЧАТЬ ИГРАТЬ - клавиша Enter";
			const WCHAR  cmd2[] = L"ВЫХОД ИЗ ИГРЫ - клавиша Esc";
			const WCHAR* caps[] = { cmd1, cmd2 };
			const UINT   lens[] = { BUF_LEN(cmd1), BUF_LEN(cmd2) };

			int k = g_app->cell_size << 1;
			int m = k << 1;
			ox    = g_width / 3 * 2;
			left  = (g_width - ox)/2;
			top   = oy + k + g_app->cell_size - (g_app->cell_size >> 1);

			for(int i = 0; i < 2; ++i, top += m){
				Rectangle(hdc, left, top, g_width - left, top + k);
				GetTextExtentPoint32W(hdc, caps[i], lens[i], &fsz);
				TextOutW(hdc, (g_width - fsz.cx)/2, top + (k - fsz.cy)/2, caps[i], lens[i]);
			}

			SetTextColor(hdc, RGB(0x11, 0xBB, 0xFF));
			const WCHAR ctrl[] = L"Управление игрой\nдвижение влево: клавиша влево или A\nдвижение вправо: клавиша вправо или D\nдвижение вниз: клавиша вниз или S\nвращение: клавиша вверх или W";

			SetRect(&rc, 5, g_height - g_app->font_height*5, g_width - 5, g_height);
			oy = DrawTextW(hdc, ctrl, BUF_LEN(ctrl), &rc, DT_WORDBREAK | DT_CALCRECT);

			rc.top    = g_height - oy - 3;
			rc.bottom = rc.top   + oy;
			DrawTextW(hdc, ctrl, BUF_LEN(ctrl), &rc, DT_WORDBREAK | DT_LEFT);

			SelectObject(hdc, hbr);
			SelectObject(hdc, old);
		}
		break;
	case GAME_LEVEL: //показ номер уровня
		{
			PatBlt(hdc, 0, 0, g_width, g_height, BLACKNESS);
			SetRect(&rc, g_app->cell_size, g_app->cell_size, g_width - g_app->cell_size, g_height - g_app->cell_size);
			FrameRect(hdc, &rc, g_app->colors[WHITE_COLOR]);
			SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));

			const WCHAR lvl[] = L"УРОВЕНЬ - ";
			GetTextExtentPoint32W(hdc, lvl, BUF_LEN(lvl), &fsz);

			int left = (g_width  - fsz.cx) / 2;
			int top  = (g_height - fsz.cy) / 2;
			TextOutW(hdc, left, top, lvl, BUF_LEN(lvl));
			draw_dword(hdc, left + fsz.cx, top, g_app->level + 1);
		}
		break;
	case GAME_OVER:
	case GAME_PAUSE:
		{
			HGDIOBJ old = SelectObject(hdc, (HPEN)GetStockObject(WHITE_PEN));
			HBRUSH  hbr = SelectObject(hdc, g_app->colors[DKGRAY_COLOR]);
			int w = (int)((float)g_width  * 0.6f);
			int h = (int)((float)g_height * 0.4f);
			int x = (g_width  - w) / 2;
			int y = (g_height - h) / 2;

			if(g_app->state == GAME_PAUSE){
				SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
				Ellipse(hdc, x, y, x + w, y + h);

				const WCHAR pause[] = L"* * * ПАУЗА * * *\nПРОДОЛЖИТЬ КЛАВИША Enter";
				SetRect(&rc, x, y + g_app->cell_size * 3, x + w, y + h);
				DrawTextW(hdc, pause, BUF_LEN(pause), &rc, DT_WORDBREAK | DT_VCENTER | DT_CENTER);
			} else {
				SetTextColor(hdc, RGB(0xFF, 0, 0));
				Rectangle(hdc, x, y, x + w, y + h);

				const WCHAR over[] = L"ВЫ ПРОИГРАЛИ ИГРУ!!!\nНАЧАТЬ ЗАНОВО КЛАВИША Enter";
				SetRect(&rc, x, y + g_app->cell_size * 3, x + w, y + h);
				DrawTextW(hdc, over, BUF_LEN(over), &rc, DT_WORDBREAK | DT_VCENTER | DT_CENTER);
			}
			SelectObject(hdc, hbr);
			SelectObject(hdc, old);
		}
		break;
	default:
		break;
	}

	SetTextColor(hdc, tcol);
	SetBkMode(hdc, mode);
	SelectObject(hdc, hfd);

	g_app->pstate = g_app->state;
}

//-----------------------------------------------------------------------------------------------

//создание квадратов для тетриса
static void create_squares(void){
	const COLORREF colorsA[MAX_COLORS] = {
		RGB(0xDD, 0x55, 0x55), RGB(0x55, 0xDD, 0x55), RGB(0x55, 0x55, 0xDD), RGB(0xDD, 0x55, 0xDD),
		RGB(0xCC, 0xCC, 0x55), RGB(0x55, 0xDD, 0xDD), RGB(0xFF, 0x88, 0),    RGB(0xFF, 0, 0x88), RGB(0xAA, 0xAA, 0xAA)
	};

	const COLORREF colorsB[MAX_COLORS] = {
		RGB(0xFF, 0x88, 0x88), RGB(0x99, 0xFF, 0x99), RGB(0x88, 0x88, 0xFF), RGB(0xFF, 0x88, 0xFF),
		RGB(0xFF, 0xFF, 0x88), RGB(0xAA, 0xFF, 0xFF), RGB(0xFF, 0xBB, 0x44), RGB(0xFF, 0x55, 0xAA), RGB(0xCC, 0xCC, 0xCC)
	};

	const COLORREF colorsC[MAX_COLORS] = {
		RGB(0xAA, 0x55, 0x55), RGB(0x55, 0xAA, 0x55), RGB(0x55, 0x55, 0xAA), RGB(0xAA, 0x55, 0xAA),
		RGB(0xAA, 0xAA, 0x55), RGB(0x55, 0xAA, 0xAA), RGB(0xCC, 0x44, 0),    RGB(0xCC, 0, 0x55), RGB(0x88, 0x88, 0x88)
	};

	for(int i = 0; i < MAX_COLORS; ++i){
		square_t sq = {
			.x      = i * g_app->cell_size,
			.y      = 0,
			.size   = g_app->cell_size,
			.colorA = colorsA[i],
			.colorB = colorsB[i],
			.colorC = colorsC[i]
		};
		put_square(&g_app->tiles, &sq);
	}
}


static void str_draw(HDC hdc, LPCWSTR str, int len, int x, int y, DWORD num, COLORREF color){
	SIZE fsz = {0};
	GetTextExtentPoint32W(hdc, str, len, &fsz);
	TextOutW(hdc, x, y, str, len);

	COLORREF col = SetTextColor(hdc, color);
	draw_dword(hdc, x + fsz.cx + 3, y, num);
	SetTextColor(hdc, col);
}
