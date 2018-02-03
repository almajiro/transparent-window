/**
 * @file transparent_testing.cpp
 * @brief application window transparent tool.
 * @author almajiro
 * @date 2018/01/18
 */

/**
 * @note
 * TCHAR = char
 * LPCTSTR = const char*
 * LPTSTR = char*
 * when UNICODE defined
 * TCHAR = WCHAR
 * LPCTSTR = const WCHAR*
 * LPTSTR = WCHAR*
 */

// include headers
#include "stdafx.h"
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winuser.h>
#include <psapi.h>
#include <string.h>
#include <conio.h>

// is window owner?
#define IsWindowOwner(h) (GetWindow(h,GW_OWNER) == NULL)

//! running applications data
struct {
	TCHAR title[1024];
	DWORD pid;
}windows[100];

//! running applications counter
int windowCounter = 0;

// functions
BOOL transparentWindow(int id, int alpha);
BOOL IsEnumCheck(HWND hWnd, LPCTSTR lpTitle, LPCTSTR lpClass);
BOOL changeTransparentUsingArrowKey(int id);
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
HWND gethWndfromWindows(int TargetID);
void setDefault();
void selected(int id);
void dispSelectedHeader(int id);
char getChoice();
char listApplications();
int inputNumber();

/**
 * @brief transparent the application window.
 * @param id integer application id.
 * @param alpha integer window transparency
 * @return window handler HWND
 */
BOOL transparentWindow(int id, int alpha)
{
	//! rgb color
	COLORREF r = RGB(255, 255, 255);

	//! window handler
	HWND hWnd = gethWndfromWindows(id);

	// if failed to get window handler.
	if (hWnd == NULL) return false;

	//! get window data with extended window style mode.
	long i = GetWindowLong(hWnd, GWL_EXSTYLE);

	// changes an attribute of the specified window.
	SetWindowLong(hWnd, GWL_EXSTYLE, i | WS_EX_LAYERED);

	// sets the opacity and transparency color key of a layered window.
	SetLayeredWindowAttributes(hWnd, r, alpha, LWA_ALPHA);

	return true;
}

/**
 * @brief enum check
 * @param hWnd HWND window handler
 * @param LPCTSTR lpTitle title
 * @param LPCTSTR lpClass class
 * @return boolean window status
 */
BOOL IsEnumCheck(HWND hWnd, LPCTSTR lpTitle, LPCTSTR lpClass)
{
	if (IsWindowVisible(hWnd)) {
		if (IsWindowOwner(hWnd)) {
			if (lpTitle[0] != TEXT('\0')) {
				if (lstrcmp(lpClass, TEXT("Progman")) != 0) {
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

/**
 * @brief callback function for EnumWindows
 * @param hWnd HWND window handler
 * @param lParal LPARAM application defined value
 * @return boolean true
 */
BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	//! title buffer
	TCHAR szTitle[1024];
	TCHAR szClass[1024];

	GetWindowText(hWnd, szTitle, sizeof(szTitle));
	GetClassName(hWnd, szClass, sizeof(szClass));

	if (IsEnumCheck(hWnd, szTitle, szClass)) {
		DWORD ProcessID;
		GetWindowThreadProcessId(hWnd, &ProcessID);

		windows[windowCounter].pid = ProcessID;
		strcpy_s(windows[windowCounter].title, szTitle);
		windowCounter++;

		TCHAR *pt = szTitle;
		int i = 0;

		_tprintf(TEXT("| No.%03d | "), windowCounter);

		while (*pt != 0) {
			if (_mbclen((BYTE*)pt) == 1) {
				printf("%c", *pt);
			}
			else {
				putchar(*pt);
			}

			if (i == 65) {
				printf(" |\n");
				printf("|        | ");
				i = 0;
			}
			else {
				i++;
			}

			pt++;
		}

		if (i <= 65) {
			for (int j = 0; j < 67 - i; j++) {
				printf(" ");
			}
			puts("|");
		}
		puts("+--------+--------------------------------------------------------------------+");
	}
	return TRUE;
}

/**
 * @brief get window handler
 * @param id integer application id.
 * @return window handler HWND
 */
HWND gethWndfromWindows(int id)
{
	//! get top window handler
	HWND hWnd = GetTopWindow(NULL);

	do {
		if (GetWindowLong(hWnd, GWL_HWNDPARENT) != 0 || !IsWindowVisible(hWnd)) continue;

		DWORD ProcessID;

		//! title buffer
		TCHAR szTitle[1024];

		GetWindowThreadProcessId(hWnd, &ProcessID);
		GetWindowText(hWnd, szTitle, sizeof(szTitle));

		if (windows[id].pid == ProcessID && !lstrcmp(windows[id].title, szTitle)) {
			return hWnd;
		}
	} while ((hWnd = GetNextWindow(hWnd, GW_HWNDNEXT)) != NULL);

	return NULL;
}

/**
 * @brief set all windows to 100%.
 */
void setDefault()
{
	for (int i = 0; i<windowCounter; i++) {
		transparentWindow(i, 255);
	}
	puts("\n >>>すべてのアプリケーションの透明度を元に戻しました。");

	// wait for 2 seconds.
	Sleep(2000);
}

/**
 * @brief display the properties page header
 * @param id integer application id.
 */
void dispSelectedHeader(int id)
{
	system("cls");

	puts("+----------------------------+                       +------------------------+");
	puts("| 選択されたアプリケーション |                       | ウィンドウ透明化ツール |");
	puts("+----------+-----------------+-----------------------+------------------------+");
	_tprintf("| タスク名 | ");

	TCHAR *pt = windows[id].title;
	int i = 0;

	while (*pt != 0) {
		if (_mbclen((BYTE*)pt) == 1) {
			printf("%c", *pt);
		}
		else { // which mean is 2 bytes character
			putchar(*pt);
		}

		if (i == 63) {
			printf(" |\n");
			printf("|          | ");
			i = 0;
		}
		else {
			i++;
		}

		pt++;
	}

	if (i <= 63) {
		for (int j = 0; j < 65 - i; j++) {
			printf(" ");
		}
		puts("|");
	}

	_tprintf("|   PID    | %d                                                             |\n", windows[id].pid);

	puts("+----------+------------------------------------------------------------------+");
	puts("   T > 透明度を矢印キーで設定 (←↑↓→)");
	puts("   C > 透明度を数値で入力 (0-100)");
	puts(" ESC > 戻る");
	puts("-------------------------------------------------------------------------------");
}

/**
 * @brief get one character.
 * @return character code.
 */
char getChoice()
{
	char ch;
	ch = toupper(_getche());
	_putch('\n');
	return ch;
}

/**
 * @brief change transparent using arrow key.
 * @param id integer application id.
 * @return hwnd error status in boolean.
 */
BOOL changeTransparentUsingArrowKey(int id)
{
	//! character buffer
	char ch;

	//! counter
	int i;
	
	//! window alpha
	int alpha = 255;

	transparentWindow(id, alpha);
	_putch('\n');
	puts(" >>> ESCで戻る");

	while (1) {
		ch = 0;

		printf("\r\t透明度:\t");
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
		printf("%003.f%%", ((float)alpha / (float)255 * 100));
		printf("] ");

		ch = _getch();
		if (ch == 0x48 || ch == 0x4d) {
			if (alpha < 255) {
				alpha += 5;
				if (!transparentWindow(id, alpha)) return false;
			}
		}
		if (ch == 0x50 || ch == 0x4b) {
			if (alpha > 5) {
				alpha -= 5;
				if (!transparentWindow(id, alpha)) return false;
			}
		}

		if (ch == 0x1B) break;
		Sleep(1);
	}

	// successful to exit property
	return true;
}

/**
 * @brief show properties.
 * @param id integer application id.
 */
void selected(int id)
{
	//! alpha temp
	int input, alpha;

	//! choice temp
	char choice;

	//! hwnd error flag
	bool hwnd_flag = false;

	//! menu sleep flag
	bool menu_flag = true;

	while (1)
	{
		menu_flag = true;

		dispSelectedHeader(id);

		printf(" 選択> ");
		choice = getChoice();

		switch (choice) {
			case 'C':	// change transparent by value
				while (1) {
					printf(" 透明度[%%] (ESCで戻る)> ");
					input = inputNumber();

					if (input == -1 || (input >= 0 && input <= 100 )) break;
					puts(" >>> 正しい数値を入力してください。");
				}

				if (input == -1) {
					menu_flag = false;
					break;
				}

				alpha = ((float)input / 100) * (float)255;
				if (transparentWindow(id, alpha))
					printf(" >>> ウィンドウの透明度を%d%%へ変更しました。\n", input);
				else
					hwnd_flag = true;

				break;
			case 'T':	// change transparent by arrow key
				hwnd_flag = !changeTransparentUsingArrowKey(id);
				menu_flag = hwnd_flag;
				break;

			default:
				menu_flag = false;
		}

		if (choice == 0x1B ) break;

		if (hwnd_flag) {
			puts("\n >>> HWNDが取得できませんでした。ホームへ戻ります。");
			Sleep(2000);
			break;
		}
		
		if(menu_flag)
			Sleep(2000);

	}
}

/**
 * @brief list applications and return the option.
 * @return option (character code).
 */
char listApplications()
{
	//! charactor code temp
	char ch;

	do {

		// clear the windows list.
		windowCounter = 0;
		for (int i = 0; i < 100; i++) {
			windows[i].pid = NULL;
			strcpy_s(windows[i].title, TEXT(""));
		}

		// clear the command line.
		system("cls");

		puts("+----------------------------------+                 +------------------------+");
		puts("| 現在起動しているアプリケーション |                 | ウィンドウ透明化ツール |");
		puts("+--------+-------------------------+-----------------+------------------------+");
		puts("| No一覧 |                       アプリケーション名                           |");
		puts("+--------+--------------------------------------------------------------------+");

		// display the applications.
		EnumWindows(EnumWindowsProc, (LPARAM)0);

		printf(" R:リスト更新, S:設定, X:すべて元に戻す, ESC:終了> ");
	} while ((ch = toupper(_getche())) == 'R');

	return ch;
}

/**
 * @brief get integer value.
 * @return integer value or -1 (ESC CODE).
 */
int inputNumber() {

	//! getch temp.
	char ch;

	//! string buffer max 255.
	char buf[255];

	//! buffer counter.
	int i = 0;

	//! escape flag.
	bool esc = false;

	while ((ch = _getch()) != 13) {
		if (ch >= '0' && ch <= '9') {
			putchar(ch);
			buf[i++] = ch;
		}

		//! 0x1B == ESC
		if (ch == 0x1B) {
			esc = true;
			break;
		}

		// backspace
		if (ch == '\b' && i > 0) {
			printf("\b \b");
			i--;
		}
	}

	if (esc) return -1;

	buf[i] = '\0';
	puts("");

	return atoi(buf);
}

/**
 * @brief the main function.
 * @return return the status code to system.
 */
int main()
{
	//! for windows id
	int id;

	//! for options
	char ch;

	system("title ウィンドウ透明化ツール");

	while (1)
	{
		//! list application list and get options.
		ch = listApplications();

		switch (ch) {
			case 'X':
				// set all windows to 100%
				setDefault();
				break;

			case 'S':
				puts("");

				while(1){
					printf(" Noを入力 (ESCで戻る)> ");
					id = inputNumber();
					if (id <= windowCounter || id < 0 && id != -1) break;
					puts(" >>> 正しいアプリケーション番号を入力してください。");
				};

				// back to the application list.
				if (id == -1) break;

				// go setting menu.
				selected(--id);
				break;
		}

		//! 0x1B == ESC
		if (ch == 0x1B) break;
	}

	// return code
	return 0;
}
