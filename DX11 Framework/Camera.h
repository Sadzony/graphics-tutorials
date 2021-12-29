#pragma once
#include "Globals.h"
#include "Structures.h"
#include <vector>
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
	XMFLOAT3 _right;
	float curAngle = 0;

	float _windowWidth;
	float _windowHeight;
	float _nearDepth;
	float _farDepth;

	XMFLOAT3 lerpStartPos;
	float _lerpElapsedTime = 0;
	bool isLerping = false;
	int _lerpCurrentListPosition = 0;

	CameraType m_Type;
	
	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;
public:
	Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, float windowWidth, float windowHeight, float nearDepth, float farDepth);
	Camera(XMFLOAT3 position, XMFLOAT3 up, float windowWidth, float windowHeight, float nearDepth, float farDepth);
	~Camera();
	void Update(float deltaTime);
	void SetPos(XMFLOAT3 newPos);
	void SetAt(XMFLOAT3 newAt);
	void SetUp(XMFLOAT3 newUp);
	void SetForward(XMFLOAT3 newForward);
	void SetRight(XMFLOAT3 newRight);
	void SetType(CameraType newType);

	void MoveDirection(XMVECTOR direction, float deltaTime);
	void RotateLookToCamera(float rotationFactor);
	bool LerpToPosition(XMFLOAT3 lerpPos, float deltaTime, float secondsToLerp);
	int LerpThroughPositions(std::vector<XMFLOAT3>& listOfPositions, float deltaTime, float secondsPerPos);
	void RotateY(XMFLOAT3 center, float newAngle);
	XMFLOAT3 GetPos();
	XMFLOAT3 GetAt();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetForward();
	XMFLOAT3 GetRight();

	XMFLOAT4X4 GetView();
	XMFLOAT4X4 GetProjection();
	XMFLOAT4X4 GetProjectionView();

	void Reshape(float windowWidth, float windowHeight, float nearDepth, float farDepth);
};

