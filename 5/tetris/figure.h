//Автор(с): Кудуштеев Алексей
#if !defined(_FIGURE_KUDUSHTEEV_H_)
#define _FIGURE_KUDUSHTEEV_H_
#ifdef _MSC_VER
#pragma once
#endif
#define FIELD_SIZE   20
#define FIGURE_SIZE  4
#define CELL_NONE    0
#define NUM_FIGURE   14


typedef struct {
	BYTE field[FIELD_SIZE][FIELD_SIZE];
} field_t;


typedef struct {
	BYTE mat[FIGURE_SIZE][FIGURE_SIZE];
} figure_t;


extern _Bool is_matrix_3x3(const figure_t* fig);
extern void figure_size(const figure_t* fig, LPRECT prc, long size);
extern _Bool is_figure_vert(const field_t* fd, const figure_t* fig, int row, int col);
extern _Bool is_figure_horz(const field_t* fd, const figure_t* fig, int row, int col);
extern void field_put_figure(field_t* fd, const figure_t* fig, int row, int col);
extern void figure_reverse_horz(figure_t* src);
extern void figure_reverse_vert(figure_t* src);
extern void figure_transponse(figure_t* dst, const figure_t* src);
extern void figure_transponse_old(figure_t* src);
extern void figure_zero(figure_t* src);
extern void figure_copy(figure_t* dst, const figure_t* src);
extern void build_level(field_t* field, DWORD level);

#endif
