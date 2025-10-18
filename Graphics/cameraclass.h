////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


//////////////
// INCLUDES //
//////////////
#include <directxmath.h>

#include "AlignedAllocationPolicy.h"

using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: CameraClass
////////////////////////////////////////////////////////////////////////////////
class CameraClass : public AlignedAllocationPolicy<16>
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);
	
	void SetPositionY(float y); // 새로 추가된 함수: Y 위치만 업데이트 (지형 높이 추적용)

	void SetCamYaw(float);
	void SetCamPitch(float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	float GetCamYaw();
	float GetCamPitch();

	bool Initialize();
	void Render();
	void UpdateCamera(float&, float&);
	void GetViewMatrix(XMMATRIX&);

	void RenderBaseViewMatrix();
	void GetBaseViewMatrix(XMMATRIX&);

private:
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_baseViewMatrix;

	XMVECTOR DefaultForward;
	XMVECTOR DefaultRight;
	XMVECTOR camForward;
	XMVECTOR camRight;

	XMMATRIX camRotationMatrix;
	XMMATRIX groundWorld;

	float camYaw = 0.0f;
	float camPitch = 0.0f;
	float camRoll = 0.0f;

	float rotx = 0;
	float rotz = 0;
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	XMMATRIX Rotationx;
	XMMATRIX Rotationz;

	XMMATRIX WVP;
	XMMATRIX cube1World;
	XMMATRIX cube2World;
	XMMATRIX camView;
	XMMATRIX camProjection;

	XMMATRIX d2dWorld;

	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;

	float camMoveLR = 0;
	float camMoveBF = 0;
};

#endif