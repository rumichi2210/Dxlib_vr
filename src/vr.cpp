#include "vr.hpp"

OpenvrForDXLib::OpenvrForDXLib()
{
	char buf[1024];
	m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
	if (error != vr::VRInitError_None) {
		m_pHMD = NULL;
		sprintf_s(buf, sizeof(buf), "VRランタイムの開始ができませんでした: %s", vr::VR_GetVRInitErrorAsEnglishDescription(error));
		printf_s(buf);
		return;
	}

	// VR Compositor が有効であることを確認し、そうでない場合はポーズを取得するとクラッシュします。
	if (!BInitCompositor()) {
		sprintf_s(buf, sizeof(buf), "VR Compositorの初期化に失敗しました。");
		printf_s(buf);
		return;
	}
	else {
		sprintf_s(buf, sizeof(buf), "VR Compositorの初期化に成功しました。\n");
		printf_s(buf);
	}

	// マニフェストファイルの準備
	// 実行ファイルからの相対パスが設定されていることを確認してください。
	const char* manifestPath = "../../vr_binding/actions.json";
	std::string manifestFileName = Path_MakeAbsolute(manifestPath, Path_StripFilename(Path_GetExecutablePath()));

	// ファイルが存在するかどうかを確認してから続行する
	if (!fileExists(manifestFileName)) {
		sprintf_s(buf, sizeof(buf), "\n致命的なエラーです。マニフェストファイルが存在しません。ロードに失敗したファイル:\n%s\n\n", manifestFileName.c_str());
		printf_s(buf);
		//exit(EXIT_FAILURE);
	}

	vr::EVRInputError inputError = vr::VRInput()->SetActionManifestPath(manifestFileName.c_str());
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "エラーマニフェストのパスが設定できません: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "マニフェストパスの使用に成功: %s\n", manifestFileName.c_str());
		printf_s(buf);
	}

	// 新しいIVRInputのためのハンドル

	inputError = vr::VRInput()->GetActionSetHandle(actionSetPath, &m_actionSet);
	if (inputError != vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "アクションセットハンドルが取得できません: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "アクションセットハンドル:%sの取得に成功しました: %d\n", actionSetPath, (int)m_actionSet);
		printf_s(buf);
	}

	// 左手用ハンドル
	GetActionHandleCheck(inputHandLeftPath, &m_inputHandLeftPath);

	// 右手用ハンドル
	GetActionHandleCheck(inputHandRightPath, &m_inputHandRightPath);

	// 選択用ハンドル
	GetActionHandleCheck(actionSelectPath, &m_actionSelect);

	// 移動用ハンドル
	GetActionHandleCheck(actionMovePath, &m_actionMove);

	// キャンセル用ハンドル
	GetActionHandleCheck(actionCancelPath, &m_actionCancel);

	// 決定用ハンドル
	GetActionHandleCheck(actionDecisionPath, &m_actionDecision);

	// 右コントローラー用ハンドル
	GetActionHandleCheck(actionControllerRightPath, &m_actionControllerRight);

	//　左コントローラー用ハンドル
	GetActionHandleCheck(actionControllerLeftPath, &m_actionControllerLeft);

	//デバイス情報を一括表示
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
		sprintf_s(buf, sizeof(buf), "エラーアクションハンドルを取得できません: %d\n", inputError);
		printf_s(buf);
	}
	else {
		sprintf_s(buf, sizeof(buf), "正常に %s ハンドルを取得しました\n", pchActionName);
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

	// 接続されたすべてのデバイスをループし、それらに関するいくつかの情報を表示する
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		// HMDがあることが条件のためスロット0にHMDが接続されていない場合は、残りのコードをスキップする
		if (!m_pHMD->IsTrackedDeviceConnected(unDevice))
			continue;

		// デバイスの種類を把握し、そのデータを使って仕事をする
		vr::ETrackedDeviceClass trackedDeviceClass = vr::VRSystem()->GetTrackedDeviceClass(unDevice);
		switch (trackedDeviceClass) {
		case vr::ETrackedDeviceClass::TrackedDeviceClass_Invalid:
			// 無効なクラスのために何かをする
			sprintf_s(buf, sizeof(buf), "デバイス %d: クラス[無効］", unDevice);
			printf_s(buf);
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
			// HMDはここで行い、コントローラ次のケースブロックにて行います。
			char buf[1024];
			sprintf_s(buf, sizeof(buf), "デバイス %d: Class: [HMD]", unDevice);
			printf_s(buf);
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
			// コントローラのためのものをここでやる
			sprintf_s(buf, sizeof(buf), "デバイス %d: クラス: [コントローラー]", unDevice);
			printf_s(buf);
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
			// 一般的なトラッカーのためのものをここで行う
			sprintf_s(buf, sizeof(buf), "デバイス %d: クラス: [一般的なトラッカー]", unDevice);
			printf_s(buf);
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference:
			/// トラッキングリファレンスのために、ここで何かをする
			sprintf_s(buf, sizeof(buf), "デバイス %d: クラス: [トラッキングリファレンス]", unDevice);
			printf_s(buf);
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect:
			/// ディスプレイのリダイレクトに必要なものをここで行う
			sprintf_s(buf, sizeof(buf), "デバイス %d: クラス: [DisplayRedirect]", unDevice);
			printf_s(buf);
			break;
		}

		// デバイスのメタデータを表示する

		// 新しいIVRInputのため、非推奨になっている関数を使用しています
		int32_t role;
		vr::ETrackedPropertyError pError;
		role = vr::VRSystem()->GetInt32TrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32, &pError);
		if (pError == vr::ETrackedPropertyError::TrackedProp_Success) {
			if (role == vr::ETrackedControllerRole::TrackedControllerRole_Invalid) {
				sprintf_s(buf, sizeof(buf), " | 無効な役割 (?): %d", role);
				printf_s(buf);
			}
			else {
				sprintf_s(buf, sizeof(buf), " | 役割: %d", role);
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

		sprintf_s(buf, sizeof(buf), " | マニュファクチャリング: %s | モデル: %s | シリアル: %s | 電源オフが可能かどうかの有無: %d\n", manufacturer, modelnumber, serialnumber, canPowerOff);
		printf_s(buf);
	}
	sprintf_s(buf, sizeof(buf), "---------------------------\nデバイス一覧の終了\n\n");
	printf_s(buf);

}

void OpenvrForDXLib::ParseTrackingFrame(int filterIndex) {

	char buf[1024];
	vr::EVRInputError inputError;

	sprintf_s(buf, sizeof(buf), "\n");
	printf_s(buf);

	// SteamVR のアクションの状態を処理する 
	// UpdateActionState はフレームごとに呼び出され、アクションの状態自体を更新する。
	//アプリケーションは、提供された VRActiveActionSet_t 構造体の配列を用いて、 
	// どのアクションセットがアクティブかを制御します。
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

	//// アナログデータ取得
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

	//// "タッチアクション "のデジタルデータ取得
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

	//// クリックアクションのデジタルデータ取得"
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


	// 右コントローラーのポーズを取得
	inputError = vr::VRInput()->GetPoseActionDataForNextFrame(m_actionControllerRight, vr::TrackingUniverseStanding, &controllerRightPoseData, sizeof(controllerRightPoseData), vr::k_ulInvalidInputValueHandle);
	if (inputError == vr::VRInputError_None) {
		sprintf_s(buf, sizeof(buf), "アクションデータを取得できました:%s\n", actionControllerRightPath);
		printf_s(buf);

		if (controllerRightPoseData.bActive) {
			vr::VRInputValueHandle_t activeOrigin = controllerRightPoseData.activeOrigin;
			bool bPoseIsValid = controllerRightPoseData.pose.bPoseIsValid;
			bool bDeviceIsConnected = controllerRightPoseData.pose.bDeviceIsConnected;
			sprintf_s(buf, sizeof(buf), "Origin: %d 有効かどうか: %d デバイスは接続済みかどうか: %d\n", (int)activeOrigin, bPoseIsValid, bDeviceIsConnected);
			printf_s(buf);
		}
		else {
			sprintf_s(buf, sizeof(buf), "%s | action not avail to be bound\n", actionControllerRightPath);
			printf_s(buf);
		}
	}
	else {
		sprintf_s(buf, sizeof(buf), "アクションデータを取得できませんでした%s | Error: %d\n", actionControllerRightPath, inputError);
		printf_s(buf);
	}

	printf("%0.2f,%0.2f,%0.2f", controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[0][3]
		, controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[1][3]
		, controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[2][3]);
	DrawSphere3D(VGet(controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[0][3], controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[1][3], controllerRightPoseData.pose.mDeviceToAbsoluteTracking.m[2][3])
		, 80.0f, 32, GetColor(255, 0, 0), GetColor(255, 0, 0),TRUE);
}

