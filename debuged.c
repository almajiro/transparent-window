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

		_tprintf(TEXT("| ID.%02d | "), structCounter);

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

void setDefault()
{
	for(int i=0;i<structCounter; i++){
		transparentWindow(i, 255);
	}
	puts("\nすべてのアプリケーションの透明度を元に戻しました。");
	Sleep(3000);
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
			puts("+----------------------------------+                 +------------------------+");
			puts("| 現在起動しているアプリケーション |                 | ウィンドウ透明化ツール |");
			puts("+-------+--------------+-----------------------------+------------------------+");
			puts("| ID一覧|                     アプリケーション名                              |");
			puts("+-------+--------------+-----------------------------+------------------------+");

			EnumWindows(EnumWindowsProc, (LPARAM)&nCount);
			printf("r:リスト更新, s:設定, x:すべて元に戻す, other:終了> ");
		} while ((ch = toupper(_getch())) == 'R');

		if(ch == 'X'){
			setDefault();
		}

		if (ch != 'X' && ch != 'S') break;

		if(ch == 'S'){
			puts("");
			printf("ID番号を入力> ");
			scanf_s("%d", &id);

			if (structCounter < id || id <= 0) {
				do {
					fflush(stdin);
					puts("正しいアプリケーションIDを入力してください。");
					Sleep(1000);
					printf("ID番号を入力> ");
					scanf_s("%d", &id);
				} while (structCounter < id || id <= 0);
			}

			id--;
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

			puts(" T > 透明度を矢印キーで設定");
			puts(" C > 透明度を数値で入力");
			puts(" X > アプリケーション一覧へ戻る");
			puts("-------------------------------------------------------------------------------");
			printf("選択> ");
			ch = _getche();

			ch = toupper(ch);

			if(ch == 'C') {
				printf("\r透明度[%%]> ");
				scanf_s("%d", &input);

				if (100 < input || input <= 0) {
					do {
						fflush(stdin);
						puts("正しい数値を入力してください。");
						Sleep(1000);
						printf("\r透明度[%%]> ");
						scanf_s("%d", &input);
					} while (100 < input || input <= 0);
				}

				alpha = ((float)input / 100) * (float)255;

				if (transparentWindow(id, alpha)){
					printf(">> ウィンドウの透明度を%d%%へ変更しました。\n", input);
					puts("'X'を押して戻る。");
					while(1)
						if((ch = toupper(getch())) == 'X' || ch == 0x1B) break;
				}
				else {
					puts(">> 透明度の設定に失敗しました。");
					puts("'X'を押して戻る。");
					while(1)
						if((ch = toupper(getch())) == 'X' || ch == 0x1B) break;
				}
			}
			else if (ch == 'T') {
				alpha = 255;
				transparentWindow(id, alpha);
				puts("\n'X'を押して戻る。");
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
					printf("%003.f%%", ((float)alpha / (float)255 * 100) );
					printf("] ");

					ch = _getch();
					if (ch == 0x48 || ch == 0x4d) {
						if (alpha < 255) {
							alpha += 5;
							if (!transparentWindow(id, alpha)) break;
						}
					}
					if (ch == 0x50 || ch == 0x4b) {
						if (alpha > 5) {
							alpha -= 5;
							if (!transparentWindow(id, alpha)) break;
						}
					}

					if(toupper(ch) == 'X' || ch == 0x1B) break;

					Sleep(1);
				}
			}
		}
	}

    return 0;
}
