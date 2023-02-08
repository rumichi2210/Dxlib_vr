#include "vr.hpp"

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
std::string m_strPoseClasses;                            // what classes we saw poses for this frame
char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class


namespace DXLIB_VR {

	bool Init() {
		m_pHMD = vr::VR_Init(&error, vr::VRApplication_Scene);
		if (error != vr::VRInitError_None) {
			m_pHMD = 0;
			return false;
		}
		Get_RecommendedRenderTargetSize(m_pHMD, &hmdWidth, &hmdHeight);
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

	MATRIX GetEyeMat(vr::EVREye eye) {

		Matrix4 matMVP;
		if (eye == vr::EVREye::Eye_Left) {

			for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
			{

				const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevice];
				if (!pose.bPoseIsValid)
					continue;

				const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
				matMVP = GetCurrentViewProjectionMatrix(vr::EVREye::Eye_Left) * matDeviceToTracking;
			}
			const float* pos = matMVP.get();
			MATRIX m_pos;
			m_pos.m[0][0] = pos[0]; m_pos.m[0][1] = pos[1]; m_pos.m[0][2] = pos[2]; m_pos.m[0][3] = pos[3];
			m_pos.m[1][0] = pos[4]; m_pos.m[1][1] = pos[5]; m_pos.m[1][2] = pos[6]; m_pos.m[1][3] = pos[7];
			m_pos.m[2][0] = pos[8]; m_pos.m[2][1] = pos[9]; m_pos.m[2][2] = pos[10]; m_pos.m[2][3] = pos[11];
			m_pos.m[3][0] = pos[12]; m_pos.m[3][1] = pos[13]; m_pos.m[3][2] = pos[14]; m_pos.m[3][3] = pos[15];


			return m_pos;
		}
		if (eye == vr::EVREye::Eye_Right) {
			for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
			{

				const vr::TrackedDevicePose_t& pose = m_rTrackedDevicePose[unTrackedDevice];
				if (!pose.bPoseIsValid)
					continue;

				const Matrix4& matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
				matMVP = GetCurrentViewProjectionMatrix(vr::EVREye::Eye_Right) * matDeviceToTracking;
			}
			const float* pos = matMVP.get();
			MATRIX m_pos;
			m_pos.m[0][0] = pos[0]; m_pos.m[0][1] = pos[1]; m_pos.m[0][2] = pos[2]; m_pos.m[0][3] = pos[3];
			m_pos.m[1][0] = pos[4]; m_pos.m[1][1] = pos[5]; m_pos.m[1][2] = pos[6]; m_pos.m[1][3] = pos[7];
			m_pos.m[2][0] = pos[8]; m_pos.m[2][1] = pos[9]; m_pos.m[2][2] = pos[10]; m_pos.m[2][3] = pos[11];
			m_pos.m[3][0] = pos[12]; m_pos.m[3][1] = pos[13]; m_pos.m[3][2] = pos[14]; m_pos.m[3][3] = pos[15];


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
			matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
		}
		else if (nEye == vr::Eye_Right)
		{
			matMVP = m_mat4ProjectionRight * m_mat4eyePosRight * m_mat4HMDPose;
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

