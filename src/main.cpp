#include "DxLib.h"
#include "vr.hpp"


int vrEyeRight;
int vrEyeLeft;
int stage;
int chara;

int cont;

//�R���\�[���X�N���[���T�C�Y
constexpr SMALL_RECT rect{ 0, 0, 200, 80 };
constexpr CONSOLE_CURSOR_INFO cursor{ 1, FALSE };
CONSOLE_CURSOR_INFO init;
/*�R���\�[���p*/
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
	DxLib::SetCameraScreenCenter(DXLIB_VR::GetHMDWidth() / 2.0f, DXLIB_VR::GetHMDHeight() / 2.0f); //�J���������Ă���f���̒��S���W���Đݒ�
	DxLib::SetupCamera_ProjectionMatrix(projection);
	DxLib::SetCameraViewMatrix(view);
	DxLib::MV1DrawModel(stage);
	DxLib::MV1DrawModel(cont);
	DxLib::SetDrawScreen(DX_SCREEN_BACK);//�`�������ɖ߂�
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
	if (DxLib::SetUseDirect3DVersion(DX_DIRECT3D_11) == -1) { return 0; }//OpenVR�ł�directX 11���g�p���邽�ߕύX
	if (DxLib::SetGraphMode(2560, 1440, 32) == -1) { return 0; }//��ʃT�C�Y�̐ݒ�
	if (DxLib::SetBackgroundColor(50, 50, 50) == -1) { return 0; }//�w�i�̐F���w��
	if (DxLib::ChangeWindowMode(TRUE) == -1) { return 0; };
	DxLib::SetWindowSizeExtendRate(1.0f);
	DxLib::SetZBufferBitDepth(32);
	DxLib::SetCreateDrawValidGraphZBufferBitDepth(32);
	DxLib::SetWaitVSyncFlag(false);//HMD��FPS�������Ă��A���j�^�[��FPS��ɂȂ��Ă��܂����ߐ���������off�ɂ��� 
	DxLib::SetUseLarge3DPositionSupport(true);
	if (DxLib::DxLib_Init() == -1) { return 0; } // DX���C�u��������������
	DxLib::SetUseRightHandClippingProcess(TRUE);//OpenVR�ł͉E����W�n�̂��߃N���b�s���O������ύX����
	if (DxLib::SetDrawScreen(DX_SCREEN_BACK) == -1) { return 0; } //�`���ʂ𗠉�ʂɐݒ�
	if (DxLib::SetUseZBuffer3D(TRUE) == -1) { return 0; }// Z�o�b�t�@��L���ɂ���B
	if (DxLib::SetWriteZBuffer3D(TRUE) == -1) { return 0; }// Z�o�b�t�@�ւ̏������݂�L���ɂ���B
	
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



	// ����ʂ�\��ʂɔ��f, ���b�Z�[�W����, ��ʃN���A, �L�[�̍X�V)
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