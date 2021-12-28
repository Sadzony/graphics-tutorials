#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
    if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
        return E_FAIL;
	}

    RECT rc;
    GetClientRect(_hWnd, &rc);
    _WindowWidth = rc.right - rc.left;
    _WindowHeight = rc.bottom - rc.top;

    if (FAILED(InitDevice()))
    {
        Cleanup();

        return E_FAIL;
    }

	// Initialize the world matrix
	XMStoreFloat4x4(&_world, XMMatrixIdentity());

    // Initialize the camera
    XMFLOAT3 Eye = XMFLOAT3(2.5f, 10.0f, 7.5f);
    XMFLOAT3 At = XMFLOAT3(0.0f, 0.0f, 7.5f);
    XMFLOAT3 Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    cameras[0] = new Camera(Eye,At,Up, _WindowWidth, _WindowHeight, 0.01f, 100.0f);

    Eye = XMFLOAT3(0.0f, 7.5f, -3.0f);
    Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
    cameras[1] = new Camera(Eye, Up, _WindowWidth, _WindowHeight, 0.01f, 100.0f);
    XMFLOAT3 lerpPos1 = XMFLOAT3(10.0f, 10.0f, 15.0f);
    XMFLOAT3 lerpPos2 = XMFLOAT3(10.0f, 20.0f, 15.0f);
    XMFLOAT3 lerpPos3 = XMFLOAT3(2.5f, 10.0f, 7.5f);
    lerpPositions.push_back(lerpPos1);
    lerpPositions.push_back(lerpPos2);
    lerpPositions.push_back(lerpPos3);

    //load texture
    HRESULT hr = CreateDDSTextureFromFile(_pd3dDevice, L"Crate_COLOR.dds", nullptr, &_pTextureRV);
    if (FAILED(hr))
        return hr;
    hr = CreateDDSTextureFromFile(_pd3dDevice, L"Pine Tree.dds", nullptr, &_pTextureTree);
    if (FAILED(hr))
        return hr;
    //define sampler
    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);
    objMeshData = OBJLoader::Load("sphere.obj", _pd3dDevice, false);
	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = nullptr;

    hr = CompileShaderFromFile(L"DX11 Framework.fx", "VS", "vs_4_0", &pVSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the vertex shader
    
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{	
		pVSBlob->Release();
        return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"DX11 Framework.fx", "PS", "ps_4_0", &pPSBlob);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
                   L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
                                        pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
        return hr;

    // Set the input layout
    _pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;

    // Create vertex buffer
    SimpleVertex cubeVertices[] =
    {
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ),   XMFLOAT3(-0.816497f, 0.408248f, -0.408248f),    XMFLOAT2(0.0f,1.0f) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f),     XMFLOAT3(0.333333f, 0.666667f, -0.666667f),     XMFLOAT2(1.0f,1.0f) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f),   XMFLOAT3(-0.408248f, -0.408248f, -0.816497f),   XMFLOAT2(0.0f,0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f),    XMFLOAT3(0.666667f, -0.666667f, -0.333333),     XMFLOAT2(1.0f,0.0f) },

        { XMFLOAT3(-1.0f, 1.0f, 1.0f),      XMFLOAT3(-0.333333f, 0.666667f, 0.666667f),         XMFLOAT2(0.0f,1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f),       XMFLOAT3(0.816497f, 0.408248f, 0.408248f),           XMFLOAT2(1.0f,1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(-0.666667f, -0.666667f, 0.333333f),      XMFLOAT2(0.0f,0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(0.408248f, -0.408248f, 0.816497f),       XMFLOAT2(1.0f,0.0f) },
    };

    D3D11_BUFFER_DESC cvb;
	ZeroMemory(&cvb, sizeof(cvb));
    cvb.Usage = D3D11_USAGE_DEFAULT;
    cvb.ByteWidth = sizeof(SimpleVertex) * 8;
    cvb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    cvb.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = cubeVertices;

    hr = _pd3dDevice->CreateBuffer(&cvb, &InitData, &_cVertexBuffer);

    if (FAILED(hr))
        return hr;

    SimpleVertex pyramidVertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f),    XMFLOAT3(-0.7071f, 0.0f, -0.7071f), XMFLOAT2(0.0f,0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f),     XMFLOAT3(0.7071f, 0.0f, -0.7071f),  XMFLOAT2(1.0f,0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f),     XMFLOAT3(-0.7071f, 0.0f, 0.7071f),  XMFLOAT2(0.0f,0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f),      XMFLOAT3(0.7071f, 0.0f, 0.7071f),   XMFLOAT2(1.0f,0.0f) },

        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.5f,1.0f) },

    };

    D3D11_BUFFER_DESC pvb;
    ZeroMemory(&pvb, sizeof(pvb));
    pvb.Usage = D3D11_USAGE_DEFAULT;
    pvb.ByteWidth = sizeof(SimpleVertex) * 5;
    pvb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    pvb.CPUAccessFlags = 0; 
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = pyramidVertices;

    hr = _pd3dDevice->CreateBuffer(&pvb, &InitData, &_pVertexBuffer);

    if (FAILED(hr))
        return hr;

    SimpleVertex planeVertices[] =
    {
        { XMFLOAT3(-2.0f, 0.0f, 2.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.0f ,1.0f)},
        { XMFLOAT3(-1.0f, 0.0f, 2.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.25f,1.0f) },
        { XMFLOAT3(0.0f, 0.0f, 2.0f),   XMFLOAT3(0.0f, 1.0f, 0.0f),   XMFLOAT2(0.5f ,1.0f) },
        { XMFLOAT3(1.0f, 0.0f, 2.0f),   XMFLOAT3(0.0f, 1.0f, 0.0f),   XMFLOAT2(0.75f,1.0f) },
        { XMFLOAT3(2.0f, 0.0f, 2.0f),   XMFLOAT3(0.0f, 1.0f, 0.0f),   XMFLOAT2(1.0f ,1.0f) },

        { XMFLOAT3(-2.0f, 0.0f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.0f ,0.75f) },
        { XMFLOAT3(-1.0f, 0.0f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.25f,0.75f) },
        { XMFLOAT3(0.0f,  0.0f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.5f ,0.75f) },
        { XMFLOAT3(1.0f,  0.0f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.75f,0.75f) },
        { XMFLOAT3(2.0f,  0.0f, 1.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(1.0f ,0.75f) },

        { XMFLOAT3(-2.0f, 0.0f, 0.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.0f ,0.5f) },
        { XMFLOAT3(-1.0f, 0.0f, 0.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.25f,0.5f) },
        { XMFLOAT3(0.0f,  0.0f, 0.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.5f ,0.5f) },
        { XMFLOAT3(1.0f,  0.0f, 0.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(0.75f,0.5f) },
        { XMFLOAT3(2.0f,  0.0f, 0.0f),  XMFLOAT3(0.0f, 1.0f, 0.0f),  XMFLOAT2(1.0f ,0.5f) },
                         
        { XMFLOAT3(-2.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f ,0.25f) },
        { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.25f,0.25f) },
        { XMFLOAT3(0.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.5f ,0.25f) },
        { XMFLOAT3(1.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.75f,0.25f) },
        { XMFLOAT3(2.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f ,0.25f) },
                         
        { XMFLOAT3(-2.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f ,0.0f) },
        { XMFLOAT3(-1.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.25f,0.0f) },
        { XMFLOAT3(0.0f,  0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.5f ,0.0f) },
        { XMFLOAT3(1.0f,  0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.75f,0.0f) },
        { XMFLOAT3(2.0f,  0.0f, -2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f ,0.0f) },

    };

    D3D11_BUFFER_DESC plvb;
    ZeroMemory(&plvb, sizeof(plvb));
    plvb.Usage = D3D11_USAGE_DEFAULT;
    plvb.ByteWidth = sizeof(SimpleVertex) * 25;
    plvb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    plvb.CPUAccessFlags = 0;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = planeVertices;

    hr = _pd3dDevice->CreateBuffer(&plvb, &InitData, &_plVertexBuffer);

    if (FAILED(hr))
        return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

    // Create index buffer
    WORD cubeIndices[] =
    {
        0,1,2,
        2,1,3,

        3,1,7,
        7,1,5,

        5,1,4,
        4,1,0,

        0,2,4,
        4,2,6,

        6,2,7,
        7,2,3,

        6,7,4,
        4,7,5,



    };

	D3D11_BUFFER_DESC cib;
	ZeroMemory(&cib, sizeof(cib));

    cib.Usage = D3D11_USAGE_DEFAULT;
    cib.ByteWidth = sizeof(WORD) * 36;
    cib.BindFlags = D3D11_BIND_INDEX_BUFFER;
    cib.CPUAccessFlags = 0; //allow reading of the buffer

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = cubeIndices;
    hr = _pd3dDevice->CreateBuffer(&cib, &InitData, &_cIndexBuffer);

    if (FAILED(hr))
        return hr;

    WORD pyramidIndices[] =
    {
        0,1,2,
        2,1,3,

        0,4,1,
        2,4,0,
        3,4,2,
        1,4,3,



    };

    D3D11_BUFFER_DESC pib;
    ZeroMemory(&pib, sizeof(pib));

    pib.Usage = D3D11_USAGE_DEFAULT;
    pib.ByteWidth = sizeof(WORD) * 18;
    pib.BindFlags = D3D11_BIND_INDEX_BUFFER;
    pib.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = pyramidIndices;
    hr = _pd3dDevice->CreateBuffer(&pib, &InitData, &_pIndexBuffer);

    if (FAILED(hr))
        return hr;
    WORD planeIndices[] =
    {
        //first row
        0,1,5,
        5,1,6,

        1,2,6,
        6,2,7,

        2,3,7,
        7,3,8,

        3,4,8,
        8,4,9,

        //second row
        5,6,10,
        10,6,11,

        6,7,11,
        11,7,12,

        7,8,12,
        12,8,13,

        8,9,13,
        13,9,14,

        //third row
        10,11,15,
        15,11,16,

        11,12,16,
        16,12,17,

        12,13,17,
        17,13,18,

        13,14,18,
        18,14,19,

        //fourth row
        15,16,20,
        20,16,21,

        16,17,21,
        21,17,22,

        17,18,22,
        22,18,23,

        18,19,23,
        23,19,24,
    };

    D3D11_BUFFER_DESC plib;
    ZeroMemory(&plib, sizeof(plib));

    plib.Usage = D3D11_USAGE_DEFAULT;
    plib.ByteWidth = sizeof(WORD) * 96;
    plib.BindFlags = D3D11_BIND_INDEX_BUFFER;
    plib.CPUAccessFlags = 0;

    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = planeIndices;
    hr = _pd3dDevice->CreateBuffer(&plib, &InitData, &_plIndexBuffer);
    if (FAILED(hr))
        return hr;

	return S_OK;
}

void Application::BillboardObject(XMFLOAT4X4* objectWorldMat, XMFLOAT3 objectPos, XMFLOAT3 objectScale, XMFLOAT3 objectForward, XMFLOAT3 objectUp, Camera* camera)
{
    //find the angle for Y rotation
    XMVECTOR objVec = XMVectorSet(objectPos.x, objectPos.y, objectPos.z, 0.0f);
    XMVECTOR eyeVecNoY = XMVectorSet(camera->GetPos().x, objectPos.y, camera->GetPos().z, 0.0f);
    XMVECTOR eyeToObj = eyeVecNoY - objVec;
    XMVECTOR forward = XMVectorSet(objectForward.x, objectForward.y, objectForward.z, 0.0f);
    XMVECTOR angleVec = XMVector3AngleBetweenVectors(eyeToObj, forward);
    float angleY = 0.0f;
    XMStoreFloat(&angleY, angleVec);

    if (camera->GetPos().x < objectPos.x) {
        angleY = -angleY;
    }

    XMStoreFloat4x4(objectWorldMat, XMMatrixScaling(objectScale.x, objectScale.y, objectScale.z) * XMMatrixRotationY(angleY) * XMMatrixTranslation(objectPos.x, objectPos.y, objectPos.z));
    //find angle for axis rotation
    XMVECTOR eyeVec = XMVectorSet(camera->GetPos().x, camera->GetPos().y, camera->GetPos().z, 0.0f);
    eyeToObj = eyeVec - objVec;
    XMVECTOR objUpVec = XMVectorSet(objectUp.x, objectUp.y, objectUp.z, 0.0f);
    angleVec = XMVector3AngleBetweenVectors(eyeToObj, objUpVec);
    float angle = 0.0f;
    XMStoreFloat(&angle, angleVec);

    //find the rotation axis
    XMVECTOR right = XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
    XMMATRIX objWorldXMMAT = XMLoadFloat4x4(objectWorldMat);
    XMVECTOR axisPositionVector = XMVector3Transform(right, objWorldXMMAT);
    XMVECTOR axis =  axisPositionVector- objVec;
    axis = XMVector3Normalize(axis);
    XMFLOAT3 float3Axis;
    XMStoreFloat3(&float3Axis, axis);

    XMStoreFloat4x4(objectWorldMat, XMMatrixScaling(objectScale.x, objectScale.y, objectScale.z) * XMMatrixRotationY(angleY) * XMMatrixRotationAxis(axis, angle)  * XMMatrixTranslation(objectPos.x, objectPos.y, objectPos.z));
}

HRESULT Application::CreateTerrain(ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& indexBuffer, int& triangleCountDest, int rowCount, int columnCount)
{
    int cellRows = rowCount - 1;
    int cellColumns = columnCount - 1;
    int cellCount = cellRows * cellColumns;
    int triangleCount = cellCount * 2;
    triangleCountDest = triangleCount;
    float cellWidth = 1.0f / cellColumns;
    float cellHeight = 1.0f / cellRows;
    int vertexCount = rowCount * columnCount;
    int indexCount = triangleCount * 3;
    std::vector<SimpleVertex> tempVertexList;
    for (int i = 0; i < rowCount; i++) {
        for (int j = 0; j < columnCount; j++) {
            SimpleVertex nextVertex;
            nextVertex.Pos = XMFLOAT3(-0.5f + j * cellWidth, 0.0f, 0.5f - i * cellHeight);
            nextVertex.Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
            nextVertex.TexC = XMFLOAT2(0, 0);
            tempVertexList.push_back(nextVertex);
        }
    }
    D3D11_SUBRESOURCE_DATA InitData;
    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.ByteWidth = sizeof(SimpleVertex) * vertexCount;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    desc.CPUAccessFlags = 0;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &(tempVertexList[0]);
    HRESULT hr = _pd3dDevice->CreateBuffer(&desc, &InitData, &vertexBuffer);

    if (FAILED(hr))
        return hr;

    std::vector<WORD> tempIndexList;
    for (int i = 0; i < cellColumns; i++) {
        for (int j = 0; j < cellRows; j++) {
            tempIndexList.push_back(i * columnCount+j);
            tempIndexList.push_back(i*columnCount+j+1);
            tempIndexList.push_back((i+1)*columnCount+j);

            tempIndexList.push_back((i+1)*columnCount+j);
            tempIndexList.push_back(i*columnCount+j+1);
            tempIndexList.push_back((i+1)*columnCount+j+1);
        }
    }
    D3D11_BUFFER_DESC indexBufferDesc;
    ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(WORD) * indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = &(tempIndexList[0]);
    hr = _pd3dDevice->CreateBuffer(&indexBufferDesc, &InitData, &indexBuffer);

    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW );
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"TutorialWindowClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
    if (!RegisterClassEx(&wcex))
        return E_FAIL;

    // Create window
    _hInst = hInstance;
    RECT rc = {0, 0, 640, 480};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    _hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
                         CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                         nullptr);
    if (!_hWnd)
		return E_FAIL;

    ShowWindow(_hWnd, nCmdShow);

    return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

    if (FAILED(hr))
    {
        if (pErrorBlob != nullptr)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

        if (pErrorBlob) pErrorBlob->Release();

        return hr;
    }

    if (pErrorBlob) pErrorBlob->Release();

    return S_OK;
}

HRESULT Application::InitDevice()
{
    HRESULT hr = S_OK;

    UINT createDeviceFlags = 0;

#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };

    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = _WindowWidth;
    sd.BufferDesc.Height = _WindowHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = _hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        _driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
                                           D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
        if (SUCCEEDED(hr))
            break;
    }

    if (FAILED(hr))
        return hr;

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

    if (FAILED(hr))
        return hr;

    hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
    pBackBuffer->Release();

    if (FAILED(hr))
        return hr;

    //define the stencil/depth buffer
    D3D11_TEXTURE2D_DESC depthStencilDesc;
    depthStencilDesc.Width = _WindowWidth;
    depthStencilDesc.Height = _WindowHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
    _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);




    

    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)_WindowWidth;
    vp.Height = (FLOAT)_WindowHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    _pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();
	InitIndexBuffer();
    CreateTerrain(_terrainVertexBuffer, _terrainIndexBuffer, _terrainTriangleCount, 100, 100);
    

    // Set primitive topology
    _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC cb;
	ZeroMemory(&cb, sizeof(cb));
	cb.Usage = D3D11_USAGE_DEFAULT;
	cb.ByteWidth = sizeof(ConstantBuffer);
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = 0;
    hr = _pd3dDevice->CreateBuffer(&cb, nullptr, &_pConstantBuffer);

    if (FAILED(hr))
        return hr;
    //set the constant buffer
    _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
    _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

    //set light values
    lightDirection = XMFLOAT3(0.25f, 0.5f, -1.0f);
    diffuseMaterial = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
    diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    ambientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    ambientMaterial = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    specularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    specularMaterial = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    specularPower = 10.0f;


    //Create wireframe rasterizer state
    D3D11_RASTERIZER_DESC wfdesc;
    ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
    wfdesc.FillMode = D3D11_FILL_WIREFRAME;
    wfdesc.CullMode = D3D11_CULL_NONE;
    hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

    if (FAILED(hr))
        return hr;
    //Create solid rasterizer state
    D3D11_RASTERIZER_DESC soliddesc;
    ZeroMemory(&soliddesc, sizeof(D3D11_RASTERIZER_DESC));
    soliddesc.FillMode = D3D11_FILL_SOLID;
    soliddesc.CullMode = D3D11_CULL_BACK;
    hr = _pd3dDevice->CreateRasterizerState(&soliddesc, &_rasterizerSolid);

    if (FAILED(hr))
        return hr;
    //set rasterrizer state
    _pImmediateContext->RSSetState(_rasterizerSolid);
    _currentState = 's';

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(rtbd));

    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    hr = _pd3dDevice->CreateBlendState(&blendDesc, &Transparency);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Application::Cleanup()
{
    if (_pImmediateContext) _pImmediateContext->ClearState();

    if (_pConstantBuffer) _pConstantBuffer->Release();
    if (_pVertexBuffer) _pVertexBuffer->Release();
    if (_pIndexBuffer) _pIndexBuffer->Release();
    if (_pVertexLayout) _pVertexLayout->Release();
    if (_pVertexShader) _pVertexShader->Release();
    if (_pPixelShader) _pPixelShader->Release();
    if (_pRenderTargetView) _pRenderTargetView->Release();
    if (_pSwapChain) _pSwapChain->Release();
    if (_pImmediateContext) _pImmediateContext->Release();
    if (_pd3dDevice) _pd3dDevice->Release();
    if (_depthStencilView) _depthStencilView->Release();
    if (_depthStencilBuffer) _depthStencilBuffer->Release();
    if (_wireFrame) _wireFrame->Release();
    if (Transparency) Transparency->Release();
    if (_pTextureRV) _pTextureRV->Release();
    if (_pTextureTree) _pTextureTree->Release();
    if (_terrainVertexBuffer) _terrainVertexBuffer->Release();
    if (_terrainIndexBuffer) _terrainIndexBuffer->Release();
}

void Application::Update()
{
    // Update our time
    static float t = 0.0f;

    if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
    {
        t += (float) XM_PI * 0.0125f;
    }
    else
    {
        static DWORD dwTimeStart = 0;
        DWORD dwTimeCur = GetTickCount();

        if (dwTimeStart == 0)
            dwTimeStart = dwTimeCur;

        t = (dwTimeCur - dwTimeStart) / 1000.0f;
    }
    //send time to constant buffer
    float lastUpdate = updateTime;
    updateTime = t;
    float deltaTime = updateTime - lastUpdate;
    

    //
    // Animate the sun
    //
    
	XMStoreFloat4x4(&_sunWorldPos, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(t*0.5f) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));

    //animate the planets
    XMStoreFloat4x4(&_planet1WorldPos, XMMatrixScaling(0.5f,0.5f,0.5f)* XMMatrixRotationY(-t) * XMMatrixTranslation(4.5f, 0.0f, 0.0f) * XMMatrixRotationY(-t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));
    XMStoreFloat4x4(&_planet2WorldPos, XMMatrixScaling(0.7f, 0.7f, 0.7f) * XMMatrixRotationY(t) * XMMatrixTranslation(8.5f, 0.0f, 0.0f) * XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));

    //animate the moons

    XMStoreFloat4x4(&_moon1WorldPos, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(0.5f * t) * XMMatrixTranslation(1.2f, 0.0f, 0.0f) * XMMatrixRotationY(3 * t) * XMMatrixTranslation(4.5f, 0.0f, 0.0f) * XMMatrixRotationY(-t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));
    XMStoreFloat4x4(&_moon2WorldPos, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(0.5f * t) * XMMatrixTranslation(1.2f, 0.0f, 0.0f) * XMMatrixRotationY(5*t)* XMMatrixTranslation(8.5f, 0.0f, 0.0f) * XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));



    XMVECTOR sunPosVec;
    XMVECTOR sunRotVec;
    XMVECTOR sunScaleVec;
    XMMatrixDecompose(&sunScaleVec, &sunRotVec, &sunPosVec, XMLoadFloat4x4(&_sunWorldPos));
    XMFLOAT3 sunPosFloat3;
    XMStoreFloat3(&sunPosFloat3, sunPosVec);
    cameras[0]->SetAt(sunPosFloat3);
    if (GetAsyncKeyState(0x52) && _RKeyPressed == false) { //changing rasterizer states
        _RKeyPressed = true;
        if (_currentState == 's') {
            _pImmediateContext->RSSetState(_wireFrame);
            _currentState = 'w';
        }
        else if (_currentState == 'w') {
            _pImmediateContext->RSSetState(_rasterizerSolid);
            _currentState = 's';
        }
    }
    else if (!GetAsyncKeyState(0x52)) {
        _RKeyPressed = false;
    }

    if (GetAsyncKeyState(0x31)) {  //changing cameras
        if (currentCameraIndex == 1) {
            currentCameraIndex = 0;
        }
    }
    if (GetAsyncKeyState(0x32)) {
        if (currentCameraIndex == 0) {
            currentCameraIndex = 1;
        }
    }
    if (lerpResult == false && GetAsyncKeyState(0x4C)) {
        int curIndex = cameras[0]->LerpThroughPositions(lerpPositions, deltaTime, 5.0f);
        if (curIndex == lerpPositions.size()) {
            lerpResult = true;
        }

    }
    //update camera
    cameras[currentCameraIndex]->Update(deltaTime);
    XMFLOAT3 planePos = XMFLOAT3(0.0f, -5.0f, 5.0f);
    XMFLOAT3 planeScale = XMFLOAT3(10.0f, 10.0f, 10.0f);
    BillboardObject(&_planeWorldPos, planePos, planeScale, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), cameras[currentCameraIndex]);
    XMStoreFloat4x4(&plane2WorldPos, XMMatrixScaling(10.0f, 1.0f, 10.0f) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));


}

void Application::Draw()
{
    
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	XMMATRIX world = XMLoadFloat4x4(&_sunWorldPos);
	XMMATRIX view = XMLoadFloat4x4(&cameras[currentCameraIndex]->GetView());
	XMMATRIX projection = XMLoadFloat4x4(&cameras[currentCameraIndex]->GetProjection());

    float blendFactor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    _pImmediateContext->OMSetBlendState(0, 0, 0xffffffff);
    //
    // Update variables
    //
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
    cb.DiffuseLight = diffuseLight;
    cb.DiffuseMaterial = diffuseMaterial;
    cb.AmbientLight = ambientLight;
    cb.AmbientMaterial = ambientMaterial;
    cb.SpecularLight = specularLight;
    cb.SpecularMaterial = specularMaterial;
    cb.SpecularPower = specularPower;
    cb.EyePosW = cameras[currentCameraIndex]->GetPos();
    
    cb.LightVecw = lightDirection;
    cb.gTime = updateTime;

	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
    _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

    _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);

    _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
    //change vertex and index buffer to sphere
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &objMeshData.VertexBuffer, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(objMeshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    

    
    
    
    

	//set texture
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureRV);
    //
    // Renders sun
    //
    _pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0);

    
    //draw planet 1
    world = XMLoadFloat4x4(&_planet1WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0);

    //draw planet 2
    world = XMLoadFloat4x4(&_planet2WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    
    _pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0);
    

    //draw moon 1
    world = XMLoadFloat4x4(&_moon1WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0);


    //draw moon 2
    world = XMLoadFloat4x4(&_moon2WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(objMeshData.IndexCount, 0, 0);

    //draw terrain
    _pImmediateContext->IASetVertexBuffers(0, 1, &_terrainVertexBuffer, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_terrainIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    world = XMLoadFloat4x4(&plane2WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(_terrainTriangleCount*3, 0, 0);

    //change vertex and index buffer to plane
    _pImmediateContext->IASetVertexBuffers(0, 1, &_plVertexBuffer, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_plIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //after rendering opaque objects, switch to transparency state
    _pImmediateContext->OMSetBlendState(Transparency, blendFactor, 0xffffffff);

    //draw tree
    _pImmediateContext->PSSetShaderResources(0, 1, &_pTextureTree);
    world = XMLoadFloat4x4(&_planeWorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    
    _pImmediateContext->DrawIndexed(96, 0, 0);



    
    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);



}