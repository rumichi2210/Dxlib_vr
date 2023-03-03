#pragma once
#include "DxLib.h"
#include <D3D11.h>
#include "openvr.h"
#include "Matrices.h"
#include "pathtools.h"
#pragma comment(lib,"openvr_api.lib")
#pragma warning(suppress : 4996)//Matrices.hとpathtools.hでvisual studio非推奨関数を使用しているため

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
	//加算
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
	/// VRアプリケーションに必ず必要なデータ
	vr::IVRSystem* m_pHMD = NULL;
	vr::EVRInitError error = vr::VRInitError_None;//openVRのエラー格納用
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];//openVRから取得する生データの格納(右手座標系)

	/// 処理に使用するVRデータ
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];

	/// HMDに描画する際の最適画面サイズ
	uint32_t hmdWidth = 0;
	uint32_t hmdHeight = 0;

	/// カメラ設定用
	Matrix4 m_mat4eyePosLeft = Matrix4();
	Matrix4 m_mat4eyePosRight = Matrix4();
	Matrix4 m_mat4HMDPose = Matrix4();

	float m_fNearClip;
	float m_fFarClip;



	/// IVRInput用のデータ///
	// 実行ファイルからの相対パスが設定されていることを確認してください。
	std::string manifestPath = "../../vr_binding/actions.json";
	vr::VRActionSetHandle_t m_actionSet = vr::k_ulInvalidActionSetHandle;
	std::string actionSetPath = "/actions/main";

	/// [Tips]
	/// IVRInputから位置などのデータ(pose)を取得するのは推奨しません。
	/// IVRInputはコントローラーの入力(ボタンやトラックパッド)や感覚デバイスを使用する場合に向いています。(キーバインド機能のため)
	/// 一般的なトラッカー(フルトラッキングデバイスなどのボタン入力を持たないデバイス)ではm_rmat4DevicePoseのデータを直接使用することをおすすめします。

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


	// nEyeを基準としたHMDMatrixPoseEyeを取得します。
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);

	// VRコンポジターの初期化
	bool BInitCompositor();

	//HMD用のカメラの準備をします
	void SetupCameras();

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	void UpdateHMDMatrixPose();

	// トラッキングフレームを解析し、その位置/回転/イベントを表示します。
	void ParseTrackingFrame();

	//接続機器の情報をCUIに一括表示
	void DeviceInfoBatchDisplay();

	//vr::VRActionHandle_tで定義したものしか使用できません
	void GetActionHandle(std::string actionName,vr::VRActionHandle_t* pHandle);
public:
	OpenvrForDXLib(float nearClip, float farClip);
	~OpenvrForDXLib();

	// nEyeを基準としたMatrixProjectionEyeを取得します。
	MATRIX GetProjectionMatrix(vr::EVREye eye);
	MATRIX GetViewMatrix(vr::EVREye eye);
	int GetHMDWidth() { return hmdWidth; }
	int GetHMDHeight() { return hmdHeight; }

	//HMDで描画する
	void PutHMD(ID3D11Texture2D* texte, vr::EVREye eye);

	//デバイスの状態を一括で取り込む
	void UpdateState();

	//OpenVRが正常に動作しているかを取得する
	bool vrCheck() { return (error == vr::VRInitError_None) ? true : false; }

};





