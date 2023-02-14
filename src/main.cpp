#include "DxLib.h"
#include "vr.hpp"`


int vrEyeRight;
int vrEyeLeft;
int stage;
int chara;

//コンソールスクリーンサイズ
constexpr SMALL_RECT rect{ 0, 0, 200, 80 };
constexpr CONSOLE_CURSOR_INFO cursor{ 1, FALSE };
CONSOLE_CURSOR_INFO init;
/*コンソール用*/
char TitleBuffer[512];
HWND ConsoleWindow;
RECT WindowRect;
FILE* fp;


void UpdateCameraScreen(vr::Hmd_Eye nEye, MATRIX view,MATRIX projection)
{
	if (nEye == vr::Eye_Right) { SetDrawScreen(vrEyeRight); }
	if (nEye == vr::Eye_Left) { SetDrawScreen(vrEyeLeft); }
	ClearDrawScreenZBuffer();
	ClearDrawScreen();
	SetCameraScreenCenter(DXLIB_VR::GetHMDWidth()/2.0f, DXLIB_VR::GetHMDHeight()/2.0f); //カメラが見ている映像の中心座標を再設定
	SetCameraNearFar(0.1f, 15000.0f);
	//SetTransformToProjection(&projection);
	//SetupCamera_ProjectionMatrix(projection);//<-Direct X 11用のサンプルはプロジェクションだけ送っている？(projectionを設定した場合は表示不可になってしまう)
	SetCameraViewMatrix(view);
	MV1DrawModel(stage);
	SetDrawScreen(DX_SCREEN_BACK);//描画先を元に戻す
}

void InitConsole() {
	AllocConsole();
	SetConsoleTitle("Console Window");
	freopen_s(&fp, "CONOUT$", "w", stdout); /* 標準出力(stdout)を新しいコンソールに向ける */
	freopen_s(&fp, "CONOUT$", "w", stderr); /* 標準エラー出力(stderr)を新しいコンソールに向ける */
	GetConsoleTitle(TitleBuffer, sizeof(TitleBuffer));// コンソールウインドウのタイトルを取得	
	ConsoleWindow = FindWindow(NULL, TitleBuffer);// タイトルからウインドウを検索してウインドウハンドルを取得
	GetWindowRect(ConsoleWindow, &WindowRect);// 現在のウインドウ矩形の位置を取得
	MoveWindow(ConsoleWindow, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);
	SetForegroundWindow(GetMainWindowHandle());// ゲーム本体のウィンドウをアクティブにする
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &init); // カーソルの初期状態を得る。
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor); // カーソルを不可視化する。
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	if (SetUseDirect3DVersion(DX_DIRECT3D_11) == -1) { return 0; }//openではdirectX 11を使用するため変更
	if (SetGraphMode(2560, 1440, 32) == -1) { return 0; }//画面サイズの設定
	if (SetBackgroundColor(50, 50, 50) == -1) { return 0; }//背景の色を指定
	if (ChangeWindowMode(TRUE) == -1) { return 0; };
	SetWindowSizeExtendRate(1.0f);
	if (DxLib_Init() == -1) { return 0; } // DXライブラリ初期化処理
	if (SetDrawScreen(DX_SCREEN_BACK) == -1) { return 0; } //描画画面を裏画面に設定
	if (SetUseZBuffer3D(TRUE) == -1) { return 0; }// Zバッファを有効にする。
	if (SetWriteZBuffer3D(TRUE) == -1) { return 0; }// Zバッファへの書き込みを有効にする。

	InitConsole();

	if (DXLIB_VR::Init()==false){
		DxLib_End();
		return 0;
	}

	vrEyeRight = MakeScreen(DXLIB_VR::GetHMDWidth(), DXLIB_VR::GetHMDHeight(), FALSE);
	vrEyeLeft = MakeScreen(DXLIB_VR::GetHMDWidth(), DXLIB_VR::GetHMDHeight(), FALSE);

	stage = MV1LoadModel(".\\res\\mmd_batokin_island\\batokin_island5.x");
	MV1SetScale(stage,VGet(3.0f, 3.0f, 3.0f));
	MV1SetPosition(stage, VGet(0.0f, 0.0f, 0.0f));
	
	// 裏画面を表画面に反映, メッセージ処理, 画面クリア, キーの更新)

	while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0) {
		DXLIB_VR::updateVRState();
		
		UpdateCameraScreen(vr::Eye_Right, DXLIB_VR::GetViewMat(vr::Eye_Right), DXLIB_VR::GetProjectMat(vr::Eye_Right));
		UpdateCameraScreen(vr::Eye_Left, DXLIB_VR::GetViewMat(vr::Eye_Left), DXLIB_VR::GetProjectMat(vr::Eye_Left));
		DXLIB_VR::putTex((ID3D11Texture2D*)GetGraphID3D11Texture2D(vrEyeRight), vr::Eye_Right);
		DXLIB_VR::putTex((ID3D11Texture2D*)GetGraphID3D11Texture2D(vrEyeLeft), vr::Eye_Left);     
			 
		SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 500.0f, 100.0f), VGet(0.0f, 0.0f, 0.0f));
		SetCameraScreenCenter(1280,720);
		MV1DrawModel(stage);

		DXLIB_VR::render();
	}

	DXLIB_VR::Fin();

	DxLib_End();

	return 0;
}