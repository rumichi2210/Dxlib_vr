#pragma once
#include "DxLib.h"
#include <D3D11.h>
#include "openvr.h"
#include "Matrices.h"
#include "pathtools.h"
#pragma comment(lib,"openvr_api.lib")
#pragma warning(suppress : 4996)//Matrices.h��pathtools.h��visual studio�񐄏��֐����g�p���Ă��邽��


namespace DXLIB_VR {
	bool Init();
	void Fin();
	void putTex(ID3D11Texture2D* texte, vr::EVREye eye);
	MATRIX GetProjectiontMat(vr::EVREye eye);
	void updateVRState();
	void render();
	int GetHMDWidth();
	int GetHMDHeight();

	MATRIX GetContolloer();
	VECTOR GetLeftContolloer();
	MATRIX GetViewMat(vr::EVREye eye);

	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose);
	void UpdateHMDMatrixPose();
	void SetupCameras();
	void Get_RecommendedRenderTargetSize(vr::IVRSystem* HMD, uint32_t* pnWidth, uint32_t* pnHeight);

	void PrintDevices();
	bool RunProcedure(bool bWaitForEvents, int filterIndex = -1);
}

class OpenvrForDXLib {
private:

	// ��{�I�Ȃ���
	vr::IVRSystem* m_pHMD = NULL;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	uint32_t hmdWidth;
	uint32_t hmdHeight;

	// For getting the steam application key
	char applicationKey[vr::k_unMaxApplicationKeyLength];

	// �V����IVRInput�̂��߂̃n���h��
	vr::VRActionSetHandle_t m_actionSet = vr::k_ulInvalidActionSetHandle;
	const char* actionSetPath = "/actions";

	vr::VRActionHandle_t m_actionSelect = vr::k_ulInvalidActionHandle;
	const char* actionSelectPath = "/actions/in/Select";

	vr::VRActionHandle_t m_actionMove = vr::k_ulInvalidActionHandle;
	const char* actionMovePath = "/actions/in/Move";

	vr::VRActionHandle_t m_actionCancel = vr::k_ulInvalidActionHandle;
	const char* actionCancelPath = "/actions/in/Cancel";

	vr::VRActionHandle_t m_actionDecision = vr::k_ulInvalidActionHandle;
	const char* actionDecisionPath = "/actions/in/Decision";

	vr::VRActionHandle_t m_actionControllerLeft = vr::k_ulInvalidActionHandle;
	const char* actionControllerLeftPath = "/actions/in/Controller_Left";

	vr::VRActionHandle_t m_actionControllerRight = vr::k_ulInvalidActionHandle;
	const char* actionControllerRightPath = "/actions/in/Controller_Right";

	vr::VRInputValueHandle_t m_inputHandLeftPath = vr::k_ulInvalidInputValueHandle;
	const char* inputHandLeftPath = "/user/hand/left";

	vr::VRInputValueHandle_t m_inputHandRightPath = vr::k_ulInvalidInputValueHandle;
	const char* inputHandRightPath = "/user/hand/right";

	inline bool fileExists(const std::string& fileName) {
		struct stat buff;
		return (stat(fileName.c_str(), &buff) == 0);
	}


public:
	OpenvrForDXLib();
	~OpenvrForDXLib();

	// VR�R���|�W�^�[�̏�����
	bool BInitCompositor();

	// openvr �̃C�x���g�����b�X�����Aprocess �� parse �̃��[�`�����Ăяo�����C�����[�v�A���� false �Ȃ�΃T�[�r�X�͏I�����Ă���
	bool RunProcedure(bool bWaitForEvents, int filterIndex);

	// VR�C�x���g���������A�����N���������ɂ��Ă̈�ʓI�ȏ���\�����܂��B
	bool ProcessVREvent(const vr::VREvent_t& event, int filterIndex);

	// �g���b�L���O�t���[������͂��A���̈ʒu/��]/�C�x���g��\�����܂��B
	// filterIndex ��-1 �ȊO���w�肷��ƁA����̃f�o�C�X�̃f�[�^�݂̂�\������B
	void ParseTrackingFrame(int filterIndex);

	// �����v�Z
	vr::HmdVector3_t GetControllerPositionDelta();
	vr::HmdVector3_t GetLeftControllerPosition();
	vr::HmdVector3_t GetRightControllerPosition();

	// �f�o�C�X�̏����������
	void PrintDevices();
};





