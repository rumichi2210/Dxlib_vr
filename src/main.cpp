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
	SetCameraScreenCenter(DXLIB_VR::GetHMDWidth()/2.0f, DXLIB_VR::GetHMDHeight()/2.0f); //�J���������Ă���f���̒��S���W���Đݒ�
	SetCameraNearFar(0.1f, 15000.0f);
	SetCameraViewMatrix(cmaraPos);
	MV1DrawModel(stage);
	SetDrawScreen(DX_SCREEN_BACK);//�`�������ɖ߂�
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	if (SetUseDirect3DVersion(DX_DIRECT3D_11) == -1) { return 0; }//open�ł�directX 11���g�p���邽�ߕύX
	if (SetGraphMode(2560, 1440, 32) == -1) { return 0; }//��ʃT�C�Y�̐ݒ�
	if (SetBackgroundColor(50, 50, 50) == -1) { return 0; }//�w�i�̐F���w��
	if (ChangeWindowMode(TRUE) == -1) { return 0; };
	SetWindowSizeExtendRate(1.0f);
	if (DxLib_Init() == -1) { return 0; } // DX���C�u��������������
	if (SetDrawScreen(DX_SCREEN_BACK) == -1) { return 0; } //�`���ʂ𗠉�ʂɐݒ�
	if (SetUseZBuffer3D(TRUE) == -1) { return 0; }// Z�o�b�t�@��L���ɂ���B
	if (SetWriteZBuffer3D(TRUE) == -1) { return 0; }// Z�o�b�t�@�ւ̏������݂�L���ɂ���B

	if (DXLIB_VR::Init()==false){
		DxLib_End();
		return 0;
	}

	vrEyeRight = MakeScreen(DXLIB_VR::GetHMDWidth(), DXLIB_VR::GetHMDHeight(), FALSE);
	vrEyeLeft = MakeScreen(DXLIB_VR::GetHMDWidth(), DXLIB_VR::GetHMDHeight(), FALSE);

	stage = MV1LoadModel(".\\res\\mmd_batokin_island\\batokin_island5.x");
	MV1SetScale(stage,VGet(3.0f, 3.0f, 3.0f));
	MV1SetPosition(stage, VGet(0.0f, 0.0f, 0.0f));
	
	// ����ʂ�\��ʂɔ��f, ���b�Z�[�W����, ��ʃN���A, �L�[�̍X�V)
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