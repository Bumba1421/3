/*
	Автор(с): Кудуштеев Алексей
	Версия языка программирования Си: C99/C11
*/
#include "headers.h"
#define RUS_LOCALE_1049  (LCID)1049


//точка входа
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){
	srand((unsigned int)time(NULL));

	if(GetThreadLocale() != RUS_LOCALE_1049)
		SetThreadLocale(RUS_LOCALE_1049);

	_Bool ret = game_run(hInstance, L"Тетрис");
	return (ret) ? EXIT_SUCCESS : EXIT_FAILURE;
}
