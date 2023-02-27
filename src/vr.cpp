#include "vr.hpp"

OpenvrForDXLib::OpenvrForDXLib()
{
	char buf[1024];
	m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
	if (error != vr::VRInitError_None) {
		m_pHMD = NULL;
		sprintf_s(buf, sizeof(buf), "VR�����^�C���̊J�n���ł��܂���ł���: %s", vr::VR_GetVRInitErrorAsEnglishDescription(error));
		printf_s(buf);
		return;
	}

	// VR Compositor ���L���ł��邱�Ƃ��m�F���A�����łȂ��ꍇ�̓|�[�Y���擾����ƃN���b�V�����܂��B
	if (!BInitCompositor()) {
		sprintf_s(buf, sizeof(buf), "VR Compositor�̏������Ɏ��s���܂����B");
		printf_s(buf);
		return;
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
		//exit(EXIT_FAILURE);
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

	inputError = vr::VRInput()->GetActionSetHandle(actionSetPath, &m_actionSet);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�A�N�V�����Z�b�g�n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "�A�N�V�����Z�b�g�n���h��:%s�̎擾�ɐ������܂���: %d\n", actionSetPath, (int)m_actionSet);
		printf_s(buf);
	}

	// ����p�n���h��
	GetActionHandleCheck(inputHandLeftPath, &m_inputHandLeftPath);

	// �E��p�n���h��
	GetActionHandleCheck(inputHandRightPath, &m_inputHandRightPath);

	// �I��p�n���h��
	GetActionHandleCheck(actionSelectPath, &m_actionSelect);

	// �ړ��p�n���h��
	GetActionHandleCheck(actionMovePath, &m_actionMove);

	// �L�����Z���p�n���h��
	GetActionHandleCheck(actionCancelPath, &m_actionCancel);

	// ����p�n���h��
	GetActionHandleCheck(actionDecisionPath, &m_actionDecision);

	// �E�R���g���[���[�p�n���h��
	GetActionHandleCheck(actionControllerRightPath, &m_actionControllerRight);

	//�@���R���g���[���[�p�n���h��
	GetActionHandleCheck(actionControllerLeftPath, &m_actionControllerLeft);

	//�f�o�C�X�����ꊇ�\��
	DeviceInfoBatchDisplay();

	GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);
	SetupCameras();
}

OpenvrForDXLib::~OpenvrForDXLib()
{
	if (m_pHMD != NULL)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}
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

void OpenvrForDXLib::GetActionHandleCheck(const char* pchActionName, vr::VRActionHandle_t* pHandle)
{
	char buf[1024];
	vr::EVRInputError inputError = vr::VRInput()->GetActionHandle(pchActionName, pHandle);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�G���[�A�N�V�����n���h�����擾�ł��܂���: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "����� %s �n���h�����擾���܂���\n", pchActionName);
		printf_s(buf);
	}
}

void OpenvrForDXLib::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
	m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
}

Matrix4 OpenvrForDXLib::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

Matrix4 OpenvrForDXLib::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);
	return matrixObj.invert();
}


MATRIX OpenvrForDXLib::GetProjectionMatrix(vr::EVREye eye) {
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

MATRIX OpenvrForDXLib::GetViewMatrix(vr::EVREye eye) {
	Matrix4 matMVP;
	matMVP = GetCurrentViewProjectionMatrix(eye);
	const float* pos = matMVP.get();
	MATRIX m_pos{};
	m_pos.m[0][0] = pos[0];		m_pos.m[0][1] = pos[1];		m_pos.m[0][2] = pos[2];		m_pos.m[0][3] = pos[3];
	m_pos.m[1][0] = pos[4];		m_pos.m[1][1] = pos[5];		m_pos.m[1][2] = pos[6];		m_pos.m[1][3] = pos[7];
	m_pos.m[2][0] = pos[8];		m_pos.m[2][1] = pos[9];		m_pos.m[2][2] = pos[10];	m_pos.m[2][3] = pos[11];
	m_pos.m[3][0] = pos[12];	m_pos.m[3][1] = pos[13];	m_pos.m[3][2] = pos[14];	m_pos.m[3][3] = pos[15];
	return m_pos;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void OpenvrForDXLib::UpdateHMDMatrixPose()
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
Matrix4 OpenvrForDXLib::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose)
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
// Purpose: Gets a Current View Projection Matrix with respect to nEye,
//          which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
Matrix4 OpenvrForDXLib::GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
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

void OpenvrForDXLib::PutHMD(ID3D11Texture2D* texte, vr::EVREye eye) {
	if (eye == vr::EVREye::Eye_Left) {
		eyeTexLeft = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
		vr::VRCompositor()->Submit(vr::EVREye::Eye_Left, &eyeTexLeft);
	}
	else {
		eyeTexRight = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
		vr::VRCompositor()->Submit(vr::EVREye::Eye_Right, &eyeTexRight);
	}
}

void OpenvrForDXLib::UpdateState()
{
	UpdateHMDMatrixPose();
	ParseTrackingFrame(-1);
}

void OpenvrForDXLib::DeviceInfoBatchDisplay() {

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

		// �V����IVRInput�̂��߁A�񐄏��ɂȂ��Ă���֐����g�p���Ă��܂�
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

void OpenvrForDXLib::ParseTrackingFrame(int filterIndex) {

	char buf[1024];
	vr::EVRInputError inputError;

	sprintf_s(buf, sizeof(buf), "\n");
	printf_s(buf);

	// SteamVR �̃A�N�V�����̏�Ԃ��������� 
	// UpdateActionState �̓t���[�����ƂɌĂяo����A�A�N�V�����̏�Ԏ��̂��X�V����B
	//�A�v���P�[�V�����́A�񋟂��ꂽ VRActiveActionSet_t �\���̂̔z���p���āA 
	// �ǂ̃A�N�V�����Z�b�g���A�N�e�B�u���𐧌䂵�܂��B
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = m_actionSet;
	inputError = vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
	if (inputError == vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "%s | UpdateActionState(): Ok\n", actionSetPath);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "%s | UpdateActionState(): Error: %d\n", actionSetPath, inputError);
		printf_s(buf);
	}

	//// �A�i���O�f�[�^�擾
	//vr::InputAnalogActionData_t analogData;
	//inputError = vr::VRInput()->GetAnalogActionData(m_actionAnalogInput, &analogData, sizeof(analogData), vr::k_ulInvalidInputValueHandle);
	//if (inputError == vr::VRInputError_None)
	//{
	//	sprintf_s(buf, sizeof(buf), "%s | GetAnalogActionData() Ok\n", actionDemoAnalogInputPath);
	//	printf_s(buf);

	//	if (analogData.bActive) {
	//		float m_vAnalogValue0 = analogData.x;
	//		float m_vAnalogValue1 = analogData.y;
	//		sprintf_s(buf, sizeof(buf), "%s | x: %f  y:%f\n", actionDemoAnalogInputPath, m_vAnalogValue0, m_vAnalogValue1);
	//		printf_s(buf);
	//	}
	//	else {
	//		sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoAnalogInputPath);
	//		printf_s(buf);
	//	}
	//}
	//else {
	//	sprintf_s(buf, sizeof(buf), "%s | GetAnalogActionData() Not Ok. Error: %d\n", actionDemoAnalogInputPath, inputError);
	//	printf_s(buf);
	//}


	//// Get digital data
	//vr::InputDigitalActionData_t digitalData;
	//inputError = vr::VRInput()->GetDigitalActionData(m_actionHideCubes, &digitalData, sizeof(digitalData), vr::k_ulInvalidInputValueHandle);
	//if (inputError == vr::VRInputError_None)
	//{
	//	sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Ok\n", actionDemoHideCubesPath);
	//	printf_s(buf);

	//	if (digitalData.bActive) {
	//		bool m_vDigitalValue0 = digitalData.bState;
	//		sprintf_s(buf, sizeof(buf), "%s | State: %d\n", actionDemoHideCubesPath, m_vDigitalValue0);
	//		printf_s(buf);

	//		// check from which device the action came
	//		vr::InputOriginInfo_t originInfo;
	//		if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(digitalData.activeOrigin, &originInfo, sizeof(originInfo)))
	//		{
	//			if (originInfo.devicePath == m_inputHandLeftPath) {
	//				sprintf_s(buf, sizeof(buf), "Action comes from left hand\n");
	//				printf_s(buf);
	//			}
	//			else if (originInfo.devicePath == m_inputHandRightPath) {
	//				sprintf_s(buf, sizeof(buf), "Action comes from right hand\n");
	//				printf_s(buf);
	//			}
	//		}

	//	}
	//	else {
	//		sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoHideCubesPath);
	//		printf_s(buf);
	//	}
	//}
	//else {
	//	sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Not Ok. Error: %d\n", actionDemoHideCubesPath, inputError);
	//	printf_s(buf);
	//}

	//// "�^�b�`�A�N�V���� "�̃f�W�^���f�[�^�擾
	//vr::InputDigitalActionData_t digitalDataTouch;
	//inputError = vr::VRInput()->GetDigitalActionData(m_actionTouch, &digitalDataTouch, sizeof(digitalDataTouch), vr::k_ulInvalidInputValueHandle);
	//if (inputError == vr::VRInputError_None)
	//{
	//	sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Ok\n", actionDemoTouchPath);
	//	printf_s(buf);

	//	if (digitalDataTouch.bActive) {
	//		bool m_vDigitalValue0 = digitalDataTouch.bState;
	//		sprintf_s(buf, sizeof(buf), "%s | State: %d\n", actionDemoTouchPath, m_vDigitalValue0);
	//		printf_s(buf);

	//		// check from which device the action came
	//		vr::InputOriginInfo_t originInfo;
	//		if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(digitalDataTouch.activeOrigin, &originInfo, sizeof(originInfo)))
	//		{
	//			if (originInfo.devicePath == m_inputHandLeftPath) {
	//				sprintf_s(buf, sizeof(buf), "Action comes from left hand\n");
	//				printf_s(buf);
	//			}
	//			else if (originInfo.devicePath == m_inputHandRightPath) {
	//				sprintf_s(buf, sizeof(buf), "Action comes from right hand\n");
	//				printf_s(buf);
	//			}
	//		}

	//	}
	//	else {
	//		sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoTouchPath);
	//		printf_s(buf);
	//	}
	//}
	//else {
	//	sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Not Ok. Error: %d\n", actionDemoTouchPath, inputError);
	//	printf_s(buf);
	//}

	//// �N���b�N�A�N�V�����̃f�W�^���f�[�^�擾"
	//vr::InputDigitalActionData_t digitalDataClick;
	//inputError = vr::VRInput()->GetDigitalActionData(m_actionClick, &digitalDataClick, sizeof(digitalDataClick), vr::k_ulInvalidInputValueHandle);
	//if (inputError == vr::VRInputError_None)
	//{
	//	sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Ok\n", actionDemoClickPath);
	//	printf_s(buf);

	//	if (digitalDataClick.bActive) {
	//		bool m_vDigitalValue0 = digitalDataClick.bState;
	//		sprintf_s(buf, sizeof(buf), "%s | State: %d\n", actionDemoClickPath, m_vDigitalValue0);
	//		printf_s(buf);

	//		// check from which device the action came
	//		vr::InputOriginInfo_t originInfo;
	//		if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(digitalDataClick.activeOrigin, &originInfo, sizeof(originInfo)))
	//		{
	//			if (originInfo.devicePath == m_inputHandLeftPath) {
	//				sprintf_s(buf, sizeof(buf), "Action comes from left hand\n");
	//				printf_s(buf);
	//			}
	//			else if (originInfo.devicePath == m_inputHandRightPath) {
	//				sprintf_s(buf, sizeof(buf), "Action comes from right hand\n");
	//				printf_s(buf);
	//			}
	//		}

	//	}
	//	else {
	//		sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionDemoClickPath);
	//		printf_s(buf);
	//	}
	//}
	//else {
	//	sprintf_s(buf, sizeof(buf), "%s | GetDigitalActionData() Not Ok. Error: %d\n", actionDemoClickPath, inputError);
	//	printf_s(buf);
	//}


	// �E�R���g���[���[�̃|�[�Y���擾
	inputError = vr::VRInput()->GetPoseActionDataForNextFrame(m_actionControllerRight, vr::TrackingUniverseStanding, &controllerRightPoseData, sizeof(controllerRightPoseData), vr::k_ulInvalidInputValueHandle);
	if (inputError == vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "�A�N�V�����f�[�^���擾�ł��܂���:%s\n", actionControllerRightPath);
		printf_s(buf);

		if (controllerRightPoseData.bActive) {
			vr::VRInputValueHandle_t activeOrigin = controllerRightPoseData.activeOrigin;
			bool bPoseIsValid = controllerRightPoseData.pose.bPoseIsValid;
			bool bDeviceIsConnected = controllerRightPoseData.pose.bDeviceIsConnected;
			sprintf_s(buf, sizeof(buf), "Origin: %d �L�����ǂ���: %d �f�o�C�X�͐ڑ��ς݂��ǂ���: %d\n", (int)activeOrigin, bPoseIsValid, bDeviceIsConnected);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionControllerRightPath);
			printf_s(buf);
		}
	}
	else {
		sprintf_s(buf, sizeof(buf), "�A�N�V�����f�[�^���擾�ł��܂���ł���%s | Error: %d\n", actionControllerRightPath, inputError);
		printf_s(buf);
	}

	printf("%0.2f,%0.2f,%0.2f", controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[0][3]
		, controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[1][3]
		, controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[2][3]);
	DrawSphere3D(VGet(controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[0][3], controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[1][3], controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[2][3])
		, 80.0f, 32, GetColor(255, 0, 0), GetColor(255, 0, 0),TRUE);
}

