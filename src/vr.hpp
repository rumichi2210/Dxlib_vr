#pragma once
#include "DxLib.h"
#include <D3D11.h>
#include "openvr.h"
#include "Matrices.h"
#include "pathtools.h"
#pragma comment(lib,"openvr_api.lib")
#pragma warning(suppress : 4996)//Matrices.h��pathtools.h��visual studio�񐄏��֐����g�p���Ă��邽��

class VR_MATRIX {
	MATRIX value;
private:
	MATRIX MSub(MATRIX In1, MATRIX In2)
	{
		MATRIX Result =
		{
			{
				{ In1.m[0][0] - In2.m[0][0], In1.m[0][1] - In2.m[0][1], In1.m[0][2] - In2.m[0][2], In1.m[0][3] - In2.m[0][3] },
				{ In1.m[1][0] - In2.m[1][0], In1.m[1][1] - In2.m[1][1], In1.m[1][2] - In2.m[1][2], In1.m[1][3] - In2.m[1][3] },
				{ In1.m[2][0] - In2.m[2][0], In1.m[2][1] - In2.m[2][1], In1.m[2][2] - In2.m[2][2], In1.m[2][3] - In2.m[2][3] },
				{ In1.m[3][0] - In2.m[3][0], In1.m[3][1] - In2.m[3][1], In1.m[3][2] - In2.m[3][2], In1.m[3][3] - In2.m[3][3] }
			}
		};
		return Result;
	}
public:
	VR_MATRIX() noexcept : value(DxLib::MGetIdent()) {}
	VR_MATRIX(MATRIX value) { this->value = value; }
	//���Z
	VR_MATRIX operator+(VR_MATRIX obj)  const noexcept { return VR_MATRIX(DxLib::MAdd(this->value, obj.value)); }
	VR_MATRIX operator+=(VR_MATRIX obj) noexcept {
		this->value = DxLib::MAdd(this->value, obj.value);
		return this->value;
	}
	VR_MATRIX operator-(VR_MATRIX obj) { return VR_MATRIX(MSub(this->value, obj.value)); }
	VR_MATRIX operator-=(VR_MATRIX obj) noexcept {
		this->value = MSub(this->value, obj.value);
		return this->value;
	}
	VR_MATRIX operator*(VR_MATRIX obj) { return VR_MATRIX(DxLib::MMult(this->value, obj.value)); }
	VR_MATRIX operator*=(VR_MATRIX obj) noexcept {
		this->value = DxLib::MMult(this->value, obj.value);
		return this->value;
	}
	VR_MATRIX Scale(float p1) const noexcept { return VR_MATRIX(DxLib::MScale(this->value, p1)); }
	VR_MATRIX Inverse() const noexcept { return VR_MATRIX(DxLib::MInverse(this->value)); }
	MATRIX Get() const noexcept { return this->value; }
};

class OpenvrForDXLib {
private:
	/// VR�A�v���P�[�V�����ɕK���K�v�ȃf�[�^
	vr::IVRSystem* m_pHMD = NULL;
	vr::EVRInitError error = vr::VRInitError_None;//openVR�̃G���[�i�[�p
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];//openVR����擾���鐶�f�[�^�̊i�[(�E����W�n)

	/// �����Ɏg�p����VR�f�[�^
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	/// HMD�ɕ`�悷��ۂ̍œK��ʃT�C�Y
	uint32_t hmdWidth = 0;
	uint32_t hmdHeight = 0;

	/// �J�����ݒ�p
	Matrix4 m_mat4eyePosLeft = Matrix4();
	Matrix4 m_mat4eyePosRight = Matrix4();
	Matrix4 m_mat4HMDPose = Matrix4();

	float m_fNearClip;
	float m_fFarClip;



	/// IVRInput�p�̃f�[�^///
	// ���s�t�@�C������̑��΃p�X���ݒ肳��Ă��邱�Ƃ��m�F���Ă��������B
	std::string manifestPath = "../../vr_binding/actions.json";
	vr::VRActionSetHandle_t m_actionSet = vr::k_ulInvalidActionSetHandle;
	std::string actionSetPath = "/actions/main";

	/// [Tips]
	/// IVRInput����ʒu�Ȃǂ̃f�[�^(pose)���擾����̂͐������܂���B
	/// IVRInput�̓R���g���[���[�̓���(�{�^����g���b�N�p�b�h)�⊴�o�f�o�C�X���g�p����ꍇ�Ɍ����Ă��܂��B(�L�[�o�C���h�@�\�̂���)
	/// ��ʓI�ȃg���b�J�[(�t���g���b�L���O�f�o�C�X�Ȃǂ̃{�^�����͂������Ȃ��f�o�C�X)�ł�m_rmat4DevicePose�̃f�[�^�𒼐ڎg�p���邱�Ƃ��������߂��܂��B

	vr::VRActionHandle_t m_actionSelect = vr::k_ulInvalidActionHandle;
	std::string actionSelectPath = "/actions/main/in/Select";

	vr::VRActionHandle_t m_actionMove = vr::k_ulInvalidActionHandle;
	std::string actionMovePath = "/actions/main/in/Move";

	vr::VRActionHandle_t m_actionCancel = vr::k_ulInvalidActionHandle;
	std::string actionCancelPath = "/actions/main/in/Cancel";

	vr::VRActionHandle_t m_actionDecision = vr::k_ulInvalidActionHandle;
	std::string actionDecisionPath = "/actions/main/in/Decision";

	vr::VRActionHandle_t m_actionControllerLeft = vr::k_ulInvalidActionHandle;
	std::string actionControllerLeftPath = "/actions/main/in/Controller_Left";
	vr::InputPoseActionData_t controllerLeftPoseData;

	vr::VRActionHandle_t m_actionControllerRight = vr::k_ulInvalidActionHandle;
	std::string actionControllerRightPath = "/actions/main/in/Controller_Right";
	vr::InputPoseActionData_t controllerRightPoseData;

	vr::VRInputValueHandle_t m_inputHandLeftPath = vr::k_ulInvalidInputValueHandle;
	std::string inputHandLeftPath = "/user/hand/left";

	vr::VRInputValueHandle_t m_inputHandRightPath = vr::k_ulInvalidInputValueHandle;
	std::string inputHandRightPath = "/user/hand/right";


	inline bool fileExists(const std::string& fileName) {
		struct stat buff;
		return (stat(fileName.c_str(), &buff) == 0);
	}


	// nEye����Ƃ���HMDMatrixPoseEye���擾���܂��B
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);

	// VR�R���|�W�^�[�̏�����
	bool BInitCompositor();

	//HMD�p�̃J�����̏��������܂�
	void SetupCameras();

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	void UpdateHMDMatrixPose();

	// �g���b�L���O�t���[������͂��A���̈ʒu/��]/�C�x���g��\�����܂��B
	void ParseTrackingFrame();

	//�ڑ��@��̏���CUI�Ɉꊇ�\��
	void DeviceInfoBatchDisplay();

	//vr::VRActionHandle_t�Œ�`�������̂����g�p�ł��܂���
	void GetActionHandle(std::string actionName,vr::VRActionHandle_t* pHandle);
public:
	OpenvrForDXLib(float nearClip, float farClip);
	~OpenvrForDXLib();

	// nEye����Ƃ���MatrixProjectionEye���擾���܂��B
	MATRIX GetProjectionMatrix(vr::EVREye eye);
	MATRIX GetViewMatrix(vr::EVREye eye);
	int GetHMDWidth() { return hmdWidth; }
	int GetHMDHeight() { return hmdHeight; }

	//HMD�ŕ`�悷��
	void PutHMD(ID3D11Texture2D* texte, vr::EVREye eye);

	//�f�o�C�X�̏�Ԃ��ꊇ�Ŏ�荞��
	void UpdateState();

	//OpenVR������ɓ��삵�Ă��邩���擾����
	bool vrCheck() { return (error == vr::VRInitError_None) ? true : false; }

};





