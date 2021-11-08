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

void Application::CalculateVertexNormals(ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer, int vertexArraySize, int indicesArraySize)
{
    /*
    // take buffers and prepare them for reading/writing
    D3D11_MAPPED_SUBRESOURCE vResource;
    _pImmediateContext->Map(vertexBuffer, 0, D3D11_MAP_READ_WRITE, 0, &vResource); //read write performance ok as this is only run once
    SimpleVertex* vertexArray = new SimpleVertex[vertexArraySize]; //new vertex array to be copied into the old one
    memcpy(vertexArray, vResource.pData, vertexArraySize * sizeof(SimpleVertex));

    D3D11_MAPPED_SUBRESOURCE iResource;
    _pImmediateContext->Map(indexBuffer, 0, D3D11_MAP_READ, 0, &iResource);
    WORD* indicesArray = new WORD[indicesArraySize]; 
    memcpy(indicesArray, iResource.pData, indicesArraySize * sizeof(WORD));

    int numberOfTriangles = (int)(indicesArraySize / 3);
    XMVECTOR *surfaceNormals = new XMVECTOR[numberOfTriangles];
    for (int i = 0; i < indicesArraySize; i++) {
        if (i % 3 == 0) {
            XMFLOAT3 vertex1 = vertexArray[(int)indicesArray[i]].Pos;
            XMFLOAT3 vertex2 = vertexArray[(int)indicesArray[i+1]].Pos; //take 3 vertices on the same triangle
            XMFLOAT3 vertex3 = vertexArray[(int)indicesArray[i + 2]].Pos;

            XMVECTOR posVector1 = XMLoadFloat3(&vertex1);
            XMVECTOR posVector2 = XMLoadFloat3(&vertex2); //turn them into pos vectors
            XMVECTOR posVector3 = XMLoadFloat3(&vertex3);

            XMVECTOR uVector = -posVector1 + posVector2; //from 1 to 2
            XMVECTOR vVector = -posVector2 + posVector3; //from 2 to 3

            XMVECTOR normal = XMVector3Cross(uVector, vVector);
            normal = XMVector3Normalize(normal);
            surfaceNormals[(int)(i / 3)] = normal;
        }
    }
    for (int i = 0; i < vertexArraySize; i++) { // for each vertex
        std::vector<int> triangleAppearances;
        std::vector<int> surfaceNormalArrayPositions;
        for (int j = 0; j < indicesArraySize; j++) {
            if ((int)(indicesArray[j]) == i) {
                triangleAppearances.push_back(j); //find every triangle which contains vertex
                if (j % 3 == 0) {
                    surfaceNormalArrayPositions.push_back((int)(j/3));
                }
                else if (j % 3 == 1) {
                    surfaceNormalArrayPositions.push_back((int)((j-1) / 3));
                }
                else if (j % 3 == 2) {
                    surfaceNormalArrayPositions.push_back((int)((j - 2) / 3));
                }
            }
        }
        XMFLOAT3 zero = XMFLOAT3(0, 0, 0);
        XMVECTOR totalNormals = XMLoadFloat3(&zero);
        for (int j = 0; j < triangleAppearances.size(); j++) { //for each time the vertex appears in a triangle
            totalNormals += surfaceNormals[surfaceNormalArrayPositions.at(j)];
        }
        XMVECTOR average = XMVector3Normalize(totalNormals / triangleAppearances.size());
        XMStoreFloat3(&vertexArray[i].Normal, average);
    }
    delete[] surfaceNormals;
    //copy new vertexArray into vertex Buffer
    _pImmediateContext->Unmap(vertexBuffer, 0);
    _pImmediateContext->Unmap(indexBuffer, 0);
    */
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

    // Initialize the view matrix
	XMVECTOR Eye = XMVectorSet(0.0f, 7.0f, -3.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 7.5f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

    // Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT) _WindowHeight, 0.01f, 100.0f));

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
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
        { XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3( -1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },

        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f),XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
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
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f),XMFLOAT3(0.0f, 0.0f, 0.0f) },

        { XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },

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
        { XMFLOAT3(-2.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(2.0f, 0.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },

        { XMFLOAT3(-2.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(0.0f,  0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(2.0f,  0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },

        { XMFLOAT3(-2.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(0.0f,  0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(2.0f,  0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
                         
        { XMFLOAT3(-2.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(0.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(2.0f,  0.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
                         
        { XMFLOAT3(-2.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, 0.0f, -2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(0.0f,  0.0f, -2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1.0f,  0.0f, -2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(2.0f,  0.0f, -2.0f), XMFLOAT3(0.0f, 0.0f, 0.0f) },

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

    // Set vertex buffer
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

	InitIndexBuffer();

    // Set index buffer
    _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

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
    updateTime = t;
    //
    // Animate the sun
    //

	XMStoreFloat4x4(&_sunWorldPos, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(t*0.5f) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));

    //animate the planets
    XMStoreFloat4x4(&_planet1WorldPos, XMMatrixScaling(0.5f,0.5f,0.5f)* XMMatrixRotationY(1.2f*t) * XMMatrixTranslation(4.5f, 0.0f, 0.0f) * XMMatrixRotationY(-t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));
    XMStoreFloat4x4(&_planet2WorldPos, XMMatrixScaling(0.7f, 0.7f, 0.7f) * XMMatrixRotationY(t) * XMMatrixTranslation(8.5f, 0.0f, 0.0f) * XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));

    //animate the moons

    XMStoreFloat4x4(&_moon1WorldPos, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(0.5f * t) * XMMatrixTranslation(1.2f, 0.0f, 0.0f) * XMMatrixRotationY(3 * t) * XMMatrixTranslation(4.5f, 0.0f, 0.0f) * XMMatrixRotationY(-t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));
    XMStoreFloat4x4(&_moon2WorldPos, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(0.5f * t) * XMMatrixTranslation(1.2f, 0.0f, 0.0f) * XMMatrixRotationY(5*t)* XMMatrixTranslation(8.5f, 0.0f, 0.0f) * XMMatrixRotationY(t) * XMMatrixTranslation(0.0f, 0.0f, 7.5f));

    //place the plane
    XMStoreFloat4x4(&_planeWorldPos, XMMatrixScaling(10.0f, 10.0f, 10.0f) * XMMatrixTranslation(0.0f, -3.0f, 7.5f));

    if (GetAsyncKeyState(0x52) && _RKeyPressed == false) {
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



}

void Application::Draw()
{
    //
    // Clear the back buffer
    //
    float ClearColor[4] = {0.0f, 0.125f, 0.3f, 1.0f}; // red,green,blue,alpha
    _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);


    //change vertex and index buffer to pyramid
    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    _pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);


	XMMATRIX world = XMLoadFloat4x4(&_sunWorldPos);
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);
    //
    // Update variables
    //
    ConstantBuffer cb;
	cb.mWorld = XMMatrixTranspose(world);
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
    cb.gTime = updateTime;

	_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
    _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
    _pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);

    //
    // Renders sun
    //

	_pImmediateContext->DrawIndexed(18, 0, 0);        

    //change vertex and index buffer to cube
    _pImmediateContext->IASetVertexBuffers(0, 1, &_cVertexBuffer, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_cIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //draw planet 1
    world = XMLoadFloat4x4(&_planet1WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);

    //draw planet 2
    world = XMLoadFloat4x4(&_planet2WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);
    

    //draw moon 1
    world = XMLoadFloat4x4(&_moon1WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);


    //draw moon 2
    world = XMLoadFloat4x4(&_moon2WorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(36, 0, 0);

    //change vertex and index buffer to plane
    _pImmediateContext->IASetVertexBuffers(0, 1, &_plVertexBuffer, &stride, &offset);
    _pImmediateContext->IASetIndexBuffer(_plIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    //draw plane
    world = XMLoadFloat4x4(&_planeWorldPos);
    cb.mWorld = XMMatrixTranspose(world);
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
    _pImmediateContext->DrawIndexed(96, 0, 0);

    _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    //
    // Present our back buffer to our front buffer
    //
    _pSwapChain->Present(0, 0);



}