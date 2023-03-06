#include "vr.hpp"

OpenvrForDXLib::OpenvrForDXLib(float nearClip, float farClip)
{
	m_fNearClip = nearClip;
	m_fFarClip = farClip;

	m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
	if (error != vr::VRInitError_None) {
		m_pHMD = NULL;
		std::cout << "VR�����^�C���̊J�n���ł��܂���ł���:" << vr::VR_GetVRInitErrorAsEnglishDescription(error) << std::endl;
		return;
	}

	// VR Compositor ���L���ł��邱�Ƃ��m�F���A�����łȂ��ꍇ�̓|�[�Y���擾����ƃN���b�V�����܂��B
	if (!BInitCompositor()) {
		std::cout << "VR Compositor�̏������Ɏ��s���܂����B" << std::endl;
		return;
	}
	else {
		std::cout << "VR Compositor�̏������ɐ������܂���" << std::endl;
	}

	// �}�j�t�F�X�g�t�@�C���̏���
	std::string manifestFileName = Path_MakeAbsolute(manifestPath, Path_StripFilename(Path_GetExecutablePath()));
	// �t�@�C�������݂��邩�ǂ������m�F���Ă��瑱�s����
	if (!fileExists(manifestFileName)) {
		std::cout << "�v���I�ȃG���[�ł��B�}�j�t�F�X�g�t�@�C�������݂��܂���B���[�h�Ɏ��s�����t�@�C��:" << manifestFileName.c_str() << std::endl;
	}

	vr::EVRInputError inputError = vr::VRInput()->SetActionManifestPath(manifestFileName.c_str());
	if (inputError != vr::VRInputError_None) {
		std::cout << "�G���[�}�j�t�F�X�g�̃p�X���ݒ�ł��܂���:" << inputError << std::endl;
	}
	else {
		std::cout << "�}�j�t�F�X�g�p�X�̎g�p�ɐ������܂���:" << manifestFileName.c_str() << std::endl;
	}

	// �V����IVRInput�n���h���ǂݍ���
	inputError = vr::VRInput()->GetActionSetHandle(actionSetPath.c_str(), &m_actionSet);
	if (inputError != vr::VRInputError_None) {
		std::cout << "�A�N�V�����Z�b�g�n���h�����擾�ł��܂���:" << inputError << std::endl;
	}
	else {
		std::cout << "�A�N�V�����Z�b�g�n���h��:" << actionSetPath << "�̎擾�ɐ������܂���:" << std::endl;
	}

	// ����p�n���h��
	GetActionHandle(inputHandLeftPath, &m_inputHandLeftPath);

	// �E��p�n���h��
	GetActionHandle(inputHandRightPath, &m_inputHandRightPath);

	// �I��p�n���h��
	GetActionHandle(actionSelectPath, &m_actionSelect);

	// �ړ��p�n���h��
	GetActionHandle(actionMovePath, &m_actionMove);

	// �L�����Z���p�n���h��
	GetActionHandle(actionCancelPath, &m_actionCancel);

	// ����p�n���h��
	GetActionHandle(actionDecisionPath, &m_actionDecision);

	// �E�R���g���[���[�p�n���h��
	GetActionHandle(actionControllerRightPath, &m_actionControllerRight);

	//�@���R���g���[���[�p�n���h��
	GetActionHandle(actionControllerLeftPath, &m_actionControllerLeft);

	//�f�o�C�X�����ꊇ�\��
	DeviceInfoBatchDisplay();

	//HMD�̉�ʃT�C�Y(�Ж�)���擾
	m_pHMD->GetRecommendedRenderTargetSize(&hmdWidth, &hmdHeight);
	SetupCameras();

	eyeRightScreen = DxLib::MakeScreen(hmdWidth, hmdHeight, FALSE);
	eyeLeftScreen = DxLib::MakeScreen(hmdWidth, hmdHeight, FALSE);
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
	error = vr::VRInitError_None;

	if (!vr::VRCompositor())
	{
		std::cout << "�R���|�W�^�[�̏������Ɏ��s���܂����B�ڂ����̓��O�t�@�C����������������" << actionSetPath << std::endl;
		return false;
	}

	return true;
}

void OpenvrForDXLib::GetActionHandle(std::string actionName, vr::VRActionHandle_t* pHandle)
{
	vr::EVRInputError inputError = vr::VRInput()->GetActionHandle(actionName.c_str(), pHandle);
	if (inputError != vr::VRInputError_None) {
		std::cout << "�G���[�A�N�V�����n���h�����擾�ł��܂���:" << inputError << std::endl;
	}
	else {
		std::cout << "�����" << actionName << "�n���h�����擾���܂���" << std::endl;
	}
}

void OpenvrForDXLib::SetupCameras()
{
	m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
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

void OpenvrForDXLib::UpdateVRMatrixPose(VECTOR basePos)
{
	if (!m_pHMD)
		return;

	/// openVR�̃f�[�^���X�V
	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	/// openVR�̃f�[�^�������₷���悤��Matrix4�ɕϊ����Ċi�[
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);


			Matrix4 baseMatrix(
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f,
				basePos.x, basePos.y, basePos.z, 1.0f
			);
			m_rmat4DevicePose[nDevice] += baseMatrix;
		}
	}

	/// HMD�̃f�[�^��m_mat4HMDPose�Ɋi�[
	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		m_mat4HMDPose.invert();
	}
}

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
	vr::Texture_t eyeTex = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
	if (eye == vr::EVREye::Eye_Left) {
		vr::VRCompositor()->Submit(vr::EVREye::Eye_Left, &eyeTex);
	}
	else {
		vr::VRCompositor()->Submit(vr::EVREye::Eye_Right, &eyeTex);
	}
}

void OpenvrForDXLib::UpdateState(VECTOR basePos)
{
	UpdateVRMatrixPose(basePos);
	ParseTrackingFrame();
}

void OpenvrForDXLib::DeviceInfoBatchDisplay() {

	std::cout << "Device list:---------------------------" << std::endl;

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
			std::cout << "�f�o�C�X" << unDevice << "�N���X[�����n" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
			// HMD�͂����ōs���A�R���g���[�����̃P�[�X�u���b�N�ɂčs���܂��B
			std::cout << "�f�o�C�X" << unDevice << "�N���X[HMD�n" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
			// �R���g���[���̂��߂̂��̂������ł��
			std::cout << "�f�o�C�X" << unDevice << "�N���X[�R���g���[���[�n" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
			// ��ʓI�ȃg���b�J�[�̂��߂̂��̂������ōs��
			std::cout << "�f�o�C�X" << unDevice << "�N���X[��ʓI�ȃR���g���[���[�n" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference:
			/// �g���b�L���O���t�@�����X�̂��߂ɁA�����ŉ���������
			std::cout << "�f�o�C�X" << unDevice << "�N���X[�g���b�L���O���t�@�����X�n" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect:
			/// �f�B�X�v���C�̃��_�C���N�g�ɕK�v�Ȃ��̂������ōs��
			std::cout << "�f�o�C�X" << unDevice << "�N���X[DisplayRedirect�n" << std::endl;
			break;
		}

		// �f�o�C�X�̃��^�f�[�^��\������

		// �V����IVRInput�̂��߁A�񐄏��ɂȂ��Ă���֐����g�p���Ă��܂�
		int32_t role;
		vr::ETrackedPropertyError pError;
		role = vr::VRSystem()->GetInt32TrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32, &pError);
		if (pError == vr::ETrackedPropertyError::TrackedProp_Success) {
			if (role == vr::ETrackedControllerRole::TrackedControllerRole_Invalid) {
				std::cout << role << ":����" << std::endl;
			}
			else {
				std::cout << role << ":�L��" << std::endl;
			}
		}

		char manufacturer[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, manufacturer, sizeof(manufacturer));

		char modelnumber[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, modelnumber, sizeof(modelnumber));

		char serialnumber[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_SerialNumber_String, serialnumber, sizeof(serialnumber));

		bool canPowerOff = vr::VRSystem()->GetBoolTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_DeviceCanPowerOff_Bool);

		std::cout << "| �}�j���t�@�N�`�������O:" << manufacturer << "| ���f��: " << modelnumber << " �d���I�t���\���ǂ����̗L��:" << canPowerOff << std::endl;
	}
	std::cout << "---------------------------�f�o�C�X�ꗗ�̏I��" << std::endl;
}

void OpenvrForDXLib::ParseTrackingFrame() {
	///basePos�̃f�[�^�͉��Z����Ă��܂���B
	vr::EVRInputError inputError;

	// SteamVR �̃A�N�V�����̏�Ԃ��������� 
	// UpdateActionState �̓t���[�����ƂɌĂяo����A�A�N�V�����̏�Ԏ��̂��X�V����B
	//�A�v���P�[�V�����́A�񋟂��ꂽ VRActiveActionSet_t �\���̂̔z���p���āA 
	// �ǂ̃A�N�V�����Z�b�g���A�N�e�B�u���𐧌䂵�܂��B
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = m_actionSet;
	inputError = vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
	if (inputError == vr::VRInputError_None) {
		std::cout << actionSetPath << " | UpdateActionState(): Ok" << std::endl;
	}
	else {
		std::cout << actionSetPath << " | UpdateActionState():Error:" << inputError << std::endl;
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
		std::cout << "�A�N�V�����f�[�^���擾�ł��܂���:\n" << actionControllerRightPath << std::endl;

		if (controllerRightPoseData.bActive) {
			vr::VRInputValueHandle_t activeOrigin = controllerRightPoseData.activeOrigin;
			bool bPoseIsValid = controllerRightPoseData.pose.bPoseIsValid;
			bool bDeviceIsConnected = controllerRightPoseData.pose.bDeviceIsConnected;
			std::cout << "Origin:" << (int)activeOrigin << "�L�����ǂ���:" << bPoseIsValid << "�f�o�C�X�͐ڑ��ς݂��ǂ��� :" << bDeviceIsConnected << std::endl;
		}
		else {
			std::cout << actionControllerRightPath << " | action not avail to be bound\n" << std::endl;
		}
	}
	else {
		std::cout << "�A�N�V�����f�[�^���擾�ł��܂���ł���" << actionControllerRightPath << "| Error:" << inputError << std::endl;
	}

	printf("%f,%f,%f", controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[0][3]
		, controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[1][3]
		, controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[2][3]);
}

void OpenvrForDXLib::UpdateVRScreen(vr::Hmd_Eye nEye, void (*DrawTask)(void)) {
	MATRIX projection;
	MATRIX view;
	if (nEye == vr::Eye_Right) {
		DxLib::SetDrawScreen(eyeRightScreen);
		projection = GetProjectionMatrix(vr::Eye_Right);
		view = GetViewMatrix(vr::Eye_Right);
	}
	else {
		DxLib::SetDrawScreen(eyeLeftScreen);
		projection = GetProjectionMatrix(vr::Eye_Left);
		view = GetViewMatrix(vr::Eye_Left);
	}
	DxLib::ClearDrawScreenZBuffer();
	DxLib::ClearDrawScreen();
	DxLib::SetCameraScreenCenter((float)hmdWidth / 2.0f, (float)hmdHeight / 2.0f); //�J���������Ă���f���̒��S���W���Đݒ�
	DxLib::SetupCamera_ProjectionMatrix(projection);
	DxLib::SetCameraViewMatrix(view);
	DrawTask();//�`�揈��
	DxLib::SetDrawScreen(DX_SCREEN_BACK);//�`�������ɖ߂�

	if (nEye == vr::Eye_Right) {
		PutHMD((ID3D11Texture2D*)DxLib::GetGraphID3D11Texture2D(eyeRightScreen), vr::Eye_Right);
	}
	else {
		PutHMD((ID3D11Texture2D*)DxLib::GetGraphID3D11Texture2D(eyeLeftScreen), vr::Eye_Left);
	}

}