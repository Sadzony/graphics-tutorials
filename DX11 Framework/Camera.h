#pragma once
#include "Structures.h"
enum class CameraType {
	LookTo,
	LookAt
};
class Camera
{
private:
	XMFLOAT3 _eye;
	XMFLOAT3 _at;
	XMFLOAT3 _up;
	XMFLOAT3 _forward;

	float _windowWidth;
	float _windowHeight;
	float _nearDepth;
	float _farDepth;
	CameraType m_Type;
	
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
public:
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, float windowWidth, float windowHeight, float nearDepth, float farDepth);
	Camera(XMFLOAT3 position, XMFLOAT3 up, float windowWidth, float windowHeight, float nearDepth, float farDepth);
	~Camera();
	void Update();
	void SetPos(XMFLOAT3 newPos);
	void SetAt(XMFLOAT3 newAt);
	void SetUp(XMFLOAT3 newUp);
	void SetForward(XMFLOAT3 newForward);
	void SetType(CameraType newType);
	XMFLOAT3 GetPos();
	XMFLOAT3 GetAt();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetForward();

	XMFLOAT4X4 GetView();
	XMFLOAT4X4 GetProjection();
	XMFLOAT4X4 GetProjectionView();

	void Reshape(float windowWidth, float windowHeight, float nearDepth, float farDepth);
};

