#include "vr.hpp"

#include <iostream>
#define VAR_NAME(var) printf_s( "\n--------"#var "--------\n" )

Matrix4 m_mat4HMDPose;
Matrix4 m_mat4eyePosLeft;
Matrix4 m_mat4eyePosRight;
Matrix4 m_mat4ProjectionCenter;
Matrix4 m_mat4ProjectionLeft;
Matrix4 m_mat4ProjectionRight;
float m_fNearClip = 0.1f;
float m_fFarClip = 15000.0f;
//vr::IVRSystem* m_pHMD = nullptr;

uint32_t hmdWidth;
uint32_t hmdHeight;
vr::Texture_t eyeTexLeft;
vr::Texture_t eyeTexRight;
vr::IVRRenderModels* m_pRenderModels;
//vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
//Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];
int m_iTrackedControllerCount;
int m_iTrackedControllerCount_Last;
int m_iValidPoseCount;
int m_iValidPoseCount_Last;
unsigned int m_uiControllerVertcount;
std::string m_strPoseClasses;                            // ���̃t���[���̃|�[�Y�������Ƃ�
char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // �e�f�o�C�X�ɂ��āA���̃N���X��\������



//--New VR system--//
char buf[1024];
// Basic stuff
vr::IVRSystem* m_pHMD = nullptr;
vr::EVRInitError error = vr::VRInitError_None;
vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

// Steam�A�v���P�[�V�����L�[�̎擾�ɂ���
char applicationKey[vr::k_unMaxApplicationKeyLength];

// �V����IVRInput�̂��߂̃n���h��
vr::VRActionSetHandle_t m_actionSetDemo = vr::k_ulInvalidActionSetHandle;
const char* actionSetDemoPath = "/actions/demo";

vr::VRActionHandle_t m_actionAnalogInput = vr::k_ulInvalidActionHandle;
const char* actionDemoAnalogInputPath = "/actions/demo/in/AnalogInput";

vr::VRActionHandle_t m_actionHideCubes = vr::k_ulInvalidActionHandle;
const char* actionDemoHideCubesPath = "/actions/demo/in/HideCubes";

vr::VRActionHandle_t m_actionClick = vr::k_ulInvalidActionHandle;
const char* actionDemoClickPath = "/actions/demo/in/ClickAction";

vr::VRActionHandle_t m_actionTouch = vr::k_ulInvalidActionHandle;
const char* actionDemoTouchPath = "/actions/demo/in/TouchAction";

vr::VRActionHandle_t m_actionDemoHandLeft = vr::k_ulInvalidActionHandle;
const char* actionDemoHandLeftPath = "/actions/demo/in/Hand_Left";

vr::VRActionHandle_t m_actionDemoHandRight = vr::k_ulInvalidActionHandle;
const char* actionDemoHandRightPath = "/actions/demo/in/Hand_Right";

vr::VRInputValueHandle_t m_inputHandLeftPath = vr::k_ulInvalidInputValueHandle;
const char* inputHandLeftPath = "/user/hand/left";

vr::VRInputValueHandle_t m_inputHandRightPath = vr::k_ulInvalidInputValueHandle;
const char* inputHandRightPath = "/user/hand/right";

//struct ControllerInfo_t
//{
//	vr::VRInputValueHandle_t m_source = vr::k_ulInvalidInputValueHandle;
//	vr::VRActionHandle_t m_actionPose = vr::k_ulInvalidActionHandle;
//	vr::VRActionHandle_t m_actionHaptic = vr::k_ulInvalidActionHandle;
//	Matrix4 m_rmat4Pose;
//	std::string m_sRenderModelName;
//	bool m_bShowController;
//};
//
//enum EHand
//{
//	Left = 0,
//	Right = 1,
//};
//ControllerInfo_t m_Hand[2];
//
//vr::HmdVector3_t m_vecLeftController = {};
//vr::HmdVector3_t m_vecRightController = {};

namespace DXLIB_VR {

	inline bool fileExists(const std::string& fileName) {
		struct stat buff;
		return (stat(fileName.c_str(), &buff) == 0);
	}

	//-----------------------------------------------------------------------------
	// Purpose: �R���|�W�^�[�����������܂��B�R���|�W�^�[�̏������ɐ��������ꍇ�� true ���A�����łȂ��ꍇ�� false ��Ԃ��܂��B
	//-----------------------------------------------------------------------------
	bool BInitCompositor()
	{
		vr::EVRInitError peError = vr::VRInitError_None;

		if (!vr::VRCompositor())
		{
			printf("Compositor initialization failed. See log file for details\n");
			return false;
		}

		return true;
	}

	bool Init() {
		m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
		if (error != vr::VRInitError_None) {
			m_pHMD = NULL;
			sprintf_s(buf, sizeof(buf), "VR�����^�C���̊J�n���ł��܂���ł���: %s", vr::VR_GetVRInitErrorAsEnglishDescription(error));
			printf_s(buf);
			return false;
		}

		// VR Compositor ���L���ł��邱�Ƃ��m�F���A�����łȂ��ꍇ�̓|�[�Y���擾����ƃN���b�V�����܂��B
		if (!BInitCompositor()) {
			sprintf_s(buf, sizeof(buf), "VR Compositor�̏������Ɏ��s���܂����B");
			printf_s(buf);
			return false;
		}
		else {
			sprintf_s(buf, sizeof(buf), "VR Compositor�̏������ɐ������܂����B\n");
			printf_s(buf);
		}

		// �}�j�t�F�X�g�t�@�C���̏���
		// ���s�t�@�C������̑��΃p�X���ݒ肳��Ă��邱�Ƃ��m�F���Ă��������B
		const char* manifestPath = "../../vr_binding/actions.json";
		std::string manifestFileName = Path_MakeAbsolute(manifestPath, Path_StripFilename(Path_GetExecutablePath()));

		// �t�@�C�������݂��邩�ǂ������m�F���Ă��瑱�s����
		if (!fileExists(manifestFileName)) {
			sprintf_s(buf, sizeof(buf), "\n�v���I�ȃG���[�ł��B�}�j�t�F�X�g�t�@�C�������݂��܂���B���[�h�Ɏ��s�����t�@�C��:\n%s\n\n", manifestFileName.c_str());
			printf_s(buf);
			return false;
		}

		vr::EVRInputError inputError = vr::VRInput()->SetActionManifestPath(manifestFileName.c_str());
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�}�j�t�F�X�g�̃p�X���ݒ�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "�}�j�t�F�X�g�p�X�̎g�p�ɐ���: %s\n", manifestFileName.c_str());
			printf_s(buf);
		}

		// �V����IVRInput�̂��߂̃n���h��
		inputError = vr::VRInput()->GetActionSetHandle(actionSetDemoPath, &m_actionSetDemo);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�@�A�N�V�����Z�b�g�n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s �A�N�V�����Z�b�g�n���h���̎擾�ɐ������܂���: %d\n", actionSetDemoPath, (int)m_actionSetDemo);
			printf_s(buf);
		}

		// ���R���g���[���|�[�Y�p�n���h��
		inputError = vr::VRInput()->GetActionHandle(actionDemoHandLeftPath, &m_actionDemoHandLeft);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoHandLeftPath, (int)m_actionDemoHandLeft);
			printf_s(buf);
		}

		// �E�R���g���[���|�[�Y�p�n���h��
		inputError = vr::VRInput()->GetActionHandle(actionDemoHandRightPath, &m_actionDemoHandRight);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoHandRightPath, (int)m_actionDemoHandRight);
			printf_s(buf);
		}

		// �A�i���O�g���b�N�p�b�h����p�n���h��
		inputError = vr::VRInput()->GetActionHandle(actionDemoAnalogInputPath, &m_actionAnalogInput);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoAnalogInputPath, (int)m_actionAnalogInput);
			printf_s(buf);
		}

		// �������L���[�u�A�N�V�����̃n���h��
		inputError = vr::VRInput()->GetActionHandle(actionDemoHideCubesPath, &m_actionHideCubes);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoHideCubesPath, (int)m_actionHideCubes);
			printf_s(buf);
		}

		// �^�b�`�A�N�V�����p�n���h��
		inputError = vr::VRInput()->GetActionHandle(actionDemoTouchPath, &m_actionTouch);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "�n���h�� %s �̎擾�ɐ������܂����B: %d\n", actionDemoTouchPath, (int)m_actionTouch);
			printf_s(buf);
		}

		// �N���b�N�n���h��
		inputError = vr::VRInput()->GetActionHandle(actionDemoClickPath, &m_actionClick);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "���̓n���h�� %s �̎擾�ɐ������܂����B: %d\n", actionDemoClickPath, (int)m_actionClick);
			printf_s(buf);
		}

		// �R���g���[���|�[�Y�\�[�X�p�n���h�� - ���ݖ��g�p
		inputError = vr::VRInput()->GetInputSourceHandle(inputHandLeftPath, &m_inputHandLeftPath);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[���̓n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "���̓n���h�� %s �̎擾�ɐ������܂����B: %d\n", inputHandLeftPath, (int)m_inputHandLeftPath);
			printf_s(buf);
		}

		inputError = vr::VRInput()->GetInputSourceHandle(inputHandRightPath, &m_inputHandRightPath);
		if (inputError != vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "�G���[���̓n���h�����擾�ł��܂���: %d\n", inputError);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "���̓n���h�� %s �̎擾�ɐ������܂����B: %d\n", inputHandRightPath, (int)m_inputHandRightPath);
			printf_s(buf);
		}

		PrintDevices();//�f�o�C�X�̏�Ԃ��擾

		Get_RecommendedRenderTargetSize(m_pHMD, &hmdWidth, &hmdHeight);
		printf("HMDWidth=%d\n", DXLIB_VR::GetHMDWidth());
		printf("HMDHeight=%d\n", DXLIB_VR::GetHMDHeight());
		SetupCameras();
		return true;
	}

	void Fin() { vr::VR_Shutdown(); }

	//HMD�̉�ʃT�C�Y(�Ж�)���擾
	void Get_RecommendedRenderTargetSize(vr::IVRSystem* HMD, uint32_t* pnWidth, uint32_t* pnHeight) { HMD->GetRecommendedRenderTargetSize(pnWidth, pnHeight); }


	//�`�悷��摜�𑗂�
	void putTex(ID3D11Texture2D* texte, vr::EVREye eye) {
		if (eye == vr::EVREye::Eye_Left) {
			eyeTexLeft = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
		}
		if (eye == vr::EVREye::Eye_Right) {
			eyeTexRight = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
		}
	}

	void MATRIX4_Print(Matrix4 val) {
		/*const float* pos = val.get();
		for (int i = 0; i < 16; i++) {
			printf("pos[%d]=%f\n", i, pos[i]);
		}*/
	}

	void MATRIX_Print(MATRIX val) {
		//printf("val[0][n]->%f,%f,%f,%f\n", val.m[0][0], val.m[0][1], val.m[0][2], val.m[0][3]);
		//printf("val[1][n]->%f,%f,%f,%f\n", val.m[1][0], val.m[1][1], val.m[1][2], val.m[1][3]);
		//printf("val[2][n]->%f,%f,%f,%f\n", val.m[2][0], val.m[2][1], val.m[2][2], val.m[2][3]);
		//printf("val[3][n]->%f,%f,%f,%f\n", val.m[3][0], val.m[3][1], val.m[3][2], val.m[3][3]);
	}


	MATRIX GetProjectiontMat(vr::EVREye eye) {
		float fLeft, fRight, fTop, fBottom;

		m_pHMD->GetProjectionRaw(eye, &fLeft, &fRight, &fTop, &fBottom);

		float idx = 1.0f / (fRight - fLeft);
		float idy = 1.0f / (fBottom - fTop);
		float idz = 1.0f / (m_fFarClip - m_fNearClip);
		float sx = fRight + fLeft;
		float sy = fBottom + fTop;

		MATRIX m_pos{};
		m_pos.m[0][0] = 2 * idx;	m_pos.m[0][1] = 0;			m_pos.m[0][2] = 0;									m_pos.m[0][3] = 0;
		m_pos.m[1][0] = 0;			m_pos.m[1][1] = 2 * idy;	m_pos.m[1][2] = 0;									m_pos.m[1][3] = 0;
		m_pos.m[2][0] = sx * idx;	m_pos.m[2][1] = sy * idy;	m_pos.m[2][2] = -m_fFarClip * idz;					m_pos.m[2][3] = -1.0f;
		m_pos.m[3][0] = 0;			m_pos.m[3][1] = 0;			m_pos.m[3][2] = -m_fFarClip * m_fNearClip * idz;	m_pos.m[3][3] = 0;
		return m_pos;
	}


	void ProjectionRawPrint(vr::EVREye eye) {
		eye == vr::EVREye::Eye_Left ? printf("-----Eye_Left-----") : printf("-----Eye_Right-----");
		float pfLeft;
		float pfRight;
		float pfTop;
		float pfBottom;
		m_pHMD->GetProjectionRaw(eye, &pfLeft, &pfRight, &pfTop, &pfBottom);
		/*VAR_NAME(pfLeft);
		printf("%f", pfLeft);
		VAR_NAME(pfRight);
		printf("%f", pfRight);
		VAR_NAME(pfTop);
		printf("%f", pfTop);
		VAR_NAME(pfBottom);
		printf("%f", pfBottom);*/
	}

	MATRIX GetViewMat(vr::EVREye eye) {
		Matrix4 matMVP;
		//for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
		//{
		//	const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevice];
		//	if (!pose.bPoseIsValid)
		//		continue;
		//	const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
		//	matMVP = GetCurrentViewProjectionMatrix(eye);// *matDeviceToTracking;
		//}
		matMVP = GetCurrentViewProjectionMatrix(eye);
		const float* pos = matMVP.get();
		MATRIX m_pos;
		m_pos.m[0][0] = pos[0];		m_pos.m[0][1] = pos[1];		m_pos.m[0][2] = pos[2];		m_pos.m[0][3] = pos[3];
		m_pos.m[1][0] = pos[4];		m_pos.m[1][1] = pos[5];		m_pos.m[1][2] = pos[6];		m_pos.m[1][3] = pos[7];
		m_pos.m[2][0] = pos[8];		m_pos.m[2][1] = pos[9];		m_pos.m[2][2] = pos[10];	m_pos.m[2][3] = pos[11];
		m_pos.m[3][0] = pos[12];	m_pos.m[3][1] = pos[13];	m_pos.m[3][2] = pos[14];	m_pos.m[3][3] = pos[15];
		return m_pos;
	}

	//�f�o�C�X�̏�Ԃ��ꊇ�Ŏ�荞��
	void updateVRState() {
		UpdateHMDMatrixPose();
	}

	//HMD�ɉf��摜���X�V
	void render() {
		vr::VRCompositor()->Submit(vr::EVREye::Eye_Left, &eyeTexLeft);
		vr::VRCompositor()->Submit(vr::EVREye::Eye_Right, &eyeTexRight);
	}

	int GetHMDWidth() { return hmdWidth; }
	int GetHMDHeight() { return hmdHeight; }

	MATRIX GetDXProjectionMatrix(vr::Hmd_Eye nEye) {
		vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

		MATRIX m_pos;
		m_pos.m[0][0] = mat.m[0][0]; m_pos.m[0][1] = mat.m[0][1]; m_pos.m[0][2] = mat.m[0][2]; m_pos.m[0][3] = mat.m[0][3];
		m_pos.m[1][0] = mat.m[1][0]; m_pos.m[1][1] = mat.m[1][1]; m_pos.m[1][2] = mat.m[1][2]; m_pos.m[1][3] = mat.m[1][3];
		m_pos.m[2][0] = mat.m[2][0]; m_pos.m[2][1] = mat.m[2][1]; m_pos.m[2][2] = mat.m[2][2]; m_pos.m[2][3] = mat.m[2][3];
		m_pos.m[3][0] = mat.m[3][0]; m_pos.m[3][1] = mat.m[3][1]; m_pos.m[3][2] = mat.m[3][2]; m_pos.m[3][3] = mat.m[3][3];

		MTranspose(m_pos);

		return m_pos;
	}

	MATRIX GetContolloer() {
		const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand)];
		/*if (!pose.bPoseIsValid)
			continue;*/
		const Matrix4& matDeviceToTracking = m_rmat4DevicePose[m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand)];
		Matrix4 matMVP = m_rmat4DevicePose[m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand)];
		matMVP.invert();
		const float* pos = matDeviceToTracking.get();
		MATRIX m_pos{};
		m_pos.m[0][0] = pos[0];		m_pos.m[0][1] = pos[1];		m_pos.m[0][2] = pos[2];		m_pos.m[0][3] = pos[3];
		m_pos.m[1][0] = pos[4];		m_pos.m[1][1] = pos[5];		m_pos.m[1][2] = pos[6];		m_pos.m[1][3] = pos[7];
		m_pos.m[2][0] = pos[8];		m_pos.m[2][1] = pos[9];		m_pos.m[2][2] = pos[10];	m_pos.m[2][3] = pos[11];
		m_pos.m[3][0] = pos[12];	m_pos.m[3][1] = pos[13];	m_pos.m[3][2] = pos[14];	m_pos.m[3][3] = pos[15];
		return m_pos;
	}

	VECTOR GetLeftContolloer() {
		Matrix4 matMVP = m_rmat4DevicePose[m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand)];
		matMVP.invert();
		const float* pos = matMVP.get();
		MATRIX m_pos{};
		m_pos.m[0][0] = pos[0];		m_pos.m[0][1] = pos[1];		m_pos.m[0][2] = pos[2];		m_pos.m[0][3] = pos[3];
		m_pos.m[1][0] = pos[4];		m_pos.m[1][1] = pos[5];		m_pos.m[1][2] = pos[6];		m_pos.m[1][3] = pos[7];
		m_pos.m[2][0] = pos[8];		m_pos.m[2][1] = pos[9];		m_pos.m[2][2] = pos[10];	m_pos.m[2][3] = pos[11];
		m_pos.m[3][0] = pos[12];	m_pos.m[3][1] = pos[13];	m_pos.m[3][1] = pos[14];	m_pos.m[3][3] = pos[15];
		return VGet(m_pos.m[3][0], m_pos.m[3][1], -m_pos.m[3][1]);
	}


	//-----------------------------------------------------------------------------
	// Purpose: Gets a Matrix Projection Eye with respect to nEye.
	//-----------------------------------------------------------------------------
	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
	{
		if (!m_pHMD)
			return Matrix4();

		vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

		return Matrix4(
			mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
			mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.                          
	//-----------------------------------------------------------------------------
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
	{
		if (!m_pHMD)
			return Matrix4();

		vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
		Matrix4 matrixObj(
			matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
			matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
			matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
			matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

		return matrixObj.invert();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Gets a Current View Projection Matrix with respect to nEye,
	//          which may be an Eye_Left or an Eye_Right.
	//-----------------------------------------------------------------------------
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
	{
		Matrix4 matMVP;
		if (nEye == vr::Eye_Left)
		{
			matMVP = m_mat4eyePosLeft * m_mat4HMDPose;
		}
		else if (nEye == vr::Eye_Right)
		{
			matMVP = m_mat4eyePosRight * m_mat4HMDPose;
		}

		return matMVP;
	}


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	void UpdateHMDMatrixPose()
	{
		if (!m_pHMD)
			return;

		vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

		m_iValidPoseCount = 0;
		m_strPoseClasses = "";
		for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		{

			if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
			{
				m_iValidPoseCount++;
				m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
				//VAR_NAME(m_rmat4DevicePose[nDevice]);
				MATRIX4_Print(m_rmat4DevicePose[nDevice]);
				if (m_rDevClassChar[nDevice] == 0)
				{
					switch (m_pHMD->GetTrackedDeviceClass(nDevice))
					{
					case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
					case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
					case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
					case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
					case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
					default:                                       m_rDevClassChar[nDevice] = '?'; break;
					}
				}
				m_strPoseClasses += m_rDevClassChar[nDevice];
			}
		}

		if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
			m_mat4HMDPose.invert();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Converts a SteamVR matrix to our local matrix class
	//-----------------------------------------------------------------------------
	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose)
	{
		Matrix4 matrixObj(
			matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
			matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
			matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
			matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
		return matrixObj;
	}


	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	void SetupCameras()
	{
		m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
		m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
		m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
		m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
	}


	Matrix4 GetHMDPose()
	{
		if (!m_pHMD)
			return Matrix4();

		vr::TrackedDevicePose_t tmp;
		m_pHMD->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0.0f, &tmp, 1);

		return ConvertSteamVRMatrixToMatrix4(tmp.mDeviceToAbsoluteTracking);;
	}


	// �ʒu��\���x�N�g�����擾����
	vr::HmdVector3_t GetPosition(vr::HmdMatrix34_t matrix) {
		vr::HmdVector3_t vector;

		vector.v[0] = matrix.m[0][3];
		vector.v[1] = matrix.m[1][3];
		vector.v[2] = matrix.m[2][3];

		return vector;
	}

	// ��]��\���l�����i�N�H�[�^�j�I���j���擾����
	vr::HmdQuaternion_t GetRotation(vr::HmdMatrix34_t matrix) {
		vr::HmdQuaternion_t q;

		q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
		q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
		q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
		q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
		q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
		q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
		q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
		return q;
	}


	void PrintDevices() {

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "\nDevice list:\n---------------------------\n");
		printf_s(buf);

		// �ڑ����ꂽ���ׂẴf�o�C�X�����[�v���A�����Ɋւ��邢�����̏���\������
		for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
		{
			// HMD�����邱�Ƃ������̂��߃X���b�g0��HMD���ڑ�����Ă��Ȃ��ꍇ�́A�c��̃R�[�h���X�L�b�v����
			if (!m_pHMD->IsTrackedDeviceConnected(unDevice))
				continue;

			// �f�o�C�X�̎�ނ�c�����A���̃f�[�^���g���Ďd��������
			vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
			switch (trackedDeviceClass) {
			case vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid:
				// �����ȃN���X�̂��߂ɉ���������

				sprintf_s(buf, sizeof(buf), "�f�o�C�X %d: �N���X[�����n", unDevice);
				printf_s(buf);
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
				// HMD�͂����ōs���A�R���g���[�����̃P�[�X�u���b�N�ɂčs���܂��B

				char buf[1024];
				sprintf_s(buf, sizeof(buf), "�f�o�C�X %d: Class: [HMD]", unDevice);
				printf_s(buf);
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
				// �R���g���[���̂��߂̂��̂������ł��
				sprintf_s(buf, sizeof(buf), "�f�o�C�X %d: �N���X: [�R���g���[���[]", unDevice);
				printf_s(buf);

				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
				// ��ʓI�ȃg���b�J�[�̂��߂̂��̂������ōs��

				sprintf_s(buf, sizeof(buf), "�f�o�C�X %d: �N���X: [��ʓI�ȃg���b�J�[]", unDevice);
				printf_s(buf);
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference:
				/// �g���b�L���O���t�@�����X�̂��߂ɁA�����ŉ���������

				sprintf_s(buf, sizeof(buf), "�f�o�C�X %d: �N���X: [�g���b�L���O���t�@�����X]", unDevice);
				printf_s(buf);
				break;

			case vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect:
				/// �f�B�X�v���C�̃��_�C���N�g�ɕK�v�Ȃ��̂������ōs��

				sprintf_s(buf, sizeof(buf), "�f�o�C�X %d: �N���X: [DisplayRedirect]", unDevice);
				printf_s(buf);
				break;

			}

			// �f�o�C�X�̃��^�f�[�^��\������

			// �V����IVRInput�̂��߁A�񐄏��H
			int32_t role;
			vr::ETrackedPropertyError pError;
			role = vr::VRSystem()->GetInt32TrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32, &pError);
			if (pError == vr::ETrackedPropertyError::TrackedProp_Success) {
				if (role == vr::ETrackedControllerRole::TrackedControllerRole_Invalid) {
					sprintf_s(buf, sizeof(buf), " | �����Ȗ��� (?): %d", role);
					printf_s(buf);
				}
				else {
					sprintf_s(buf, sizeof(buf), " | ����: %d", role);
					printf_s(buf);
				}
			}

			char manufacturer[1024];
			vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, manufacturer, sizeof(manufacturer));

			char modelnumber[1024];
			vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, modelnumber, sizeof(modelnumber));

			char serialnumber[1024];
			vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_SerialNumber_String, serialnumber, sizeof(serialnumber));

			bool canPowerOff = vr::VRSystem()->GetBoolTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_DeviceCanPowerOff_Bool);

			sprintf_s(buf, sizeof(buf), " | �}�j���t�@�N�`�������O: %s | ���f��: %s | �V���A��: %s | �d���I�t���\���ǂ����̗L��: %d\n", manufacturer, modelnumber, serialnumber, canPowerOff);
			printf_s(buf);
		}
		sprintf_s(buf, sizeof(buf), "---------------------------\n�f�o�C�X�ꗗ�̏I��\n\n");
		printf_s(buf);

	}

	//-----------------------------------------------------------------------------
	// Purpose: 1��VR�C�x���g����������
	//-----------------------------------------------------------------------------
	bool ProcessVREvent(const vr::VREvent_t& event, int filterOutIndex = -1)
	{
		// ���[�U���f�o�C�X�t�B���^�C���f�b�N�X���w�肵���ꍇ�A���̃f�o�C�X�̃C�x���g�݂̂�\������B
		if (filterOutIndex != -1)
			if (event.trackedDeviceIndex == filterOutIndex)
				return true;

		// �l�X�ȃC�x���g�̕\������i���S�ȃ��X�g�ł͂���܂���j
		switch (event.eventType)
		{
		case vr::VREvent_TrackedDeviceActivated:
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �f�o�C�X : %d ���ڑ�����܂����B\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		case vr::VREvent_TrackedDeviceDeactivated:
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �f�o�C�X : %d ���؂藣����܂����B\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		case vr::VREvent_TrackedDeviceUpdated:
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �f�o�C�X : %d ���X�V����܂����B\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		case (vr::VREvent_DashboardActivated):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �_�b�V���{�[�h�N��\n");
			printf_s(buf);
		}
		break;

		case (vr::VREvent_DashboardDeactivated):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �_�b�V���{�[�h�̔�A�N�e�B�u��\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_ChaperoneDataHasChanged):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �V���y�����f�[�^���ύX�ɂȂ�܂���\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_ChaperoneSettingsHaveChanged):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �V���y�����̐ݒ肪�ύX�ɂȂ�܂���\n");
			printf_s(buf);
		}
		break;

		case (vr::VREvent_ChaperoneUniverseHasChanged):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �V���y�������j�o�[�X���ς��܂���\n");
			printf_s(buf);

		}
		break;

		case (vr::VREvent_Quit):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) VREvent_Quit\n");
			printf_s(buf);

			return false;
		}
		break;

		case (vr::VREvent_ProcessQuit):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) VREvent_ProcessQuit\n");
			printf_s(buf);

			return false;
		}
		break;

		case (vr::VREvent_QuitAcknowledged):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �I���m�F\n");
			printf_s(buf);

			return false;
		}
		break;

		case (vr::VREvent_TrackedDeviceRoleChanged):
		{

			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �g���b�L���O�f�o�C�X�̖����ύX: %d\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		case (vr::VREvent_TrackedDeviceUserInteractionStarted):
		{
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) �g���b�L���O�f�o�C�X�̃��[�U�[�C���^���N�V�����J�n: %d\n", event.trackedDeviceIndex);
			printf_s(buf);
		}
		break;

		// �O�̃X�C�b�`�`�����N�ɂ܂���������Ă��Ȃ�/�ړ�����Ă��Ȃ��l�X�ȃC�x���g�B
		default: {
			char buf[1024];
			switch (event.eventType) {
			case vr::EVREventType::VREvent_ButtonTouch:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �^�b�`�f�o�C�X: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_ButtonUntouch:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �A���^�b�`�f�o�C�X: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_ButtonPress:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: Press Device: %d\n", event.trackedDeviceIndex);
				printf_s(buf);

				vr::VREvent_Data_t vrevent_data = event.data;
				vr::VREvent_Controller_t vrevent_controller = vrevent_data.controller;
				sprintf_s(buf, sizeof(buf), "(OpenVR) �R���g���[���[�{�^��: %d\n", vrevent_controller.button);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_ButtonUnpress:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: Release Device: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_EnterStandbyMode:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �X�^���o�C���[�h�ֈڍs: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_LeaveStandbyMode:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: Leave StandbyMode: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_StatusUpdate:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �X�e�[�^�X�X�V: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_PropertyChanged:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �v���p�e�B���ύX���ꂽ�f�o�C�X: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;

			case vr::EVREventType::VREvent_SceneApplicationChanged:
			{
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �V�[���A�v���P�[�V������ύX���܂���\n");
				printf_s(buf);

				sprintf_s(buf, sizeof(buf), "(OpenVR) old pid: %d new pid: %d\n", event.data.process.oldPid, event.data.process.pid);
				printf_s(buf);

				// ���݂̃V�[���A�v���P�[�V�������擾����ʂ̕��@�i���ł��Ăяo�����Ƃ��ł��܂��j
				uint32_t pid = vr::VRApplications()->GetCurrentSceneProcessId();
				sprintf_s(buf, sizeof(buf), "(OpenVR) vr::VRApplications()->GetCurrentSceneProcessId() pid: %d\n", pid);
				printf_s(buf);

				// �X�`�[���A�v���L�[��v�����Č�����
				vr::EVRApplicationError eError = vr::VRApplications()->GetApplicationKeyByProcessId(pid, applicationKey, vr::k_unMaxApplicationKeyLength);
				sprintf_s(buf, sizeof(buf), "(OpenVR) vr::VRApplications()->GetApplicationKeyByProcessId() key: %s\n", applicationKey);
				printf_s(buf);

			}
			break;

			case vr::EVREventType::VREvent_SceneFocusChanged:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event:�V�[���t�H�[�J�X��ύX\n");
				printf_s(buf);

				sprintf_s(buf, sizeof(buf), "(OpenVR) old pid: %d new pid: %d\n", event.data.process.oldPid, event.data.process.pid);
				printf_s(buf);
				break;

			case vr::EVREventType::VREvent_TrackedDeviceUserInteractionStarted:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �g���b�L���O���ꂽ�f�o�C�X ���[�U�[�Ƃ̃C���^���N�V���� �J�n���ꂽ�f�o�C�X: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_TrackedDeviceUserInteractionEnded:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �g���b�L���O���ꂽ�f�o�C�X ���[�U�[�C���^���N�V���� �I�������f�o�C�X: %d\n", event.trackedDeviceIndex);
				printf_s(buf);
				break;

			case vr::EVREventType::VREvent_ProcessDisconnected:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �v���Z�X���ؒf���ꂽ\n");
				printf_s(buf);
				break;
			case vr::EVREventType::VREvent_ProcessConnected:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: �v���Z�X���ڑ�����܂���\n");
				printf_s(buf);
				break;

			case vr::VREvent_Compositor_ApplicationNotResponding:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Compositor: �A�v���P�[�V�������������Ȃ�\n");
				printf_s(buf);
				break;
			case vr::VREvent_Compositor_ApplicationResumed:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Compositor: �A�v���P�[�V�����̍ĊJ\n");
				printf_s(buf);
				break;
			case vr::VRInitError_Compositor_FirmwareRequiresUpdate:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Compositor: �t�@�[���E�F�A�̍X�V���K�v\n");
				printf_s(buf);
				break;
			case vr::VRInitError_Compositor_SettingsInterfaceIsNull:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Compositor: �ݒ�C���^�[�t�F�C�X��NULL\n");
				printf_s(buf);
				break;
			case vr::VRInitError_Compositor_MessageOverlaySharedStateInitFailure:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Compositor: ���b�Z�[�W�I�[�o�[���C���L��ԏ��������s\n");
				printf_s(buf);
				break;

			case vr::VREvent_Input_BindingLoadFailed:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: Input: �o�C���f�B���O���[�h���s\n");
				printf_s(buf);
				break;
			case vr::VREvent_Input_BindingLoadSuccessful:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: Input: �o�C���f�B���O���[�h����������\n");
				printf_s(buf);
				break;
			case vr::VREvent_Input_ActionManifestReloaded:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: Input: �A�N�V�����E�}�j�t�F�X�g �����[�f�b�h\n");
				printf_s(buf);
				break;

			case vr::VREvent_Input_HapticVibration:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Event: Input: �G�o�o�C�u���[�V�����@�R���|�[�l���g�n���h��: %d �R���e�i�n���h��: %d �p������: %f �U��: %f ���g��: %f\n", (int)event.data.hapticVibration.componentHandle, (int)event.data.hapticVibration.containerHandle, event.data.hapticVibration.fDurationSeconds, event.data.hapticVibration.fAmplitude, event.data.hapticVibration.fFrequency);
				printf_s(buf);
				break;

			default:
				sprintf_s(buf, sizeof(buf), "(OpenVR) Unmanaged Event: %d Device: %d\n", event.eventType, event.trackedDeviceIndex);
				printf_s(buf);
				break;
			}
		}
			   break;
		}

		return true;
	}

	/*
	* �g���b�L���O�V�X�e������̃f�[�^���܂�Frame�̃p�[�X
	* Open VR Convention (OpenGL�Ɠ���)
	* �E��n
	* +y is up
	* +x is to the right
	* -z is going away from you
	*/
	void ParseTrackingFrame(int filterIndex) {

		char buf[1024];
		vr::EVRInputError inputError;

		sprintf_s(buf, sizeof(buf), "\n");
		printf_s(buf);

		// SteamVR �̃A�N�V�����̏�Ԃ��������� 
		// UpdateActionState �̓t���[�����ƂɌĂяo����A�A�N�V�����̏�Ԏ��̂��X�V����B
		//�A�v���P�[�V�����́A�񋟂��ꂽ VRActiveActionSet_t �\���̂̔z���p���āA 
		// �ǂ̃A�N�V�����Z�b�g���A�N�e�B�u���𐧌䂵�܂��B
		vr::VRActiveActionSet_t actionSet = { 0 };
		actionSet.ulActionSet = m_actionSetDemo;
		inputError = vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
		if (inputError == vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "%s | UpdateActionState(): Ok\n", actionSetDemoPath);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | UpdateActionState(): Error: %d\n", actionSetDemoPath, inputError);
			printf_s(buf);
		}

		// �A�i���O�f�[�^�擾
		vr::InputAnalogActionData_t analogData;
		inputError = vr::VRInput()->GetAnalogActionData(m_actionAnalogInput, &analogData, sizeof(analogData), vr::k_ulInvalidInputValueHandle);
		if (inputError == vr::VRInputError_None)
		{
			sprintf_s(buf, sizeof(buf), "%s | GetAnalogActionData() Ok\n", actionDemoAnalogInputPath);
			printf_s(buf);

			if (analogData.bActive) {
				float m_vAnalogValue0 = analogData.x;
				float m_vAnalogValue1 = analogData.y;
				sprintf_s(buf, sizeof(buf), "%s | x: %f  y:%f\n", actionDemoAnalogInputPath, m_vAnalogValue0, m_vAnalogValue1);
				printf_s(buf);
			}
			else {
				sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoAnalogInputPath);
				printf_s(buf);
			}
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | GetAnalogActionData() Not Ok. Error: %d\n", actionDemoAnalogInputPath, inputError);
			printf_s(buf);
		}


		// Get digital data
		vr::InputDigitalActionData_t digitalData;
		inputError = vr::VRInput()->GetDigitalActionData(m_actionHideCubes, &digitalData, sizeof(digitalData), vr::k_ulInvalidInputValueHandle);
		if (inputError == vr::VRInputError_None)
		{
			sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Ok\n", actionDemoHideCubesPath);
			printf_s(buf);

			if (digitalData.bActive) {
				bool m_vDigitalValue0 = digitalData.bState;
				sprintf_s(buf, sizeof(buf), "%s | State: %d\n", actionDemoHideCubesPath, m_vDigitalValue0);
				printf_s(buf);

				// check from which device the action came
				vr::InputOriginInfo_t originInfo;
				if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(digitalData.activeOrigin, &originInfo, sizeof(originInfo)))
				{
					if (originInfo.devicePath == m_inputHandLeftPath) {
						sprintf_s(buf, sizeof(buf), "Action comes from left hand\n");
						printf_s(buf);
					}
					else if (originInfo.devicePath == m_inputHandRightPath) {
						sprintf_s(buf, sizeof(buf), "Action comes from right hand\n");
						printf_s(buf);
					}
				}

			}
			else {
				sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoHideCubesPath);
				printf_s(buf);
			}
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Not Ok. Error: %d\n", actionDemoHideCubesPath, inputError);
			printf_s(buf);
		}

		// "�^�b�`�A�N�V���� "�̃f�W�^���f�[�^�擾
		vr::InputDigitalActionData_t digitalDataTouch;
		inputError = vr::VRInput()->GetDigitalActionData(m_actionTouch, &digitalDataTouch, sizeof(digitalDataTouch), vr::k_ulInvalidInputValueHandle);
		if (inputError == vr::VRInputError_None)
		{
			sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Ok\n", actionDemoTouchPath);
			printf_s(buf);

			if (digitalDataTouch.bActive) {
				bool m_vDigitalValue0 = digitalDataTouch.bState;
				sprintf_s(buf, sizeof(buf), "%s | State: %d\n", actionDemoTouchPath, m_vDigitalValue0);
				printf_s(buf);

				// check from which device the action came
				vr::InputOriginInfo_t originInfo;
				if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(digitalDataTouch.activeOrigin, &originInfo, sizeof(originInfo)))
				{
					if (originInfo.devicePath == m_inputHandLeftPath) {
						sprintf_s(buf, sizeof(buf), "Action comes from left hand\n");
						printf_s(buf);
					}
					else if (originInfo.devicePath == m_inputHandRightPath) {
						sprintf_s(buf, sizeof(buf), "Action comes from right hand\n");
						printf_s(buf);
					}
				}

			}
			else {
				sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoTouchPath);
				printf_s(buf);
			}
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Not Ok. Error: %d\n", actionDemoTouchPath, inputError);
			printf_s(buf);
		}

		// �N���b�N�A�N�V�����̃f�W�^���f�[�^�擾"
		vr::InputDigitalActionData_t digitalDataClick;
		inputError = vr::VRInput()->GetDigitalActionData(m_actionClick, &digitalDataClick, sizeof(digitalDataClick), vr::k_ulInvalidInputValueHandle);
		if (inputError == vr::VRInputError_None)
		{
			sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Ok\n", actionDemoClickPath);
			printf_s(buf);

			if (digitalDataClick.bActive) {
				bool m_vDigitalValue0 = digitalDataClick.bState;
				sprintf_s(buf, sizeof(buf), "%s | State: %d\n", actionDemoClickPath, m_vDigitalValue0);
				printf_s(buf);

				// check from which device the action came
				vr::InputOriginInfo_t originInfo;
				if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(digitalDataClick.activeOrigin, &originInfo, sizeof(originInfo)))
				{
					if (originInfo.devicePath == m_inputHandLeftPath) {
						sprintf_s(buf, sizeof(buf), "Action comes from left hand\n");
						printf_s(buf);
					}
					else if (originInfo.devicePath == m_inputHandRightPath) {
						sprintf_s(buf, sizeof(buf), "Action comes from right hand\n");
						printf_s(buf);
					}
				}

			}
			else {
				sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoClickPath);
				printf_s(buf);
			}
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Not Ok. Error: %d\n", actionDemoClickPath, inputError);
			printf_s(buf);
		}


		// get pose data
		vr::InputPoseActionData_t poseData;
		inputError = vr::VRInput()->GetPoseActionDataForNextFrame(m_actionDemoHandLeft, vr::TrackingUniverseStanding, &poseData, sizeof(poseData), vr::k_ulInvalidInputValueHandle);
		if (inputError == vr::VRInputError_None) {
			sprintf_s(buf, sizeof(buf), "%s | GetPoseActionData() Ok\n", actionDemoHandLeftPath);
			printf_s(buf);

			if (poseData.bActive) {
				vr::VRInputValueHandle_t activeOrigin = poseData.activeOrigin;
				bool bPoseIsValid = poseData.pose.bPoseIsValid;
				bool bDeviceIsConnected = poseData.pose.bDeviceIsConnected;
				sprintf_s(buf, sizeof(buf), "Origin: %d Validity: %d DeviceIsConnected: %d\n", (int)activeOrigin, bPoseIsValid, bDeviceIsConnected);
				printf_s(buf);


				/* Code below is old ---> */
				vr::HmdVector3_t position;
				vr::HmdQuaternion_t quaternion;

				// get the position and rotation
				position = GetPosition(poseData.pose.mDeviceToAbsoluteTracking);
				quaternion = GetRotation(poseData.pose.mDeviceToAbsoluteTracking);

				// print the tracking data
				//if (printHmdTrackingData) {
				sprintf_s(buf, sizeof(buf), "\n%s Pose\nx: %.2f y: %.2f z: %.2f\n", actionDemoHandLeftPath, position.v[0], position.v[1], position.v[2]);
				printf_s(buf);
				sprintf_s(buf, sizeof(buf), "qw: %.2f qx: %.2f qy: %.2f qz: %.2f\n", quaternion.w, quaternion.x, quaternion.y, quaternion.z);
				printf_s(buf);
				//}
			/* <--- End of old code */


			}
			else {
				sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoHandLeftPath);
				printf_s(buf);
			}
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | GetPoseActionData() Call Not Ok. Error: %d\n", actionDemoHandLeftPath, inputError);
			printf_s(buf);
		}
	}

	/*
	* ���[�v���ăC�x���g�𕷂��A�������͂��� (�Ⴆ�΃��[�U�ɕ\������)
	filterIndex �� -1 �ȊO���w�肷��ƁA���̃f�o�C�X�̏��݂̂�\������B
	��������� true�Aopenvr ���I�������ꍇ�� false ��Ԃ��B
	*/
	bool RunProcedure(bool bWaitForEvents, int filterIndex) {


		// HMD���ڑ�����Ă��邩�ǂ������m�F����
		if (!m_pHMD->IsTrackedDeviceConnected(0)) {
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "(OpenVR) HMD���ڑ�\n");
			printf_s(buf);
			return false;
		}

		// �n���h�R���g���[���[�̃{�^�����������Ȃǂ̃C�x���g��҂��Ă���A��͂���B
		if (bWaitForEvents) {
			// VREvent����������
			vr::VREvent_t event;
			while (m_pHMD->PollNextEvent(&event, sizeof(event)))
			{
				// �v���Z�X�C�x���g
				if (!ProcessVREvent(event, filterIndex)) {
					char buf[1024];
					sprintf_s(buf, sizeof(buf), "\n\n(OpenVR) �T�[�r�X�I���H\n");
					printf_s(buf);
				}
			}
		}
		else {
			// �C�x���g�Ɋւ�炸�A�g���b�L���O�f�[�^�̌p���I�ȃp�[�V���O������
			ParseTrackingFrame(filterIndex);
		}

		return true;
	}

}

OpenvrForDXLib::OpenvrForDXLib()
{
	m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
	if (error != vr::VRInitError_None) {
		m_pHMD = NULL;
		sprintf_s(buf, sizeof(buf), "VR�����^�C���̊J�n���ł��܂���ł���: %s", vr::VR_GetVRInitErrorAsEnglishDescription(error));
		printf_s(buf);
		exit(EXIT_FAILURE);
	}

	// VR Compositor ���L���ł��邱�Ƃ��m�F���A�����łȂ��ꍇ�̓|�[�Y���擾����ƃN���b�V�����܂��B
	if (!BInitCompositor()) {
		sprintf_s(buf, sizeof(buf), "VR Compositor�̏������Ɏ��s���܂����B");
		printf_s(buf);
		exit(EXIT_FAILURE);
	}
	else {
		sprintf_s(buf, sizeof(buf), "VR Compositor�̏������ɐ������܂����B\n");
		printf_s(buf);
	}

	// �}�j�t�F�X�g�t�@�C���̏���
	// ���s�t�@�C������̑��΃p�X���ݒ肳��Ă��邱�Ƃ��m�F���Ă��������B
	const char* manifestPath = "../../vr_binding/actions.json";
	std::string manifestFileName = Path_MakeAbsolute(manifestPath, Path_StripFilename(Path_GetExecutablePath()));

	// �t�@�C�������݂��邩�ǂ������m�F���Ă��瑱�s����
	if (!fileExists(manifestFileName)) {
		sprintf_s(buf, sizeof(buf), "\n�v���I�ȃG���[�ł��B�}�j�t�F�X�g�t�@�C�������݂��܂���B���[�h�Ɏ��s�����t�@�C��:\n%s\n\n", manifestFileName.c_str());
		printf_s(buf);
		exit(EXIT_FAILURE);
	}

	vr::EVRInputError inputError = vr::VRInput()->SetActionManifestPath(manifestFileName.c_str());
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�}�j�t�F�X�g�̃p�X���ݒ�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "�}�j�t�F�X�g�p�X�̎g�p�ɐ���: %s\n", manifestFileName.c_str());
		printf_s(buf);
	}

	// �V����IVRInput�̂��߂̃n���h��
	inputError = vr::VRInput()->GetActionSetHandle(actionSetDemoPath, &m_actionSetDemo);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�@�A�N�V�����Z�b�g�n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "%s �A�N�V�����Z�b�g�n���h���̎擾�ɐ������܂���: %d\n", actionSetDemoPath, (int)m_actionSetDemo);
		printf_s(buf);
	}

	// ���R���g���[���|�[�Y�p�n���h��
	inputError = vr::VRInput()->GetActionHandle(actionDemoHandLeftPath, &m_actionDemoHandLeft);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoHandLeftPath, (int)m_actionDemoHandLeft);
		printf_s(buf);
	}

	// �E�R���g���[���|�[�Y�p�n���h��
	inputError = vr::VRInput()->GetActionHandle(actionDemoHandRightPath, &m_actionDemoHandRight);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoHandRightPath, (int)m_actionDemoHandRight);
		printf_s(buf);
	}

	// �A�i���O�g���b�N�p�b�h����p�n���h��
	inputError = vr::VRInput()->GetActionHandle(actionDemoAnalogInputPath, &m_actionAnalogInput);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoAnalogInputPath, (int)m_actionAnalogInput);
		printf_s(buf);
	}

	// �������L���[�u�A�N�V�����̃n���h��
	inputError = vr::VRInput()->GetActionHandle(actionDemoHideCubesPath, &m_actionHideCubes);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���: %d\n", actionDemoHideCubesPath, (int)m_actionHideCubes);
		printf_s(buf);
	}

	// �^�b�`�A�N�V�����p�n���h��
	inputError = vr::VRInput()->GetActionHandle(actionDemoTouchPath, &m_actionTouch);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "�n���h�� %s �̎擾�ɐ������܂����B: %d\n", actionDemoTouchPath, (int)m_actionTouch);
		printf_s(buf);
	}

	// �N���b�N�n���h��
	inputError = vr::VRInput()->GetActionHandle(actionDemoClickPath, &m_actionClick);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "���̓n���h�� %s �̎擾�ɐ������܂����B: %d\n", actionDemoClickPath, (int)m_actionClick);
		printf_s(buf);
	}

	// �R���g���[���|�[�Y�\�[�X�p�n���h�� - ���ݖ��g�p
	inputError = vr::VRInput()->GetInputSourceHandle(inputHandLeftPath, &m_inputHandLeftPath);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[���̓n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "���̓n���h�� %s �̎擾�ɐ������܂����B: %d\n", inputHandLeftPath, (int)m_inputHandLeftPath);
		printf_s(buf);
	}

	inputError = vr::VRInput()->GetInputSourceHandle(inputHandRightPath, &m_inputHandRightPath);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[���̓n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "���̓n���h�� %s �̎擾�ɐ������܂����B: %d\n", inputHandRightPath, (int)m_inputHandRightPath);
		printf_s(buf);
	}

	PrintDevices();//�f�o�C�X�̏�Ԃ��擾

	Get_RecommendedRenderTargetSize(m_pHMD, &hmdWidth, &hmdHeight);
	printf("HMDWidth=%d\n", DXLIB_VR::GetHMDWidth());
	printf("HMDHeight=%d\n", DXLIB_VR::GetHMDHeight());
	SetupCameras();
	return true;
}

bool OpenvrForDXLib::BInitCompositor()
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if (!vr::VRCompositor())
	{
		printf("Compositor initialization failed. See log file for details\n");
		return false;
	}

	return true;
}
