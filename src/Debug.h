#pragma once
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "DxLib.h"


#define     COL_BLACK   0x00
#define     COL_DARK_BLUE       0x01
#define     COL_DARK_GREEN 0x02
#define     COL_DARK_CYAN       0x03
#define     COL_DARK_RED     0x04
#define     COL_DARK_VIOLET0x05
#define     COL_DARK_YELLOW   0x06
#define     COL_GRAY 0x07
#define     COL_LIGHT_GRAY      0x08
#define     COL_BLUE     0x09
#define     COL_GREEN   0x0a
#define     COL_CYAN     0x0b
#define     COL_RED      0x0c
#define     COL_VIOLET  0x0d
#define     COL_YELLOW 0x0e
#define     COL_WHITE   0x0f
#define     COL_INTENSITY     0x08     // 高輝度マスク
#define     COL_RED_MASK     0x04     // 赤色ビット
#define     COL_GREEN_MASK 0x02     // 緑色ビット
#define     COL_BLUE_MASK   0x01     //  青色ビット

/**
	@brief	デバッグ用コンソールを表示します
*/
void CreateConsole();

/**
	@brief	デバッグ用コンソールを更新します
	@param	flag 0の場合上書き 1の場合消去
*/
void ConsoleUpdate(int flag);


/**
	@brief	デバッグ用コンソールにFPSを出力します
*/
inline void FpsMeasurement();


/**
	@brief	デバッグ用コンソールにメッセージを送ります
*/
void SendConsole(const char* commentString, ...);

/**
	@brief	デバッグ用コンソールにメッセージを送ります(カラー対応)
	@param BLACK   0x00
	@param DARK_BLUE       0x01
	@param DARK_GREEN 0x02
	@param DARK_CYAN       0x03
	@param DARK_RED     0x04
	@param DARK_VIOLET0x05
	@param DARK_YELLOW   0x06
	@param GRAY 0x07
	@param LIGHT_GRAY      0x08
	@param BLUE     0x09
	@param GREEN   0x0a
	@param CYAN     0x0b
	@param RED      0x0c
	@param VIOLET  0x0d
	@param YELLOW 0x0e
	@param WHITE   0x0f
	@param INTENSITY     0x08     // 高輝度マスク
	@param RED_MASK     0x04     // 赤色ビット
	@param GREEN_MASK 0x02     // 緑色ビット
	@param BLUE_MASK   0x01     //  青色ビット
*/
void SendConsoleC(_In_ WORD stringColor, const char* commentString, ...);

/**
	@brief	PC情報をコンソールに出力します
*/
void GetPCInfo();
//------------------------------------------------

//int listViwe(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) ;

//LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);