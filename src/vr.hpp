#pragma once
#include "DxLib.h"
#include <D3D11.h>
#include "openvr.h"
#include "Matrices.h"
#pragma comment(lib,"openvr_api.lib")


namespace DXLIB_VR {
	bool Init();
	void Fin();
	void putTex(ID3D11Texture2D* texte, vr::EVREye eye);
	MATRIX GetViewMat(vr::EVREye eye);
	MATRIX GetProjectiontMat(vr::EVREye eye);
	void updateVRState();
	void render();
	int GetHMDWidth();
	int GetHMDHeight();

	MATRIX GetContolloer();
	VECTOR GetLeftContolloer();
	MATRIX GetViewMat2(vr::EVREye eye);

	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose);
	void UpdateHMDMatrixPose();
	void SetupCameras();
	void Get_RecommendedRenderTargetSize(vr::IVRSystem* HMD, uint32_t* pnWidth, uint32_t* pnHeight);
}





