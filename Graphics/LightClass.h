#pragma once

class LightClass
{
public:
	LightClass();
	LightClass(const LightClass&);
	~LightClass();

	void SetAmbientColor(XMFLOAT4);
	void SetDiffuseColor(XMFLOAT4);
	void SetPosition(XMFLOAT3);
	void SetLookAt(XMFLOAT3);
	void SetSpecularColor(XMFLOAT4);
	void SetSpecularPower(float);

	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();
	XMFLOAT3 GetDirection();
	XMFLOAT4 GetSpecularColor();
	float GetSpecularPower();
	XMFLOAT3 GetPosition();


	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float, float);

	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);

private:
	XMFLOAT4 m_ambientColor;
	XMFLOAT4 m_diffuseColor;
	XMFLOAT3 m_lookAt;
	XMFLOAT4 m_specularColor;
	float m_specularPower;

	XMFLOAT3 m_position;

	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;
};