#pragma once
#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include <vector>
#include "DDSTextureLoader.h"
#include "Structures.h"
#include "Globals.h"
#include "OBJLoader.h"
#include "Camera.h"
using namespace DirectX;

struct ConstantBuffer
{
	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 AmbientLight;
	XMFLOAT4 AmbientMaterial;

	XMFLOAT4 DiffuseMaterial;
	XMFLOAT4 DiffuseLight;

	XMFLOAT4 SpecularMaterial;
	XMFLOAT4 SpecularLight;
	float SpecularPower;
	XMFLOAT3 EyePosW;

	XMFLOAT3 LightVecw;
	float gTime;



};

class Application
{
private:
	ID3D11RasterizerState* _wireFrame;
	ID3D11RasterizerState* _rasterizerSolid;
	char _currentState;
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;

	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;

	ID3D11BlendState* Transparency;

	ID3D11Buffer*           _cVertexBuffer;
	ID3D11Buffer*           _cIndexBuffer;
	ID3D11Buffer*			_pVertexBuffer;
	ID3D11Buffer*			_plVertexBuffer;
	ID3D11Buffer*			_pIndexBuffer;
	ID3D11Buffer*			_plIndexBuffer;
	ID3D11Buffer*           _pConstantBuffer;
	XMFLOAT4X4              _world;

	Camera* cameras[2];
	int currentCameraIndex = 0;
	bool lerpResult = false;
	std::vector<XMFLOAT3> lerpPositions;

	//light objects
	XMFLOAT3 lightDirection;
	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	XMFLOAT4 ambientLight;
	XMFLOAT4 ambientMaterial;

	XMFLOAT4 specularLight;
	XMFLOAT4 specularMaterial;
	float specularPower;

	//texturing
	ID3D11ShaderResourceView* _pTextureRV = nullptr;
	ID3D11ShaderResourceView* _pTextureTree = nullptr;

	ID3D11SamplerState* _pSamplerLinear = nullptr;

	//world objects
	XMFLOAT4X4              _sunWorldPos, _planet1WorldPos, _planet2WorldPos, _moon1WorldPos, _moon2WorldPos, _planeWorldPos;
	MeshData objMeshData;
	//depth and stencil buffer
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11Texture2D* _depthStencilBuffer;

	bool _RKeyPressed = false;
	float updateTime = 0;
private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();

	UINT _WindowHeight;
	UINT _WindowWidth;

public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

