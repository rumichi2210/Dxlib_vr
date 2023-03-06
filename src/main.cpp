#include "DxLib.h"
#include "vr.hpp"
#include <memory>

int stage;

inline void modelDraw() {
	DxLib::MV1DrawModel(stage);
}

inline int vrLoadModel(const TCHAR* FileName) {
	int temp = DxLib::MV1LoadModel(FileName);
	for (int i = 0; i < DxLib::MV1GetMeshNum(temp); i++)
	{
		DxLib::MV1SetMeshBackCulling(temp, i, DX_CULLING_RIGHT);
	}
	return temp;
}

//コンソールスクリーンサイズ
constexpr SMALL_RECT rect{ 0, 0, 200, 80 };
constexpr CONSOLE_CURSOR_INFO cursor{ 1, FALSE };
CONSOLE_CURSOR_INFO init;

void SetMainCamera(float nearClip, float farClip, VECTOR position, VECTOR target) {
	MATRIX mat_projective;//射影行列のアドレス
	DxLib::CreatePerspectiveFovMatrixRH(&mat_projective, DEFAULT_FOV, nearClip, farClip);// 射影行列を作成する
	DxLib::SetupCamera_ProjectionMatrix(mat_projective);// 射影行列を直接設定する

	MATRIX mat_view;// ビューマトリクス
	VECTOR vec_up = VGet(0.0f, 1.0f, 0.0f);// カメラの上方向
	DxLib::CreateLookAtMatrixRH(&mat_view, &position, &target, &vec_up);
	DxLib::SetCameraViewMatrix(mat_view);//ビュー行列を直接設定する
}

void InitConsole() {
	char TitleBuffer[512];
	HWND ConsoleWindow;
	RECT WindowRect;
	FILE* fp;

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
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//メモリリーク検出->Debug時のみ有功になります
	if (DxLib::SetUseDirect3DVersion(DX_DIRECT3D_11) == -1) { return 0; }//OpenVRではdirectX 11を使用するため変更
	if (DxLib::SetGraphMode(2560, 1440, 32) == -1) { return 0; }//画面サイズの設定
	if (DxLib::SetBackgroundColor(50, 50, 50) == -1) { return 0; }//背景の色を指定
	if (DxLib::ChangeWindowMode(TRUE) == -1) { return 0; };
	DxLib::SetWindowSizeExtendRate(1.0f);
	DxLib::SetZBufferBitDepth(32);
	DxLib::SetCreateDrawValidGraphZBufferBitDepth(32);
	DxLib::SetWaitVSyncFlag(false);//HMDのFPSが高くても、モニターのFPS基準になってしまうため垂直同期をoffにする 
	DxLib::SetUseLarge3DPositionSupport(true);
	if (DxLib::DxLib_Init() == -1) { return 0; } // DXライブラリ初期化処理
	DxLib::SetUseRightHandClippingProcess(TRUE);//OpenVRでは右手座標系のためクリッピング処理を変更する
	if (DxLib::SetDrawScreen(DX_SCREEN_BACK) == -1) { return 0; } //描画画面を裏画面に設定
	if (DxLib::SetUseZBuffer3D(TRUE) == -1) { return 0; }// Zバッファを有効にする。
	if (DxLib::SetWriteZBuffer3D(TRUE) == -1) { return 0; }// Zバッファへの書き込みを有効にする。

	InitConsole();
	DxLib::SetCreateDrawValidGraphMultiSample(8, 8);
	auto vrHandler = std::make_unique<OpenvrForDXLib>(0.1f, 15000.0f);
	if (!vrHandler || !vrHandler->vrCheck()) {
		MessageBox(NULL, TEXT("警告"), TEXT("openVRを初期化できませんでした"), MB_OK);
		DxLib::DxLib_End();
		return 0;
	}

	stage = vrLoadModel(".\\res\\mmd_batokin_island\\batokin_island5.x");
	DxLib::MV1SetScale(stage, VGet(3.0f, 3.0f, 3.0f));
	DxLib::MV1SetPosition(stage, VGet(0.0f, 0.0f, 0.0f));

	printf("Sキーで計測開始");
	while (CheckHitKey(KEY_INPUT_S) == 0 && DxLib::ProcessMessage() == 0) {}
	system("cls");//画面をクリア
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0,0 });

	float sx = 0.0f;
	float sy = 50.0f;
	float sz = 100.0f;

	// 裏画面を表画面に反映, メッセージ処理, 画面クリア, キーの更新)
	while (DxLib::ScreenFlip() == 0 && DxLib::ProcessMessage() == 0 && DxLib::ClearDrawScreen() == 0) {
		vrHandler->UpdateState(VGet(0.0f,20.0f,0.0f));
		vrHandler->UpdateVRScreen(vr::Eye_Right, modelDraw);
		vrHandler->UpdateVRScreen(vr::Eye_Left,modelDraw);

		DxLib::SetCameraScreenCenter(1280, 720);
		SetMainCamera(0.1f, 15000.0f, VGet(sx, sy, sz), VGet(0.0f, 0.0f, 0.0f));
		DxLib::DrawSphere3D(vrHandler->GetHMDPos(), 1.0f, 32, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);

		modelDraw();
		if (CheckHitKey(KEY_INPUT_A) != 0) { sx -= 0.1f; }
		if (CheckHitKey(KEY_INPUT_D) != 0) { sx += 0.1f; }
		if (CheckHitKey(KEY_INPUT_S) != 0) { sy -= 0.1f; }
		if (CheckHitKey(KEY_INPUT_W) != 0) { sy += 0.1f; }
		if (CheckHitKey(KEY_INPUT_Q) != 0) { sz -= 0.1f; }
		if (CheckHitKey(KEY_INPUT_E) != 0) { sz += 0.1f; }
		if (CheckHitKey(KEY_INPUT_F10) != 0) { return 0; }

		DrawFormatString(0, 0, GetColor(255, 255, 255), "sx=%f,sy=%f,sz+%f", sx, sy, sz);
	}

	DxLib::DxLib_End();

	return 0;
}