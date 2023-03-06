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

//�R���\�[���X�N���[���T�C�Y
constexpr SMALL_RECT rect{ 0, 0, 200, 80 };
constexpr CONSOLE_CURSOR_INFO cursor{ 1, FALSE };
CONSOLE_CURSOR_INFO init;

void SetMainCamera(float nearClip, float farClip, VECTOR position, VECTOR target) {
	MATRIX mat_projective;//�ˉe�s��̃A�h���X
	DxLib::CreatePerspectiveFovMatrixRH(&mat_projective, DEFAULT_FOV, nearClip, farClip);// �ˉe�s����쐬����
	DxLib::SetupCamera_ProjectionMatrix(mat_projective);// �ˉe�s��𒼐ڐݒ肷��

	MATRIX mat_view;// �r���[�}�g���N�X
	VECTOR vec_up = VGet(0.0f, 1.0f, 0.0f);// �J�����̏����
	DxLib::CreateLookAtMatrixRH(&mat_view, &position, &target, &vec_up);
	DxLib::SetCameraViewMatrix(mat_view);//�r���[�s��𒼐ڐݒ肷��
}

void InitConsole() {
	char TitleBuffer[512];
	HWND ConsoleWindow;
	RECT WindowRect;
	FILE* fp;

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
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);//���������[�N���o->Debug���̂ݗL���ɂȂ�܂�
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
	DxLib::SetCreateDrawValidGraphMultiSample(8, 8);
	auto vrHandler = std::make_unique<OpenvrForDXLib>(0.1f, 15000.0f);
	if (!vrHandler || !vrHandler->vrCheck()) {
		MessageBox(NULL, TEXT("�x��"), TEXT("openVR���������ł��܂���ł���"), MB_OK);
		DxLib::DxLib_End();
		return 0;
	}

	stage = vrLoadModel(".\\res\\mmd_batokin_island\\batokin_island5.x");
	DxLib::MV1SetScale(stage, VGet(3.0f, 3.0f, 3.0f));
	DxLib::MV1SetPosition(stage, VGet(0.0f, 0.0f, 0.0f));

	printf("S�L�[�Ōv���J�n");
	while (CheckHitKey(KEY_INPUT_S) == 0 && DxLib::ProcessMessage() == 0) {}
	system("cls");//��ʂ��N���A
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), COORD{ 0,0 });

	float sx = 0.0f;
	float sy = 50.0f;
	float sz = 100.0f;

	// ����ʂ�\��ʂɔ��f, ���b�Z�[�W����, ��ʃN���A, �L�[�̍X�V)
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