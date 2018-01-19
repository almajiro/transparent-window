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
	COLORREF r = RGB(255, 255, 255);

	HWND hWnd = gethWndfromWindows(id);

	if (hWnd == NULL) {
		puts("");
		for (int i = 0; i < 5; i++) puts("FAILED!!!");
		Sleep(3000);
		return false;
	}

	long i = GetWindowLong(hWnd, GWL_EXSTYLE);

	SetWindowLong(hWnd, GWL_EXSTYLE, i | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, r, alpha, LWA_ALPHA);
	return true;
}

BOOL IsEnumCheck(HWND hWnd, LPCTSTR lpTitle, LPCTSTR lpClass)
{
	if (IsWindowVisible(hWnd)) {
		if (IsWindowOwner(hWnd)) {
			if (lpTitle[0] != TEXT('\0')) {
				if (lstrcmp(lpClass, TEXT("Progman")) != 0) {
					_tprintf(TEXT("| NO.%02d | "), structCounter+1);
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

	GetWindowText(hWnd, szTitle, sizeof(szTitle));
	GetClassName(hWnd, szClass, sizeof(szClass));

	if (IsEnumCheck(hWnd, szTitle, szClass)) {
		DWORD ProcessID;
		GetWindowThreadProcessId(hWnd, &ProcessID);

		windows[structCounter].pid = ProcessID;
		strcpy_s(windows[structCounter].title, szTitle);
		structCounter++;

		TCHAR *pt = szTitle;
		int i = 0;

		while (*pt != 0) {
			if (_mbclen((BYTE*)pt) == 1) {
				printf("%c", *pt);
			}
			else { // which mean is 2 bytes character
				putchar(*pt);
			}

			if (i == 66) {
				printf(" |\n");
				printf("|       | ");
				i = 0;
			}
			else {
				i++;
			}

			pt++;
		}

		if (i <= 66) {
			for (int j = 0; j < 68 - i; j++) {
				printf(" ");
			}
			puts("|");
		}
		puts("+-------+---------------------------------------------------------------------+");
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
			return hWnd;
		}
	} while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) != NULL);

	return NULL;
}

int main()
{
	int id, alpha, input;
	int nCount = 0;
	int i;

	char ch;

	while (1)
	{
		// List applications
		do {
			structCounter = 0;
			for (int i = 0; i < 100; i++) {
				windows[i].pid = NULL;
				strcpy_s(windows[i].title, TEXT(""));
			}

			system("cls");
			puts("+------------------+");
			puts("| APPLICATION LIST |");
			puts("+-------+----------+----------------------------------------------------------+");

			EnumWindows(EnumWindowsProc, (LPARAM)&nCount);
			printf("r:REFRESH, s:SET, other:EXIT: ");
		} while ((ch = _getche()) == 'r');

		if (ch == 'x' || ch != 's') break;

		puts("");
		printf("ENTER THE APPLICATION ID: ");
		scanf_s("%d", &id);

		if (structCounter < id || id <= 0) {
			do {
				puts("PLEASE ENTER THE CORRECT APPLICATION ID.");
				Sleep(1000);
				printf("ENTER THE APPLICATION ID: ");
				scanf_s("%d", &id);
			} while (structCounter < id || id <= 0);
		}

		id--;
		system("cls");

		puts("SELECTED APPLICATION:");
		puts("-------------------------------------------------------------------------------");
		_tprintf("PROCESS NAME: %s\n", windows[id].title);
		_tprintf("\t PID: %d\n", windows[id].pid);
		puts("-------------------------------------------------------------------------------");

		puts("PRESS t TO CHANGE TRANSPARENCY USING ARROW KEY");
		puts("PRESS c TO ENTER THE VALUE");
		puts("PRESS x TO BACK APPLICATION LIST");
		puts("-------------------------------------------------------------------------------");
		printf("YOUR CHOICE: ");
		ch = _getche();

		if(ch == 'c') {
			printf("\rTRANSPARENCY[%%]: ");
			scanf_s("%d", &input);

			if (100 < input || input <= 0) {
				do {
					puts("IT EXCEEDS THE RANGE. OR THE VALUE IS TOO SMALL.");
					Sleep(1000);
					printf("TRANSPARENCY[%%]: ");
					scanf_s("%d", &input);
				} while (100 < input || input <= 0);
			}

			alpha = ((float)input / 100) * (float)255;

			if (transparentWindow(id, alpha)){
				printf("WINDOW TRANSPARENCY SET TO %d%%\n", input);
				Sleep(3000);
			}
			else {
				puts("FAIL TO SET TRANSPARENCY");
			}
		}
		else if (ch == 't') {
			alpha = 255;
			transparentWindow(id, alpha);
			while (1) {
				ch = 0;

				printf("\rTRANSPARENCY:\t");
				printf("|");
				for (i = 0; i < alpha / 5; i++) {
					if ((alpha / 5 - 1) == i) {
						printf(">");
					}
					else {
						printf("-");
					}
				}
				for (; i < 51; i++) {
					printf(" ");
				}

				printf("|");

				printf("  [");
				printf("%003.f%%", ((float)alpha / (float)255 * 100) );
				printf("] ");

				ch = _getch();
				if (ch == 0x48 || ch == 0x4d) {
					if (alpha < 255) alpha += 5;
					if (!transparentWindow(id, alpha)) break;
				}
				if (ch == 0x50 || ch == 0x4b) {
					if (alpha > 5) alpha -= 5;
					if(!transparentWindow(id, alpha)) break;
				}

				if (ch == 'x') break;
			}
		}
	}

    return 0;
}
