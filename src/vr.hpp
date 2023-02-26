#pragma once
#include "DxLib.h"
#include <D3D11.h>
#include "openvr.h"
#include "Matrices.h"
#include "pathtools.h"
#pragma comment(lib,"openvr_api.lib")
#pragma warning(suppress : 4996)//Matrices.h��pathtools.h��visual studio�񐄏��֐����g�p���Ă��邽��


namespace DXLIB_VR {

}

class OpenvrForDXLib {
private:

	// ��{�I�Ȃ���
	vr::IVRSystem* m_pHMD = NULL;
	vr::EVRInitError error = vr::VRInitError_None;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	uint32_t hmdWidth;
	uint32_t hmdHeight;
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;
	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;
	float m_fNearClip = 0.1f;
	float m_fFarClip = 15000.0f;

	// For getting the steam application key
	char applicationKey[vr::k_unMaxApplicationKeyLength];

	// �V����IVRInput�̂��߂̃n���h��
	vr::VRActionSetHandle_t m_actionSet = vr::k_ulInvalidActionSetHandle;
	const char* actionSetPath = "/actions/main";

	vr::VRActionHandle_t m_actionSelect = vr::k_ulInvalidActionHandle;
	const char* actionSelectPath = "/actions/main/in/Select";

	vr::VRActionHandle_t m_actionMove = vr::k_ulInvalidActionHandle;
	const char* actionMovePath = "/actions/main/in/Move";

	vr::VRActionHandle_t m_actionCancel = vr::k_ulInvalidActionHandle;
	const char* actionCancelPath = "/actions/main/in/Cancel";

	vr::VRActionHandle_t m_actionDecision = vr::k_ulInvalidActionHandle;
	const char* actionDecisionPath = "/actions/main/in/Decision";

	vr::VRActionHandle_t m_actionControllerLeft = vr::k_ulInvalidActionHandle;
	const char* actionControllerLeftPath = "/actions/main/in/Controller_Left";
	vr::InputPoseActionData_t controllerLeftPoseData;

	vr::VRActionHandle_t m_actionControllerRight = vr::k_ulInvalidActionHandle;
	const char* actionControllerRightPath = "/actions/main/in/Controller_Right";
	vr::InputPoseActionData_t controllerRightPoseData;

	vr::VRInputValueHandle_t m_inputHandLeftPath = vr::k_ulInvalidInputValueHandle;
	const char* inputHandLeftPath = "/user/hand/left";

	vr::VRInputValueHandle_t m_inputHandRightPath = vr::k_ulInvalidInputValueHandle;
	const char* inputHandRightPath = "/user/hand/right";

	inline bool fileExists(const std::string& fileName) {
		struct stat buff;
		return (stat(fileName.c_str(), &buff) == 0);
	}

	//HMD�̉�ʃT�C�Y(�Ж�)���擾
	void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight) { m_pHMD->GetRecommendedRenderTargetSize(pnWidth, pnHeight); }


	// Purpose: nEye����Ƃ���Matrix Projection Eye���擾���܂��B
	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);


	// Purpose: nEye����Ƃ���HMDMatrixPoseEye���擾���܂��B
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);

	// VR�R���|�W�^�[�̏�����
	bool BInitCompositor();

	//HMD�p�̃J�����̏��������܂�
	void SetupCameras();

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	void UpdateHMDMatrixPose();

	//�ڑ��@��̏���CUI�Ɉꊇ�\��
	void DeviceInformationBatchDisplay();
public:
	OpenvrForDXLib();
	~OpenvrForDXLib();





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


	//vr::VRActionHandle_t�Œ�`�������̂����g�p�ł��܂���
	void GetActionHandleCheck(const char* pchActionName, vr::VRActionHandle_t* pHandle);

	MATRIX GetProjectionMatrix(vr::EVREye eye);
	MATRIX GetViewMatrix(vr::EVREye eye);
	//MATRIX GetProjectionMatrix(vr::Hmd_Eye nEye);
	int GetHMDWidth() { return hmdWidth; }
	int GetHMDHeight() { return hmdHeight; }
	//HMD�ŕ`�悷��
	void PutHMD(ID3D11Texture2D* texte, vr::EVREye eye);

	//�f�o�C�X�̏�Ԃ��ꊇ�Ŏ�荞��
	void UdateState();

};





