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
	XMStoreFloat4x4(&_view, XMMatrixLookToLH(eyeVec, forwardVec, upVec));
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _windowWidth / (FLOAT)_windowHeight, _nearDepth, _farDepth));
}

void Camera::Update()
{
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

