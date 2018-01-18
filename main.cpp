#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <psapi.h>
#include <string.h>
#include <conio.h>

// オーナー・ウインドウの判別
#define IsWindowOwner(h) (GetWindow(h,GW_OWNER) == NULL)

struct {
	TCHAR title[1024];
	DWORD pid;
}windows[100];

int structCounter = 0;

BOOL transparentWindow(int id, int alpha);
BOOL IsEnumCheck(HWND hWnd, LPCTSTR lpTitle, LPCTSTR lpClass);
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
HWND gethWndfromWindows(int TargetID);

BOOL transparentWindow(int id, int alpha)
{
	COLORREF r;

	HWND hWnd = gethWndfromWindows(id);

	if (hWnd == NULL) {
		for (int i = 0; i < 5; i++) puts("FAILED!!!");
		Sleep(3000);
		return false;
	}

	printf("Window Handler: %p\n", &hWnd);

	//HWND hWnd = (struct HWND__*)0x000801BE;
	long i = GetWindowLong(hWnd, GWL_EXSTYLE);

	SetWindowLong(hWnd, GWL_EXSTYLE, i | WS_EX_LAYERED);
	r = RGB(255, 255, 255);

	SetLayeredWindowAttributes(hWnd, r, alpha, LWA_ALPHA);

	printf("Set window alpha to %d\n", alpha);
	Sleep(2000);

	return true;
}

BOOL IsEnumCheck(HWND hWnd, LPCTSTR lpTitle, LPCTSTR lpClass)
{
	if (IsWindowVisible(hWnd)) {
		if (IsWindowOwner(hWnd)) {
			if (lpTitle[0] != TEXT('\0')) {
				if (lstrcmp(lpClass, TEXT("Progman")) != 0) {
					_tprintf(TEXT("No.%02d\n"), structCounter+1);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	TCHAR   szTitle[1024];
	TCHAR   szClass[1024];

	//INT*    lpCount = (INT *)lParam;
	//*lpCount += 1;                                      // カウントの加算

	GetWindowText(hWnd, szTitle, sizeof(szTitle));	// キャプションの取得
	GetClassName(hWnd, szClass, sizeof(szClass));	// クラス文字列の取得

	if (IsEnumCheck(hWnd, szTitle, szClass)) {
		DWORD ProcessID;
		GetWindowThreadProcessId(hWnd, &ProcessID);
		/*
		_tprintf(TEXT("%4d: %c %c %c %c %c %c %c [%-50s] [%s]\n"),
			*lpCount,								// 列挙番号
			IsWindowOwner(hWnd) ? 'O' : '_',		// Ownerウインドウ
			IsWindowUnicode(hWnd) ? 'U' : '_',		// Unicodeタイプ
			IsWindowVisible(hWnd) ? 'V' : '_',		// 可視状態
			IsWindowEnabled(hWnd) ? 'E' : '_',		// 有効状態
			IsIconic(hWnd) ? 'I' : '_',				// 最小化状態
			IsZoomed(hWnd) ? 'Z' : '_',				// 最大化状態
			IsWindow(hWnd) ? 'W' : '_',				// ウインドウ有無
			szClass,								// クラス文字列
			szTitle);								// キャプション
		*/

		windows[structCounter].pid = ProcessID;
		strcpy_s(windows[structCounter].title, szTitle);
		structCounter++;

		puts("------------------------------------------------");
		_tprintf(TEXT("TASK:\t%s\nPID:\t%d\n"), szTitle, ProcessID);
		puts("------------------------------------------------");
	}
	return TRUE;
}

HWND gethWndfromWindows(int id)
{
	HWND hWnd = GetTopWindow(NULL);

	do {
		if (GetWindowLong(hWnd, GWL_HWNDPARENT) != 0 || !IsWindowVisible(hWnd)) continue;

		DWORD ProcessID;
		TCHAR szTitle[1024];

		GetWindowThreadProcessId(hWnd, &ProcessID);
		GetWindowText(hWnd, szTitle, sizeof(szTitle));

		if (windows[id].pid == ProcessID && !lstrcmp(windows[id].title, szTitle)) {
			printf("Window handler found! PID: %d\n", ProcessID);
			return hWnd;
		}
	} while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) != NULL);

	return NULL;
}

int main()
{
	int id;
	int alpha;
	int nCount = 0;

	char ch;

	while (1)
	{
		do {
			structCounter = 0;
			for (int i = 0; i < 100; i++) {
				windows[i].pid = NULL;
				strcpy_s(windows[i].title, TEXT(""));
			}

			system("cls");
			puts("Window Transparent Tool by almajiro");
			puts("----------------------------------------------");

			EnumWindows(EnumWindowsProc, (LPARAM)&nCount);
			printf("r=Refresh, s=Set, other=Exit: ");
		} while ((ch = _getche()) == 'r');

		if (ch == 'x' || ch != 's') break;

		puts("");
		printf("Enter the application id: ");
		scanf_s("%d", &id);

		if (structCounter < id || id <= 0) {
			do {
				puts("Please enter the correct application id.");
				Sleep(1000);
				printf("Enter the application id: ");
				scanf_s("%d", &id);
			} while (structCounter < id || id <= 0);
		}

		printf("Alpha (MAX: 255): ");
		scanf_s("%d", &alpha);

		if (255 < id || id <= 0) {
			do {
				puts("It exceeds the range. Or the value is too small.");
				Sleep(1000);
				printf("Alpha (MAX: 255): ");
				scanf_s("%d", &alpha);
			} while (structCounter < id || id <= 0);
		}

		transparentWindow(id-1, alpha);
		system("cls");
	}

    return 0;
}
