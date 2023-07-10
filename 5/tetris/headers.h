//Автор(с): Кудуштеев Алексей
#if !defined(_HEADERS_KUDUSHTEEV_H_)
#define _HEADERS_KUDUSHTEEV_H_
#ifdef _MSC_VER
#pragma once
#endif

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS  1
#endif

#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <string.h>
#include <malloc.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#if defined(__GNUC__) && !defined(__fastcall)
#define __fastcall __attribute__((fastcall))
#endif

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#include "util.h"
#include "figure.h"
#include "game.h"
#endif
