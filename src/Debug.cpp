#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Debug.h"
#include "DxLib.h"
#include <commctrl.h>
#include <stdio.h>
#include <psapi.h>
#include <stdarg.h>
#define TITLE TEXT("Debug List Window")
#pragma comment(lib, "comctl32.lib")


/*コンソール用*/
char TitleBuffer[512];
HWND ConsoleWindow;
RECT WindowRect;
FILE* fp;
//コンソールスクリーンサイズ
constexpr SMALL_RECT rect{ 0, 0, 200, 80 };

constexpr CONSOLE_CURSOR_INFO cursor{ 1, FALSE };

CONSOLE_CURSOR_INFO init;

DATEDATA Date;//現在時刻

CONSOLE_SCREEN_BUFFER_INFO info;//コンソールデータ


/*その他ツール*/
int counter = 0, FpsTime[2] = { 0, }, FpsTime_i = 0;
double Fps = 0.0;
int Xn = 0;

//-------------------------------------
/*
LPTSTR strItem[] = { TEXT("TEST") , TEXT("TEST") , TEXT("TEST") , TEXT("TEST") };
LPTSTR strAttr[] = {
	TEXT("NULL") ,
	TEXT("NULL") ,
	TEXT("NULL"),
	TEXT("NULL")
};
*/



//-------------------------------------

////////////////////////////////////
// PrintMemoryInfoは
//https://docs.microsoft.com/ja-jp/windows/win32/psapi/collecting-memory-usage-information-for-a-process
// を元としています
///////////////////////////////////
#define DISP_PARAM(n)    (n), ((n) / 1024),((n)/1048576)
void PrintMemoryInfo(DWORD processID)
{
#ifdef _DEBUG
	HANDLE hProcess;
	PROCESS_MEMORY_COUNTERS pmc;

	// Print the process identifier.

	printf("\nProcess ID: %u\n", processID);

	// Print information about the memory usage of the process.

	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);
	if (NULL == hProcess)
		return;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
	
		printf(TEXT("プロセスID									：%lu\n"), processID);
		printf(TEXT("プロセス・ハンドル							：0x%p\n"), hProcess);
		printf(TEXT("ページフォールト数(ページ フォルト)			：%10lu個\n"), pmc.PageFaultCount);
		printf(TEXT("最大ワーキングセットのサイズ(最大メモリ使用量) ：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.PeakWorkingSetSize));
		printf(TEXT("現在ワーキングセットのサイズ(メモリ使用量)		：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.WorkingSetSize));
		printf(TEXT("最大ページプールの使用サイズ					：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.QuotaPeakPagedPoolUsage));
		printf(TEXT("現在ページプールの使用サイズ					：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.QuotaPagedPoolUsage));
		printf(TEXT("最大非ページプールの使用サイズ				：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.QuotaPeakNonPagedPoolUsage));
		printf(TEXT("現在非ページプールの使用サイズ(非ページ プール)：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.QuotaNonPagedPoolUsage));
		printf(TEXT("最大ページングファイルの使用サイズ				：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.PeakPagefileUsage));
		printf(TEXT("現在ページングファイルの使用サイズ(仮想メモリ サイズ)：%10luバイト(%lu KB)(%lu MB)\n"), DISP_PARAM(pmc.PagefileUsage));
	}

	CloseHandle(hProcess);
#endif // DEBUG
}



void CreateConsole() {
#ifdef _DEBUG
	SetMainWindowText("MainWindow (Debug Mode)");
	AllocConsole();
	SetConsoleTitle("Debug Console Window");
	freopen_s(&fp, "CONOUT$", "w", stdout); /* 標準出力(stdout)を新しいコンソールに向ける */
	freopen_s(&fp, "CONOUT$", "w", stderr); /* 標準エラー出力(stderr)を新しいコンソールに向ける */

	// コンソールウインドウのタイトルを取得	
	GetConsoleTitle(TitleBuffer, sizeof(TitleBuffer));

	// タイトルからウインドウを検索してウインドウハンドルを取得
	ConsoleWindow = FindWindow(NULL, TitleBuffer);

	// 現在のウインドウ矩形の位置を取得
	GetWindowRect(ConsoleWindow, &WindowRect);


	// ウインドウの左上端を( 0, 0 )、右下端を( WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top )に変更
	MoveWindow(ConsoleWindow, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);
	// デバッグコンソールがアクティブウィンドウになるので
	// ゲーム本体のウィンドウをアクティブにする
	SetForegroundWindow(GetMainWindowHandle());

	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &init); // カーソルの初期状態を得る。

	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor); // カーソルを不可視化する。

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_YELLOW);        // 描画文字をシアンに変更
	printf("===============================================================================\n");
	printf("\n");
	printf("%35s DebugTool\n", "");
	printf("%54s [v0.3A Update2022/11/23]\n", "");
	printf("===============================================================================\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_WHITE);        // 描画文字を白に変更
#endif // DEBUG
#ifndef _DEBUG//リリース用
	SetOutApplicationLogValidFlag(FALSE);//ログ出力を行うか否かのフラグ
	
	//デバッカーの検出
	if (IsDebuggerPresent() == 1) { MessageBox(NULL, TEXT("解析ソフトの動作が確認された為、ソフトを終了します\n誤作動の場合はご連絡ください"), TEXT("警告"), MB_OK | MB_ICONWARNING); DxLib_End(); }
#endif // RELEASE
}


void ConsoleUpdate(int flag) {
#ifdef _DEBUG
	GetDateTime(&Date);//現在の時刻を取得
	if (flag > 0) {
		//system("cls");//画面をクリア
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_GREEN);
		PrintMemoryInfo(GetCurrentProcessId());
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_WHITE);        // 描画文字を白に変更
	}



	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);//カーソールの位置を取得
	
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_CYAN);        // 描画文字をシアンに変更
	
	/*FpsMeasurement();*/

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0,info.dwCursorPosition.Y });
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_WHITE);        // 描画文字を白に変更
#endif // DEBUG
}

void FpsMeasurement() {
#ifdef _DEBUG
	if (FpsTime_i == 0)
		FpsTime[0] = GetNowCount();               //1周目の時間取得
	if (FpsTime_i == 49) {
		FpsTime[1] = GetNowCount();               //50周目の時間取得
		Fps = 1000.0f / ((FpsTime[1] - FpsTime[0]) / 50.0f);//測定した値からfpsを計算
		FpsTime_i = 0;//カウントを初期化
	}
	else
		FpsTime_i++;//現在何周目かカウント
	if (Fps != 0)
	printf("現在時刻%02d時%02d分%02d秒", Date.Hour, Date.Min, Date.Sec);
	printf("%2s","");
	printf("FPS:%.1f",Fps);
	printf("%2s", "");
	printf("LoadTask=%d",GetASyncLoadNum());
	//printf("LoadTask=%d,DXlibFPS=%f", GetASyncLoadNum(), GetFPS());
	printf("%2s", "");
	printf("F2キーでメモリー量を表示します");
	//PrintMemoryInfo(GetCurrentProcessId());
#endif // DEBUG
}

void SendConsole(const char* commentString,...) {
#ifdef _DEBUG
	va_list VaList;
	TCHAR String[2048];
	va_start(VaList, commentString);
	vsnprintf(String, sizeof(String) / sizeof(TCHAR), commentString, VaList);
	va_end(VaList);
	GetDateTime(&Date);//現在の時刻を取得
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);//カーソールの位置を取得
	printf("%1000s\n", "");//一度一行を空白で埋める
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0,info.dwCursorPosition.Y });

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_CYAN);
	printf("[%02d時%02d分%02d秒]", Date.Hour, Date.Min, Date.Sec);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_WHITE);
	printf("%s\n",String);
#endif // DEBUG
}

void SendConsoleC(_In_ WORD stringColor, const char* commentString, ...) {
#ifdef _DEBUG
	va_list VaList;
	TCHAR String[2048];
	va_start(VaList, commentString);
	vsnprintf(String, sizeof(String) / sizeof(TCHAR), commentString, VaList);
	va_end(VaList);
	GetDateTime(&Date);//現在の時刻を取得
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);//カーソールの位置を取得
	printf("%1000s\n", "");//一度一行を空白で埋める
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0,info.dwCursorPosition.Y });

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_CYAN);
	printf("[%02d時%02d分%02d秒]", Date.Hour, Date.Min, Date.Sec);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), stringColor);
	printf("%s\n", String);
#endif // DEBUG
}

void GetPCInfo() {
#ifdef _DEBUG
	char OSString[256];
	char DirectXString[256];
	char CPUString[256];
	int* CPUSpeed=0; /* 単位MHz */
	double* FreeMemorySize=0; /* 単位MByte */
	double* TotalMemorySize=0;
	char VideoDriverFileName[256];
	char VideoDriverString[256];
	double* FreeVideoMemorySize=0; /* 単位MByte */
	double* TotalVideoMemorySize=0;

	GetPcInfo(OSString, DirectXString, CPUString, CPUSpeed /* 単位MHz */, FreeMemorySize /* 単位MByte */, TotalMemorySize, VideoDriverFileName, VideoDriverString, FreeVideoMemorySize /* 単位MByte */, TotalVideoMemorySize);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_RED);        // 描画文字をシアンに変更
	printf("\n");//改行
	printf("アプリケーションの初期構築に成功しました。PCの情報を表示します\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_YELLOW);        // 描画文字をシアンに変更
	printf("===============================================================================\n");
	printf("OS_Ver=%s\n", OSString);
	printf("DirectX_Ver=%s\n", DirectXString);
	printf("CPU=%s\n", CPUString);
	//printf("CPUSpeed=%x[MHz]\n", CPUSpeed);
	//printf("FreeMemorySize=%lf[MByte]\n", FreeMemorySize);
	//printf("TotalMemorySize=%lf[MByte]\n", TotalMemorySize);
	printf("GraphicBoard=%s\n", VideoDriverString);
	printf("VideoDriverDLL=%s\n", VideoDriverFileName);
	//printf("FreeVideoMemorySize=%lf[MByte]\n", FreeVideoMemorySize);
	//printf("TotalVideoMemorySize=%lf[MByte]\n", TotalVideoMemorySize);
	printf("\n");
	printf("===============================================================================\n");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), COL_WHITE);        // 描画文字を白に変更

#endif // DEBUG

}



/*
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
	LVCOLUMN col;
	LVITEM item = { 0 };
	int iCount = 0;
	static HWND hList;

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CREATE:
		InitCommonControls();
		hList = CreateWindowEx(0, WC_LISTVIEW, 0,
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EDITLABELS,
			0, 0, 10, 10, hWnd, (HMENU)1,
			((LPCREATESTRUCT)lp)->hInstance, NULL);

		col.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		col.fmt = LVCFMT_LEFT;
		col.cx = 100;
		col.iSubItem = 0;
		col.pszText = TEXT("変数名");
		ListView_InsertColumn(hList, 0, &col);

		col.iSubItem = 1;
		col.pszText = TEXT("数値");
		ListView_InsertColumn(hList, 1, &col);

		item.mask = LVIF_TEXT;
		for (; iCount < (sizeof(strItem)/sizeof(strItem[0])); iCount++) {
			item.pszText = strItem[iCount];
			item.iItem = iCount;
			item.iSubItem = 0;
			ListView_InsertItem(hList, &item);

			item.pszText = strAttr[iCount];
			item.iItem = iCount;
			item.iSubItem = 1;
			ListView_SetItem(hList, &item);
		}
		return 0;
	case WM_SIZE:
		MoveWindow(hList, 0, 0, LOWORD(lp), HIWORD(lp), FALSE);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

*/
/*
int listViwe(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;
	WNDCLASS winc;

	winc.style = CS_HREDRAW | CS_VREDRAW;
	winc.lpfnWndProc = WndProc;
	winc.cbClsExtra = winc.cbWndExtra = 0;
	winc.hInstance = hInstance;
	winc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	winc.lpszMenuName = NULL;
	winc.lpszClassName = TEXT("Debug");

	if (!RegisterClass(&winc)) return -1;

	hWnd = CreateWindow(
		TEXT("Debug"), TITLE,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL
	);
	if (hWnd == NULL) return -1;
	return 0;
}

*/
