////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_position.x = 0.0f;
	m_position.y = 0.0f;
	m_position.z = 0.0f;

	m_rotation.x = 0.0f;
	m_rotation.y = 0.0f;
	m_rotation.z = 0.0f;

	DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

// ✅ 새로운 메서드: Y 위치만 업데이트 (지형 높이 추적용)
void CameraClass::SetPositionY(float y)
{
	m_position.y = y;
	// camPosition의 Y 값도 직접 업데이트
	camPosition = XMVectorSetY(camPosition, y);
}

void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}

void CameraClass::SetCamYaw(float yaw)
{
	camYaw = yaw;
}

void CameraClass::SetCamPitch(float pitch)
{
	camPitch = pitch;
}

XMFLOAT3 CameraClass::GetPosition()
{
	return m_position;
}


XMFLOAT3 CameraClass::GetRotation()
{
	return m_rotation;
}

float CameraClass::GetCamYaw()
{
	return camYaw;
}

float CameraClass::GetCamPitch()
{
	return camPitch;
}


bool CameraClass::Initialize()
{

	// Setup the vector that points upwards.
	camUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// Setup the position of the camera in the world.
	camPosition = XMLoadFloat3(&m_position);

	// Setup where the camera is looking by default.
	camTarget = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	camPitch = m_rotation.x * 0.0174532925f;
	camYaw = m_rotation.y * 0.0174532925f;
	camRoll = m_rotation.z * 0.0174532925f;

	// Create the rotation matrix from the yaw, pitch, and roll values.
	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, camRoll);

	// Transform the lookAt and up vector by the rotation matrix so the view is correctly rotated at the origin.
	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	camUp = XMVector3TransformCoord(camUp, camRotationMatrix);

	// Translate the rotated camera position to the location of the viewer.
	camTarget = camPosition + camTarget;

	return true;
}

void CameraClass::Render()
{
	// Rotating the Camera
	//
	// The first three lines in our UpdateCamera() function will update the cameras target. 
	// The first line uses a new function from the xna math library, called 
	// XMMatrixRotationRollPitchYaw. This function has three parameters, the first is the pitch 
	// in radians to rotate, the second is the Yaw in radians to rotate, and the third is the Roll 
	// in radians to rotate. It returns a rotation matrix. This function is very usefull for cameras 
	// especially. you are able to rotate around all axis' at the same time, Yaw, Pitch, Roll. 
	// Yaw is the rotation left/right (y-axis), pitch is the rotation up/down (x-axis) and Roll is 
	// the rotation like your doing a cartwheel (z-axis). Our camera will be looking up/down and 
	// left/right, So we will be setting the yaw and pitch parameters of this function. Since we are 
	// not using the roll parameter, we set that to zero.

	camRotationMatrix = XMMatrixRotationRollPitchYaw(camPitch, camYaw, 0);

	// The next line sets our cameras Target vector by rotating the DefaultForward vector with the 
	// rotationMatrix we just created, and setting it to our Target matrix. After that, we normalize 
	// our vector, because the last operation may have made one or more of the Target vectors values 
	// greater than 1.0f, or less than -1.0f, in which case it would be not of unit length.

	camTarget = XMVector3TransformCoord(DefaultForward, camRotationMatrix);
	camTarget = XMVector3Normalize(camTarget);

	// Now we need to find the new Right and Forward directions of our camera. We will do this using 
	// a rotation matrix which will be rotated on the Y-axis. Since our camera is a first person camera, 
	// and we only need to move in the X and Z directions, We need to keep our camera's forward and right 
	// vectors pointing only in the x and z axis. We will use those vectors to move our camera back, 
	// forward, left and right. First we create a new matrix, then we rotate that matrix using the yaw 
	// variable we have set when the mouse moves.

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = XMMatrixRotationY(camYaw);

	// Update the camForward, camUp, and camRight Vectors
	//
	// Next we transform the camera's right, up, and forward vectors using the RotateYTempMatrix matrix 
	// we just defined, and rotate the default right, up, and default forward vectors then set the result 
	// in the right, up, and forward vectors.
	camRight = XMVector3TransformCoord(DefaultRight, RotateYTempMatrix);
	camUp = XMVector3TransformCoord(camUp, RotateYTempMatrix);
	camForward = XMVector3TransformCoord(DefaultForward, RotateYTempMatrix);

	// Moving the Camera
	// 
	// Next we update the position of our camera using the two values, moveLeftRight, and moveBackForward, 
	// and the two vectors, Right, and Forward. To move the camera left and right, we multiply the moveLeftRight 
	// variable with the Right vector, and add that to the Position. Then to move back and forward, we multiply 
	// moveBackForward with the Forward vector and add that to the Position too. The moveLeftRight and 
	// moveBackForward values will be calculated when direct input detects a certain key was pressed (in the 
	// case of this lesson A, S, D, or W).
	// After that we reset the moveLeftRight and moveBackForward variables.
	camPosition += camMoveLR * camRight;
	camPosition += camMoveBF * camForward;

	camMoveLR = 0.0f;
	camMoveBF = 0.0f;

	// Set the camView Matrix
	//
	// Now we add the Position of our camera to the target vector, then update our View matrix by using the 
	// XMMatrixLookAtLH() D3D function. This function updates our View matrix we use to calculate our WVP 
	// matrix by using the cameras position, Target, and Up vectors.

	camTarget = camPosition + camTarget;

	m_viewMatrix = XMMatrixLookAtLH(camPosition, camTarget, camUp);
	XMStoreFloat3(&m_position, camPosition);

	return;
}

void CameraClass::UpdateCamera(float& moveLeftRight, float& moveBackForward)
{
	camMoveLR = moveLeftRight;
	camMoveBF = moveBackForward;

	moveLeftRight = 0.0f;
	moveBackForward = 0.0f;

	return;
}

void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void CameraClass::RenderBaseViewMatrix()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVector, positionVector, lookAtVector;
	// 위쪽을 가리키는 벡터를 설정합니다.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// XMVECTOR 구조체에 로드한다.
	upVector = XMLoadFloat3(&up);

	// 3D월드에서 카메라의 위치를 ​​설정합니다.
	position = m_position;

	// XMVECTOR 구조체에 로드한다.
	positionVector = XMLoadFloat3(&position);

	// 라디안 단위로 회전을 계산합니다.
	float radians = m_rotation.y * 0.0174532925f;

	// 카메라 포지션을 lookAt 값에 설정합니다.
	lookAt.x = sinf(radians) + m_position.x;
	lookAt.y = m_position.y;
	lookAt.z = cosf(radians) + m_position.z;

	// XMVECTOR 구조체에 로드한다.
	lookAtVector = XMLoadFloat3(&lookAt);

	// 마지막으로 세 개의 업데이트 된 벡터에서 뷰 행렬을 만듭니다.
	m_baseViewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}


void CameraClass::GetBaseViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_baseViewMatrix;
}