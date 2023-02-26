#pragma once
#include "DxLib.h"
#include <D3D11.h>
#include "openvr.h"
#include "Matrices.h"
#include "pathtools.h"
#pragma comment(lib,"openvr_api.lib")
#pragma warning(suppress : 4996)//Matrices.hとpathtools.hでvisual studio非推奨関数を使用しているため


namespace DXLIB_VR {

}

class OpenvrForDXLib {
private:

	// 基本的なもの
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

	// 新しいIVRInputのためのハンドル
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

	//HMDの画面サイズ(片目)を取得
	void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight) { m_pHMD->GetRecommendedRenderTargetSize(pnWidth, pnHeight); }


	// Purpose: nEyeを基準としたMatrix Projection Eyeを取得します。
	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);


	// Purpose: nEyeを基準としたHMDMatrixPoseEyeを取得します。
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);

	// VRコンポジターの初期化
	bool BInitCompositor();

	//HMD用のカメラの準備をします
	void SetupCameras();

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	void UpdateHMDMatrixPose();

	//接続機器の情報をCUIに一括表示
	void DeviceInformationBatchDisplay();
public:
	OpenvrForDXLib();
	~OpenvrForDXLib();





	// openvr のイベントをリッスンし、process と parse のルーチンを呼び出すメインループ、もし false ならばサービスは終了している
	bool RunProcedure(bool bWaitForEvents, int filterIndex);

	// VRイベントを処理し、何が起こったかについての一般的な情報を表示します。
	bool ProcessVREvent(const vr::VREvent_t& event, int filterIndex);

	// トラッキングフレームを解析し、その位置/回転/イベントを表示します。
	// filterIndex に-1 以外を指定すると、特定のデバイスのデータのみを表示する。
	void ParseTrackingFrame(int filterIndex);

	// 差分計算
	vr::HmdVector3_t GetControllerPositionDelta();
	vr::HmdVector3_t GetLeftControllerPosition();
	vr::HmdVector3_t GetRightControllerPosition();


	//vr::VRActionHandle_tで定義したものしか使用できません
	void GetActionHandleCheck(const char* pchActionName, vr::VRActionHandle_t* pHandle);

	MATRIX GetProjectionMatrix(vr::EVREye eye);
	MATRIX GetViewMatrix(vr::EVREye eye);
	//MATRIX GetProjectionMatrix(vr::Hmd_Eye nEye);
	int GetHMDWidth() { return hmdWidth; }
	int GetHMDHeight() { return hmdHeight; }
	//HMDで描画する
	void PutHMD(ID3D11Texture2D* texte, vr::EVREye eye);

	//デバイスの状態を一括で取り込む
	void UdateState();

};





