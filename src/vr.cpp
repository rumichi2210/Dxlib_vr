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
vr::IVRSystem* m_pHMD = nullptr;
vr::EVRInitError error = vr::VRInitError_None;
uint32_t hmdWidth;
uint32_t hmdHeight;
vr::Texture_t eyeTexLeft;
vr::Texture_t eyeTexRight;
vr::IVRRenderModels* m_pRenderModels;
vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];
int m_iTrackedControllerCount;
int m_iTrackedControllerCount_Last;
int m_iValidPoseCount;
int m_iValidPoseCount_Last;
unsigned int m_uiControllerVertcount;
std::string m_strPoseClasses;                            // このフレームのポーズを見たとき
char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // 各デバイスについて、そのクラスを表す文字


namespace DXLIB_VR {

	bool Init() {
		m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
		if (error != vr::VRInitError_None) {
			m_pHMD = 0;
			return false;
		}
		Get_RecommendedRenderTargetSize(m_pHMD, &hmdWidth, &hmdHeight);
		printf("HMDWidth=%d\n", DXLIB_VR::GetHMDWidth());
		printf("HMDHeight=%d\n", DXLIB_VR::GetHMDHeight());
		SetupCameras();
		return true;
	}

	void Fin() { vr::VR_Shutdown(); }

	//HMDの画面サイズ(片目)を取得
	void Get_RecommendedRenderTargetSize(vr::IVRSystem* HMD, uint32_t* pnWidth, uint32_t* pnHeight) { HMD->GetRecommendedRenderTargetSize(pnWidth, pnHeight); }


	//描画する画像を送る
	void putTex(ID3D11Texture2D* texte, vr::EVREye eye) {
		if (eye == vr::EVREye::Eye_Left) {
			eyeTexLeft = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
		}
		if (eye == vr::EVREye::Eye_Right) {
			eyeTexRight = { (void*)texte, vr::ETextureType::TextureType_DirectX,vr::EColorSpace::ColorSpace_Auto };
		}
	}

	void MATRIX4_Print(Matrix4 val) {
		const float* pos = val.get();
		for (int i = 0; i < 16; i++) {
			printf("pos[%d]=%f\n", i, pos[i]);
		}
	}

	void MATRIX_Print(MATRIX val) {
		printf("val[0][n]->%f,%f,%f,%f\n", val.m[0][0], val.m[0][1], val.m[0][2], val.m[0][3]);
		printf("val[1][n]->%f,%f,%f,%f\n", val.m[1][0], val.m[1][1], val.m[1][2], val.m[1][3]);
		printf("val[2][n]->%f,%f,%f,%f\n", val.m[2][0], val.m[2][1], val.m[2][2], val.m[2][3]);
		printf("val[3][n]->%f,%f,%f,%f\n", val.m[3][0], val.m[3][1], val.m[3][2], val.m[3][3]);
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
		VAR_NAME(pfLeft);
		printf("%f", pfLeft);
		VAR_NAME(pfRight);
		printf("%f", pfRight);
		VAR_NAME(pfTop);
		printf("%f", pfTop);
		VAR_NAME(pfBottom);
		printf("%f", pfBottom);
	}

	MATRIX GetViewMat2(vr::EVREye eye) {
		Matrix4 matMVP;
		for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
		{
			const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevice];
			if (!pose.bPoseIsValid)
				continue;
			const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
			matMVP = GetCurrentViewProjectionMatrix(eye);// *matDeviceToTracking;
		}
		const float* pos = matMVP.get();
		MATRIX m_pos;
		m_pos.m[0][0] = pos[0];		m_pos.m[0][1] = pos[1];		m_pos.m[0][2] = pos[2];		m_pos.m[0][3] = pos[3];
		m_pos.m[1][0] = pos[4];		m_pos.m[1][1] = pos[5];		m_pos.m[1][2] = pos[6];		m_pos.m[1][3] = pos[7];
		m_pos.m[2][0] = pos[8];		m_pos.m[2][1] = pos[9];		m_pos.m[2][2] = pos[10];	m_pos.m[2][3] = pos[11];
		m_pos.m[3][0] = pos[12];	m_pos.m[3][1] = pos[13];	m_pos.m[3][2] = pos[14];	m_pos.m[3][3] = pos[15];
		return m_pos;
	}

	MATRIX GetViewMat(vr::EVREye eye) {

		Matrix4 matMVP;
		if (eye == vr::EVREye::Eye_Left) {
			ProjectionRawPrint(eye);
			for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
			{
				const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevice];
				if (!pose.bPoseIsValid)
					continue;

				printf("unTrackedDevice=%d->type<%c>\n", unTrackedDevice, m_rDevClassChar[unTrackedDevice]);
				const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
				VAR_NAME(matDeviceToTracking);
				MATRIX4_Print(matDeviceToTracking);
				matMVP = GetCurrentViewProjectionMatrix(vr::EVREye::Eye_Left) * matDeviceToTracking;
			}
			const float* pos = matMVP.get();
			MATRIX m_pos;
			m_pos.m[0][0] = pos[0]; m_pos.m[0][1] = pos[1]; m_pos.m[0][2] = pos[2]; m_pos.m[0][3] = pos[3];
			m_pos.m[1][0] = pos[4]; m_pos.m[1][1] = pos[5]; m_pos.m[1][2] = pos[6]; m_pos.m[1][3] = pos[7];
			m_pos.m[2][0] = pos[8]; m_pos.m[2][1] = pos[9]; m_pos.m[2][2] = pos[10]; m_pos.m[2][3] = pos[11];
			m_pos.m[3][0] = pos[12]; m_pos.m[3][1] = pos[13]; m_pos.m[3][2] = pos[14]; m_pos.m[3][3] = pos[15];
			VAR_NAME(m_pos);
			MATRIX_Print(m_pos);
			return m_pos;
		}
		else
		{
			ProjectionRawPrint(eye);
			for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
			{

				const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevice];
				if (!pose.bPoseIsValid)
					continue;
				printf("unTrackedDevice=%d->type<%c>\n", unTrackedDevice, m_rDevClassChar[unTrackedDevice]);
				const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
				VAR_NAME(matDeviceToTracking);
				MATRIX4_Print(matDeviceToTracking);
				matMVP = GetCurrentViewProjectionMatrix(vr::EVREye::Eye_Right) * matDeviceToTracking;
			}
			const float* pos = matMVP.get();
			MATRIX m_pos;
			m_pos.m[0][0] = pos[0]; m_pos.m[0][1] = pos[1]; m_pos.m[0][2] = pos[2]; m_pos.m[0][3] = pos[3];
			m_pos.m[1][0] = pos[4]; m_pos.m[1][1] = pos[5]; m_pos.m[1][2] = pos[6]; m_pos.m[1][3] = pos[7];
			m_pos.m[2][0] = pos[8]; m_pos.m[2][1] = pos[9]; m_pos.m[2][2] = pos[10]; m_pos.m[2][3] = pos[11];
			m_pos.m[3][0] = pos[12]; m_pos.m[3][1] = pos[13]; m_pos.m[3][2] = pos[14]; m_pos.m[3][3] = pos[15];
			VAR_NAME(m_pos);
			MATRIX_Print(m_pos);
			return m_pos;
		}
	}

	//デバイスの状態を一括で取り込む
	void updateVRState() {
		UpdateHMDMatrixPose();
	}

	//HMDに映る画像を更新
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
		Matrix4 matMVP=m_rmat4DevicePose[m_pHMD->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand)];
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
		vr::TrackedDevicePose_t tmp2;
		vr::VRControllerState_t night;
		m_pHMD->GetControllerStateWithPose(vr::TrackingUniverseStanding, 0, &night, &tmp2);
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
				VAR_NAME(m_rmat4DevicePose[nDevice]);
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

}

