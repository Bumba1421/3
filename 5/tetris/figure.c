//Автор(с): Кудуштеев Алексей
#include "headers.h"


//проверка на матрицу 3x3
_Bool is_matrix_3x3(const figure_t* fig){
	int a, b, c, d, square = 0, m3x3 = 0;
	for(int i = 0; i < FIGURE_SIZE; ++i){
		a = fig->mat[i][0];//левый-катет
		b = fig->mat[0][i];//верхний-катет
		c = fig->mat[i][FIGURE_SIZE - 1];//правый-катет
		d = fig->mat[FIGURE_SIZE - 1][i];//нижний-катет

		if((a | b | c | d) == CELL_NONE)
			++square;

		if((c | d) == CELL_NONE)
			++m3x3;
	}

	if(square == FIGURE_SIZE) //проверку на квадрат
		return false;
	return (m3x3 == FIGURE_SIZE);
}


//размер фигуры без пустых клеток
void figure_size(const figure_t* fig, LPRECT prc, long size){
	const long len = is_matrix_3x3(fig) ? (FIGURE_SIZE - 1) : FIGURE_SIZE;

	prc->left = prc->top = prc->right = prc->bottom = -1;
	for(long i = 0; i < len; ++i){
		for(long j = 0; j < len; ++j){
			if(fig->mat[i][j] == CELL_NONE)
				continue;

			if((prc->left == -1) || (prc->left > j))
				prc->left = j;
			if((prc->right == -1) || (prc->right < j))
				prc->right = j;

			if((prc->top == -1) || (prc->top > i))
				prc->top = i;
			if((prc->bottom == -1) || (prc->bottom < i))
				prc->bottom = i;
		}
	}
	prc->left  *= size;
	prc->top   *= size;
	prc->right  = (prc->right  + 1) * size;
	prc->bottom = (prc->bottom + 1) * size;
}


//проверка на движение вниз
_Bool is_figure_vert(const field_t* fd, const figure_t* fig, int row, int col){
	int r, c;
	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j){
			if(fig->mat[i][j] == CELL_NONE)
				continue;

			r = i + row;
			c = j + col;
			if((r < 0) || (c < 0) || (c >= FIELD_SIZE))
				continue;
			else if((r >= FIELD_SIZE) || (fd->field[r][c] != CELL_NONE))
				return false;
		}
	}
	return true;
}


//проверка на движение по горизонтали
_Bool is_figure_horz(const field_t* fd, const figure_t* fig, int row, int col){
	int r, c;
	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j){
			if(fig->mat[i][j] == CELL_NONE)
				continue;

			r = i + row;
			c = j + col;
			if((c < 0) || (c >= FIELD_SIZE))
				return false;
			else if((r < 0) || (r >= FIELD_SIZE))
				continue;
			else if(fd->field[r][c] != CELL_NONE)
				return false;
		}
	}
	return true;
}


//вставка фигуры в поле
void field_put_figure(field_t* fd, const figure_t* fig, int row, int col){
	int r, c;
	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j){
			if(fig->mat[i][j] != CELL_NONE){
				r = i + row;
				c = j + col;
				if((r >= 0) && (c >= 0) && (r < FIELD_SIZE) && (c < FIELD_SIZE))
					fd->field[r][c] = fig->mat[i][j];
			}
		}
	}
}


//реверсирование строк
void figure_reverse_horz(figure_t* src){
	const int len = is_matrix_3x3(src) ? (FIGURE_SIZE - 1) : FIGURE_SIZE;

	int i = 0, j = len - 1;
	for(; i < j; ++i, --j){
		for(int r = 0; r < len; ++r){
			BYTE v = src->mat[r][i];
			src->mat[r][i] = src->mat[r][j];
			src->mat[r][j] = v;
		}
	}
}


//реверсирование столбцов
void figure_reverse_vert(figure_t* src){
	const int len = is_matrix_3x3(src) ? (FIGURE_SIZE - 1) : FIGURE_SIZE;

	int i = 0, j = len - 1;
	for(; i < j; ++i, --j){
		for(int c = 0; c < len; ++c){
			BYTE v = src->mat[i][c];
			src->mat[i][c] = src->mat[j][c];
			src->mat[j][c] = v;
		}
	}
}


//транспонирование
void figure_transponse(figure_t* dst, const figure_t* src){
	const int len = is_matrix_3x3(src) ? (FIGURE_SIZE - 1) : FIGURE_SIZE;

	for(int i = 0; i < len; ++i){
		for(int j = 0; j < len; ++j)
			dst->mat[j][i] = src->mat[i][j];
	}
}


//транспонирование
void figure_transponse_old(figure_t* src){
	const int len = is_matrix_3x3(src) ? (FIGURE_SIZE - 1) : FIGURE_SIZE;

	for(int i = 0; i < len; ++i){
		for(int j = 0; j < len; ++j){
			BYTE v = src->mat[j][i];
			src->mat[j][i] = src->mat[i][j];
			src->mat[i][j] = v;
		}
	}
}


//обнуление
void figure_zero(figure_t* src){
	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j)
			src->mat[i][j] = CELL_NONE;
	}
}


//копирование
void figure_copy(figure_t* dst, const figure_t* src){
	for(int i = 0; i < FIGURE_SIZE; ++i)
		memcpy(dst->mat[i], src->mat[i], FIGURE_SIZE * sizeof(BYTE));
}


//генерация фигуры
int build_figure(figure_t* fig, _Bool no_repeat, int current){
	//фигуры
	const BYTE figures[FIGURE_SIZE * NUM_FIGURE][FIGURE_SIZE] = {
		{0, 1, 0, 0}, //0
		{0, 1, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 0, 0},

		{1, 0, 0, 0}, //1
		{1, 0, 0, 0},
		{1, 1, 0, 0},
		{0, 0, 0, 0},

		{1, 1, 0, 0}, //2
		{0, 1, 1, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},

		{0, 1, 1, 0}, //3
		{1, 1, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},

		{0, 0, 1, 0}, //4
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0},

		{0, 0, 0, 0}, //5
		{1, 1, 1, 0},
		{0, 1, 0, 0},
		{0, 0, 0, 0},

		{1, 0, 0, 0}, //6
		{1, 1, 0, 0},
		{1, 1, 0, 0},
		{0, 0, 0, 0},

		{0, 0, 1, 0}, //7
		{0, 1, 1, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0},

		{0, 0, 0, 0}, //8
		{0, 1, 1, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0},

		{0, 0, 0, 0}, //9
		{0, 1, 0, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0},

		{0, 0, 0, 0}, //10
		{0, 0, 1, 0},
		{0, 1, 1, 0},
		{0, 0, 0, 0},

		{0, 0, 0, 0}, //11
		{1, 1, 1, 0},
		{1, 0, 1, 0},
		{0, 0, 0, 0},

		{0, 0, 1, 0}, //12
		{0, 0, 1, 0},
		{1, 1, 1, 0},
		{0, 0, 0, 0},

		{1, 0, 0, 0}, //13
		{1, 0, 0, 0},
		{1, 1, 1, 0},
		{0, 0, 0, 0}
	};

	int index = 0;
	int type  = rand() % 7 + 1;

	if(no_repeat){
		do {
			index = rand() % 12 * 4;
		} while(index == current);
	} else
		index = rand() % 12 * 4;

	for(int i = 0; i < FIGURE_SIZE; ++i){
		for(int j = 0; j < FIGURE_SIZE; ++j){
			if(figures[index + i][j] == 1)
				fig->mat[i][j] = type;
			else
				fig->mat[i][j] = CELL_NONE;
		}
	}

	type = rand() % 10;
	if(type <= 3)
		figure_transponse_old(fig);
	else if(type <= 5)
		figure_reverse_vert(fig);
	else if(type <= 7)
		figure_reverse_horz(fig);
	return index;
}


//генерация уровней
void build_level(field_t* field, DWORD level){
	for(int i = 0; i < FIELD_SIZE; ++i)
		memset(field->field[i], CELL_NONE, FIELD_SIZE * sizeof(BYTE));

	int num = 2;
	if((level >= 0) && (level <= 2))
		num = 3;
	else if((level >= 3) && (level <= 5))
		num = 4;
	else if((level >= 6) && (level <= 8))
		num = 5;
	else if((level >= 9) && (level <= 11))
		num = 6;
	else if((level >= 12) && (level <= 14))
		num = 7;
	else
		num = 3 + rand() % 5;

	int n = FIELD_SIZE - num; //миниум-3  максимум  ROWS - 8
	int t = 1 + rand() % 7;
	for(int i = FIELD_SIZE - 1; i > n; --i){
		for(int j = 0; j < FIELD_SIZE; ++j){
			if(rand() & 1)
				field->field[i][j] = t;
		}
	}
}
