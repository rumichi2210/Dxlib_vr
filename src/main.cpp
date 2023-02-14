#include "DxLib.h"
#include "vr.hpp"`


int vrEyeRight;
int vrEyeLeft;
int stage;
int chara;

//�R���\�[���X�N���[���T�C�Y
constexpr SMALL_RECT rect{ 0, 0, 200, 80 };
constexpr CONSOLE_CURSOR_INFO cursor{ 1, FALSE };
CONSOLE_CURSOR_INFO init;
/*�R���\�[���p*/
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
	SetCameraScreenCenter(DXLIB_VR::GetHMDWidth()/2.0f, DXLIB_VR::GetHMDHeight()/2.0f); //�J���������Ă���f���̒��S���W���Đݒ�
	SetCameraNearFar(0.1f, 15000.0f);
	//SetTransformToProjection(&projection);
	//SetupCamera_ProjectionMatrix(projection);//<-Direct X 11�p�̃T���v���̓v���W�F�N�V�������������Ă���H(projection��ݒ肵���ꍇ�͕\���s�ɂȂ��Ă��܂�)
	SetCameraViewMatrix(view);
	MV1DrawModel(stage);
	SetDrawScreen(DX_SCREEN_BACK);//�`�������ɖ߂�
}

void InitConsole() {
	AllocConsole();
	SetConsoleTitle("Console Window");
	freopen_s(&fp, "CONOUT$", "w", stdout); /* �W���o��(stdout)��V�����R���\�[���Ɍ����� */
	freopen_s(&fp, "CONOUT$", "w", stderr); /* �W���G���[�o��(stderr)��V�����R���\�[���Ɍ����� */
	GetConsoleTitle(TitleBuffer, sizeof(TitleBuffer));// �R���\�[���E�C���h�E�̃^�C�g�����擾	
	ConsoleWindow = FindWindow(NULL, TitleBuffer);// �^�C�g������E�C���h�E���������ăE�C���h�E�n���h�����擾
	GetWindowRect(ConsoleWindow, &WindowRect);// ���݂̃E�C���h�E��`�̈ʒu���擾
	MoveWindow(ConsoleWindow, 0, 0, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, TRUE);
	SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE), TRUE, &rect);
	SetForegroundWindow(GetMainWindowHandle());// �Q�[���{�̂̃E�B���h�E���A�N�e�B�u�ɂ���
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &init); // �J�[�\���̏�����Ԃ𓾂�B
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor); // �J�[�\����s��������B
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
	
	// ����ʂ�\��ʂɔ��f, ���b�Z�[�W����, ��ʃN���A, �L�[�̍X�V)

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