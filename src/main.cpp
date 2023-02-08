#include "DxLib.h"
#include "vr.hpp"


int vrEyeRight;
int vrEyeLeft;
int stage;
int chara;

void UpdateCameraScreen(vr::Hmd_Eye nEye, MATRIX cmaraPos)
{
	if (nEye == vr::Eye_Right) { SetDrawScreen(vrEyeRight); }
	if (nEye == vr::Eye_Left) { SetDrawScreen(vrEyeLeft); }
	ClearDrawScreenZBuffer();
	ClearDrawScreen();
	SetCameraScreenCenter(DXLIB_VR::GetHMDWidth()/2.0f, DXLIB_VR::GetHMDHeight()/2.0f); //カメラが見ている映像の中心座標を再設定
	SetCameraNearFar(0.1f, 15000.0f);
	SetCameraViewMatrix(cmaraPos);
	MV1DrawModel(stage);
	SetDrawScreen(DX_SCREEN_BACK);//描画先を元に戻す
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
		
		UpdateCameraScreen(vr::Eye_Right, DXLIB_VR::GetEyeMat(vr::Eye_Right));
		UpdateCameraScreen(vr::Eye_Left, DXLIB_VR::GetEyeMat(vr::Eye_Left));
		DXLIB_VR::putTex((ID3D11Texture2D*)GetGraphID3D11Texture2D(vrEyeRight), vr::Eye_Right);
		DXLIB_VR::putTex((ID3D11Texture2D*)GetGraphID3D11Texture2D(vrEyeLeft), vr::Eye_Left);     
			 
		SetCameraPositionAndTarget_UpVecY(VGet(0.0f, 500.0f, 100.0f), VGet(0.0f, 0.0f, 0.0f));
		SetCameraScreenCenter(1280,720);
		MV1DrawModel(stage);
		/*
		DXLIB_VR::putTex((ID3D11Texture2D*)GetUseDirect3D11BackBufferTexture2D(), vr::Eye_Right);
		DXLIB_VR::putTex((ID3D11Texture2D*)GetUseDirect3D11BackBufferTexture2D(), vr::Eye_Left);
		*/


		DXLIB_VR::render();
	}

	DXLIB_VR::Fin();

	DxLib_End();

	return 0;
}