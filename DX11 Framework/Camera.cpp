#include "Camera.h"

Camera::Camera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up, float windowWidth, float windowHeight, float nearDepth, float farDepth)
{
	_eye = position;
	_at = at;
	_up = up;
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
	m_Type = CameraType::LookAt;
	XMVECTOR eyeVec = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
	XMVECTOR atVec = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);
	XMVECTOR upVec = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);
	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(eyeVec, atVec, upVec));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, _nearDepth , _farDepth));
}

Camera::Camera(XMFLOAT3 position, XMFLOAT3 up, float windowWidth, float windowHeight, float nearDepth, float farDepth)
{
	_eye = position;
	_up = up;
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
	m_Type = CameraType::LookTo;
	XMVECTOR eyeVec = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
	XMVECTOR upVec = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);
	_forward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR forwardVec = XMVectorSet(_forward.x, _forward.y, _forward.z, 0.0f);
	forwardVec = XMVector3Normalize(forwardVec);
	XMStoreFloat3(&_forward, forwardVec);
	XMStoreFloat4x4(&_view, XMMatrixLookToLH(eyeVec, forwardVec, upVec));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, _nearDepth, _farDepth));
}

void Camera::Update(float deltaTime)
{
	if (!isLerping) {
		if (GetAsyncKeyState(0x57)) { //w key
			if (m_Type == CameraType::LookTo) { //if look to camera then move forward
				XMVECTOR forward = XMVectorSet(0, 0, 1, 0);
				MoveDirection(forward, deltaTime);
			}
			else if (m_Type == CameraType::LookAt) { //if look at camera then zoom in

				//find direction
				XMVECTOR dir = XMVectorSet(_at.x - _eye.x, _at.y - _eye.y, _at.z - _eye.z, 0);
				dir = XMVector3Normalize(dir);


				//calculate distance after zoom
				XMVECTOR curPosVec = XMVectorSet(_eye.x, _eye.y, _eye.z, 0);
				XMVECTOR newPosVec = curPosVec + (dir * CAMERA_SPEED * deltaTime);
				XMVECTOR atVector = XMVectorSet(_at.x, _at.y, _at.z, 0);
				XMVECTOR newDistance = atVector - newPosVec;
				XMVECTOR lengthVec = XMVector3Length(newDistance);
				float distance = 0.0f;
				XMStoreFloat(&distance, lengthVec);
				if (distance > 2.0f) {
					MoveDirection(dir, deltaTime);
				}
			}
		}
		else if (GetAsyncKeyState(0x53)) { //s key
			if (m_Type == CameraType::LookTo) { //if look to camera then move backwards
				XMVECTOR back = XMVectorSet(0, 0, -1, 0);
				MoveDirection(back, deltaTime);
			}
			else if (m_Type == CameraType::LookAt) { //if look at camera then zoom out
				XMVECTOR dir = XMVectorSet(_at.x - _eye.x, _at.y - _eye.y, _at.z - _eye.z, 0);
				dir = XMVector3Normalize(-dir);
				MoveDirection(dir, deltaTime);
			}
		}

		if (GetAsyncKeyState(0x41)) { //A key
			if (m_Type == CameraType::LookTo) { //if look to camera then move left
				XMVECTOR left = XMVectorSet(-1, 0, 0, 0);
				MoveDirection(left, deltaTime);
			}
			else if (m_Type == CameraType::LookAt) {
				RotateY(_at, CAMERA_ROTATE_SPEED * deltaTime);
			}
		}

		else if (GetAsyncKeyState(0x44)) { //D Key
			if (m_Type == CameraType::LookTo) { //if look to camera then move right
				XMVECTOR right = XMVectorSet(1, 0, 0, 0);
				MoveDirection(right, deltaTime);
			}
			else if (m_Type == CameraType::LookAt) {
				RotateY(_at, -CAMERA_ROTATE_SPEED * deltaTime);
			}
		}
	}
	XMVECTOR eyeVec = XMVectorSet(_eye.x, _eye.y, _eye.z, 0.0f);
	XMVECTOR upVec = XMVectorSet(_up.x, _up.y, _up.z, 0.0f);
	if (m_Type == CameraType::LookAt) {
		XMVECTOR atVec = XMVectorSet(_at.x, _at.y, _at.z, 0.0f);
		XMStoreFloat4x4(&_view, XMMatrixLookAtLH(eyeVec, atVec, upVec));
	}
	else if (m_Type == CameraType::LookTo) {
		XMVECTOR forwardVec = XMVectorSet(_forward.x, _forward.y, _forward.z, 0.0f);
		XMStoreFloat4x4(&_view, XMMatrixLookToLH(eyeVec, forwardVec, upVec));
	}
}

void Camera::SetPos(XMFLOAT3 newPos)
{
	_eye = newPos;
}

void Camera::SetAt(XMFLOAT3 newAt)
{
	_at = newAt;
}

void Camera::SetUp(XMFLOAT3 newUp)
{
	_up = newUp;
}

void Camera::SetForward(XMFLOAT3 newForward)
{
	_forward = newForward;
}

void Camera::SetType(CameraType newType)
{
	m_Type = newType;
}

void Camera::MoveDirection(XMVECTOR direction, float deltaTime)
{
	XMVECTOR curPosVec = XMVectorSet(_eye.x, _eye.y, _eye.z, 0);
	XMVECTOR newPosVec = curPosVec + (direction * CAMERA_SPEED * deltaTime);
	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos, newPosVec);
	SetPos(newPos);
}

bool Camera::LerpToPosition(XMFLOAT3 lerpPos, float deltaTime, float secondsToLerp)
{
	if (!isLerping) {
		lerpStartPos = _eye;
		isLerping = true;
	}
	_lerpElapsedTime += deltaTime;
	float lerpFactor = _lerpElapsedTime / secondsToLerp;
	float x = MathFunction::lerp(lerpStartPos.x, lerpPos.x, lerpFactor);
	float y = MathFunction::lerp(lerpStartPos.y, lerpPos.y, lerpFactor);
	float z = MathFunction::lerp(lerpStartPos.z, lerpPos.z, lerpFactor);
	XMFLOAT3 nextPos = XMFLOAT3(x, y, z);
	SetPos(nextPos);
	if (lerpFactor <= 1) {
		return false;
	}
	else if (lerpFactor > 1) {
		isLerping = false;
		_lerpElapsedTime = 0;
		return true;
	}
	
}

int Camera::LerpThroughPositions(std::vector<XMFLOAT3>& listOfPositions, float deltaTime, float secondsPerPos)
{
	if (_lerpCurrentListPosition < listOfPositions.size()) {
		if (LerpToPosition(listOfPositions.at(_lerpCurrentListPosition), deltaTime, secondsPerPos)) {
			_lerpCurrentListPosition++;
		}
	}
	if (_lerpCurrentListPosition >= listOfPositions.size()) {
		_lerpCurrentListPosition = 0;
		return listOfPositions.size();
	}
	return _lerpCurrentListPosition;
}

void Camera::RotateY(XMFLOAT3 center, float rotationFactor)
{
	XMFLOAT4X4 cameraWorldPos;
	XMStoreFloat4x4(&cameraWorldPos,  XMMatrixTranslation(_eye.x-center.x, _eye.y - center.y, _eye.z - center.z) * XMMatrixRotationY(rotationFactor) * XMMatrixTranslation(center.x, center.y, center.z));
	XMVECTOR PosVec;
	XMVECTOR RotVec;
	XMVECTOR ScaleVec;
	XMMatrixDecompose(&ScaleVec, &RotVec, &PosVec, XMLoadFloat4x4(&cameraWorldPos));
	XMFLOAT3 PosFloat3;
	XMStoreFloat3(&PosFloat3, PosVec);
	SetPos(PosFloat3);
}

XMFLOAT3 Camera::GetPos()
{
	return _eye;
}

XMFLOAT3 Camera::GetAt()
{
	return _at;
}

XMFLOAT3 Camera::GetUp()
{
	return _up;
}


XMFLOAT3 Camera::GetForward()
{
	return _forward;
}

XMFLOAT4X4 Camera::GetView()
{
	return _view;
}

XMFLOAT4X4 Camera::GetProjection()
{
	return _projection;
}

XMFLOAT4X4 Camera::GetProjectionView()
{
	XMMATRIX _viewMat = XMLoadFloat4x4(&_view);
	XMMATRIX _projMat = XMLoadFloat4x4(&_projection);
	XMMATRIX multiply =  XMMatrixMultiply(_viewMat, _projMat);
	XMFLOAT4X4 returnVal;
	XMStoreFloat4x4(&returnVal, multiply);
	return returnVal;

	
}

void Camera::Reshape(float windowWidth, float windowHeight, float nearDepth, float farDepth)
{
	_windowWidth = windowWidth;
	_windowHeight = windowHeight;
	_nearDepth = nearDepth;
	_farDepth = farDepth;
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, _nearDepth, _farDepth));
}

