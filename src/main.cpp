#include "DxLib.h"
#include "vr.hpp"


int vrEyeRight;
int vrEyeLeft;
int stage;
int chara;

int cont;

//コンソールスクリーンサイズ
constexpr SMALL_RECT rect{ 0, 0, 200, 80 };
constexpr CONSOLE_CURSOR_INFO cursor{ 1, FALSE };
CONSOLE_CURSOR_INFO init;
/*コンソール用*/
char TitleBuffer[512];
HWND ConsoleWindow;
RECT WindowRect;
FILE* fp;


void UpdateCameraScreen(vr::Hmd_Eye nEye, MATRIX view, MATRIX projection)
{
	if (nEye == vr::Eye_Right) { DxLib::SetDrawScreen(vrEyeRight); }
	if (nEye == vr::Eye_Left) { DxLib::SetDrawScreen(vrEyeLeft); }
	DxLib::ClearDrawScreenZBuffer();
	DxLib::ClearDrawScreen();
	DxLib::SetCameraScreenCenter(DXLIB_VR::GetHMDWidth() / 2.0f, DXLIB_VR::GetHMDHeight() / 2.0f); //カメラが見ている映像の中心座標を再設定
	DxLib::SetupCamera_ProjectionMatrix(projection);
	DxLib::SetCameraViewMatrix(view);
	DxLib::MV1DrawModel(stage);
	DxLib::MV1DrawModel(cont);
	DxLib::SetDrawScreen(DX_SCREEN_BACK);//描画先を元に戻す
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

	if (DXLIB_VR::Init()==false){
		DxLib::DxLib_End();
		return 0;
	}
	int MultiSamplBit = 16;
	printf("SampleQualityMAX->%d", DxLib::GetMultiSampleQuality(MultiSamplBit));
	DxLib::SetCreateDrawValidGraphMultiSample(MultiSamplBit, 16);
	vrEyeRight = DxLib::MakeScreen(DXLIB_VR::GetHMDWidth(), DXLIB_VR::GetHMDHeight(), FALSE);
	vrEyeLeft = DxLib::MakeScreen(DXLIB_VR::GetHMDWidth(), DXLIB_VR::GetHMDHeight(), FALSE);

	stage = DxLib::MV1LoadModel(".\\res\\mmd_batokin_island\\batokin_island5.x");
	for (int i = 0; i < DxLib::MV1GetMeshNum(stage); i++)
	{
		DxLib::MV1SetMeshBackCulling(stage, i, DX_CULLING_RIGHT);
	}
	DxLib::MV1SetScale(stage,VGet(3.0f, 3.0f, 3.0f));
	DxLib::MV1SetPosition(stage, VGet(0.0f, 0.0f, 0.0f));

	cont = DxLib::MV1LoadModel(".\\res\\vive_ Controler.mv1");
	for (int i = 0; i < DxLib::MV1GetMeshNum(cont); i++)
	{
		DxLib::MV1SetMeshBackCulling(cont, i, DX_CULLING_RIGHT);
	}

	int DispFPS = 0;
	int FPSCount = 0;
	int FPSTime = GetNowCount();



	// 裏画面を表画面に反映, メッセージ処理, 画面クリア, キーの更新)
	while (DxLib::ScreenFlip() == 0 && DxLib::ProcessMessage() == 0 && DxLib::ClearDrawScreen() == 0) {
		DXLIB_VR::updateVRState();
		DxLib::MV1SetMatrix(cont, DxLib::MMult(DxLib::MGetScale(DxLib::VGet(0.01f,0.01f,0.01f)),DXLIB_VR::GetContolloer()));

		DrawSphere3D(DXLIB_VR::GetLeftContolloer(), 80.0f, 32, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);
		UpdateCameraScreen(vr::Eye_Right, DXLIB_VR::GetViewMat2(vr::Eye_Right), DXLIB_VR::GetProjectiontMat(vr::Eye_Right));
		UpdateCameraScreen(vr::Eye_Left, DXLIB_VR::GetViewMat2(vr::Eye_Left), DXLIB_VR::GetProjectiontMat(vr::Eye_Left));
		DXLIB_VR::putTex((ID3D11Texture2D*)DxLib::GetGraphID3D11Texture2D(vrEyeRight), vr::Eye_Right);
		DXLIB_VR::putTex((ID3D11Texture2D*)DxLib::GetGraphID3D11Texture2D(vrEyeLeft), vr::Eye_Left);
			 
		DxLib::SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 500.0f, 100.0f), VGet(0.0f, 0.0f, 0.0f));
		DxLib::SetCameraScreenCenter(1280,720);
		DxLib::MV1DrawModel(stage);

		FPSCount++;
		int NowTime = GetNowCount();
		if (NowTime - FPSTime >= 1000)
		{
			DispFPS = FPSCount;
			FPSCount = 0;
			FPSTime = NowTime;
		}
		DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS:%d", DispFPS);

		DXLIB_VR::render();
	}

	DXLIB_VR::Fin();

	DxLib::DxLib_End();

	return 0;
}