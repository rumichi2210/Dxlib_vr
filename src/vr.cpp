#include "vr.hpp"

OpenvrForDXLib::OpenvrForDXLib(float nearClip, float farClip)
{
	m_fNearClip = nearClip;
	m_fFarClip = farClip;

	m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
	if (error != vr::VRInitError_None) {
		m_pHMD = NULL;
		std::cout << "VRランタイムの開始ができませんでした:" << vr::VR_GetVRInitErrorAsEnglishDescription(error) << std::endl;
		return;
	}

	// VR Compositor が有効であることを確認し、そうでない場合はポーズを取得するとクラッシュします。
	if (!BInitCompositor()) {
		std::cout << "VR Compositorの初期化に失敗しました。" << std::endl;
		return;
	}
	else {
		std::cout << "VR Compositorの初期化に成功しました" << std::endl;
	}

	// マニフェストファイルの準備
	std::string manifestFileName = Path_MakeAbsolute(manifestPath, Path_StripFilename(Path_GetExecutablePath()));
	// ファイルが存在するかどうかを確認してから続行する
	if (!fileExists(manifestFileName)) {
		std::cout << "致命的なエラーです。マニフェストファイルが存在しません。ロードに失敗したファイル:" << manifestFileName.c_str() << std::endl;
	}

	vr::EVRInputError inputError = vr::VRInput()->SetActionManifestPath(manifestFileName.c_str());
	if (inputError != vr::VRInputError_None) {
		std::cout << "エラーマニフェストのパスが設定できません:" << inputError << std::endl;
	}
	else {
		std::cout << "マニフェストパスの使用に成功しました:" << manifestFileName.c_str() << std::endl;
	}

	// 新しいIVRInputハンドル読み込み
	inputError = vr::VRInput()->GetActionSetHandle(actionSetPath.c_str(), &m_actionSet);
	if (inputError != vr::VRInputError_None) {
		std::cout << "アクションセットハンドルが取得できません:" << inputError << std::endl;
	}
	else {
		std::cout << "アクションセットハンドル:" << actionSetPath << "の取得に成功しました:" << std::endl;
	}

	// 左手用ハンドル
	GetActionHandle(inputHandLeftPath, &m_inputHandLeftPath);

	// 右手用ハンドル
	GetActionHandle(inputHandRightPath, &m_inputHandRightPath);

	// 選択用ハンドル
	GetActionHandle(actionSelectPath, &m_actionSelect);

	// 移動用ハンドル
	GetActionHandle(actionMovePath, &m_actionMove);

	// キャンセル用ハンドル
	GetActionHandle(actionCancelPath, &m_actionCancel);

	// 決定用ハンドル
	GetActionHandle(actionDecisionPath, &m_actionDecision);

	// 右コントローラー用ハンドル
	GetActionHandle(actionControllerRightPath, &m_actionControllerRight);

	//　左コントローラー用ハンドル
	GetActionHandle(actionControllerLeftPath, &m_actionControllerLeft);

	//デバイス情報を一括表示
	DeviceInfoBatchDisplay();

	//HMDの画面サイズ(片目)を取得
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
		std::cout << "コンポジターの初期化に失敗しました。詳しくはログファイルをご覧ください" << actionSetPath << std::endl;
		return false;
	}

	return true;
}

void OpenvrForDXLib::GetActionHandle(std::string actionName, vr::VRActionHandle_t* pHandle)
{
	vr::EVRInputError inputError = vr::VRInput()->GetActionHandle(actionName.c_str(), pHandle);
	if (inputError != vr::VRInputError_None) {
		std::cout << "エラーアクションハンドルを取得できません:" << inputError << std::endl;
	}
	else {
		std::cout << "正常に" << actionName << "ハンドルを取得しました" << std::endl;
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

	/// openVRのデータを更新
	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	/// openVRのデータを扱いやすいようにMatrix4に変換して格納
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

	/// HMDのデータをm_mat4HMDPoseに格納
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
			std::cout << "デバイス" << unDevice << "クラス[無効］" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_HMD:
			// HMDはここで行い、コントローラ次のケースブロックにて行います。
			std::cout << "デバイス" << unDevice << "クラス[HMD］" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_Controller:
			// コントローラのためのものをここでやる
			std::cout << "デバイス" << unDevice << "クラス[コントローラー］" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_GenericTracker:
			// 一般的なトラッカーのためのものをここで行う
			std::cout << "デバイス" << unDevice << "クラス[一般的なコントローラー］" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_TrackingReference:
			/// トラッキングリファレンスのために、ここで何かをする
			std::cout << "デバイス" << unDevice << "クラス[トラッキングリファレンス］" << std::endl;
			break;
		case vr::ETrackedDeviceClass::TrackedDeviceClass_DisplayRedirect:
			/// ディスプレイのリダイレクトに必要なものをここで行う
			std::cout << "デバイス" << unDevice << "クラス[DisplayRedirect］" << std::endl;
			break;
		}

		// デバイスのメタデータを表示する

		// 新しいIVRInputのため、非推奨になっている関数を使用しています
		int32_t role;
		vr::ETrackedPropertyError pError;
		role = vr::VRSystem()->GetInt32TrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ControllerRoleHint_Int32, &pError);
		if (pError == vr::ETrackedPropertyError::TrackedProp_Success) {
			if (role == vr::ETrackedControllerRole::TrackedControllerRole_Invalid) {
				std::cout << role << ":無効" << std::endl;
			}
			else {
				std::cout << role << ":有効" << std::endl;
			}
		}

		char manufacturer[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ManufacturerName_String, manufacturer, sizeof(manufacturer));

		char modelnumber[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_ModelNumber_String, modelnumber, sizeof(modelnumber));

		char serialnumber[1024];
		vr::VRSystem()->GetStringTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_SerialNumber_String, serialnumber, sizeof(serialnumber));

		bool canPowerOff = vr::VRSystem()->GetBoolTrackedDeviceProperty(unDevice, vr::ETrackedDeviceProperty::Prop_DeviceCanPowerOff_Bool);

		std::cout << "| マニュファクチャリング:" << manufacturer << "| モデル: " << modelnumber << " 電源オフが可能かどうかの有無:" << canPowerOff << std::endl;
	}
	std::cout << "---------------------------デバイス一覧の終了" << std::endl;
}

void OpenvrForDXLib::ParseTrackingFrame() {
	///basePosのデータは加算されていません。
	vr::EVRInputError inputError;

	// SteamVR のアクションの状態を処理する 
	// UpdateActionState はフレームごとに呼び出され、アクションの状態自体を更新する。
	//アプリケーションは、提供された VRActiveActionSet_t 構造体の配列を用いて、 
	// どのアクションセットがアクティブかを制御します。
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = m_actionSet;
	inputError = vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
	if (inputError == vr::VRInputError_None) {
		std::cout << actionSetPath << " | UpdateActionState(): Ok" << std::endl;
	}
	else {
		std::cout << actionSetPath << " | UpdateActionState():Error:" << inputError << std::endl;
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
		std::cout << "アクションデータを取得できました:\n" << actionControllerRightPath << std::endl;

		if (controllerRightPoseData.bActive) {
			vr::VRInputValueHandle_t activeOrigin = controllerRightPoseData.activeOrigin;
			bool bPoseIsValid = controllerRightPoseData.pose.bPoseIsValid;
			bool bDeviceIsConnected = controllerRightPoseData.pose.bDeviceIsConnected;
			std::cout << "Origin:" << (int)activeOrigin << "有効かどうか:" << bPoseIsValid << "デバイスは接続済みかどうか :" << bDeviceIsConnected << std::endl;
		}
		else {
			std::cout << actionControllerRightPath << " | action not avail to be bound\n" << std::endl;
		}
	}
	else {
		std::cout << "アクションデータを取得できませんでした" << actionControllerRightPath << "| Error:" << inputError << std::endl;
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
	DxLib::SetCameraScreenCenter((float)hmdWidth / 2.0f, (float)hmdHeight / 2.0f); //カメラが見ている映像の中心座標を再設定
	DxLib::SetupCamera_ProjectionMatrix(projection);
	DxLib::SetCameraViewMatrix(view);
	DrawTask();//描画処理
	DxLib::SetDrawScreen(DX_SCREEN_BACK);//描画先を元に戻す

	if (nEye == vr::Eye_Right) {
		PutHMD((ID3D11Texture2D*)DxLib::GetGraphID3D11Texture2D(eyeRightScreen), vr::Eye_Right);
	}
	else {
		PutHMD((ID3D11Texture2D*)DxLib::GetGraphID3D11Texture2D(eyeLeftScreen), vr::Eye_Left);
	}

}