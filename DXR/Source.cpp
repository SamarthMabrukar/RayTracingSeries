#include <Windows.h>
#include <d3d12.h>
#include <comdef.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>

#include <iostream>
#include <vector>

#pragma warning(disable : 4838)
#pragma warning(disable : 4996)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment (lib,"d3d12.lib")
#pragma comment (lib,"DXGI.lib")


#define WIN_WIDTH					800
#define WIN_HEIGHT					600
#define SWAPCHAIN_BUFFER_COUNT		3
#define RENDER_TARGETVIEW_HEAP_SIZE 3

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

FILE* g_pFile = NULL;
char g_szLogFileName[] = "sam_d3d12_LogFile.txt";

HWND g_hWnd = NULL;

// for Fullscreen
DWORD g_dwStyle = 0;
WINDOWPLACEMENT wpPrev = { sizeof(wpPrev) };

bool g_bActiveWindow = false;
bool g_bEscapePressed = false;
bool g_bFullScreen = false;

float g_fClearColor[4] = { 0.39f, 0.58f, 0.93f, 1.0f }; // fill background with this color

IDXGIFactory4* g_pIDXGIFactory = NULL;
ID3D12Device5* g_pID3D12Device5 = NULL;
ID3D12CommandQueue* g_pID3D12CommandQueue = NULL;
IDXGISwapChain3* g_pIDXGISwapChain = NULL;


// Heap data
struct HeapData
{
	ID3D12DescriptorHeap* pID3D12DescriptorHeap;
	uint32_t usedEntries = 0;
};
HeapData g_heapData{};

BOOL g_bEnableDebugging = FALSE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{

	HRESULT Initialize(void);
	void UnInitialize(void);
	void Update(void);
	void Render(void);

	WNDCLASSEX wndclass;
	HWND hwnd = NULL;
	TCHAR szClassName[] = TEXT("SamD3D12");
	RECT rc;
	MSG msg;
	bool bDone = false;
	HRESULT hr = NULL;

	if (fopen_s(&g_pFile, g_szLogFileName, "w+") != 0)
	{
		MessageBox(NULL, TEXT("Could Not Open File"), TEXT("Error..!!"), MB_OK);
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf_s(g_pFile, "Log File Opened\n");
		fclose(g_pFile);
	}

	SecureZeroMemory((void*)&wndclass, sizeof(wndclass));
	wndclass.cbSize = sizeof(wndclass);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(hInstance, IDC_ARROW);

	if (!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL, TEXT("Could Not RegisterClassEx()"), TEXT("Error..!!"), MB_OK);
		exit(EXIT_FAILURE);
	}

	SecureZeroMemory((void*)&rc, sizeof(rc));
	rc.right = WIN_WIDTH;
	rc.bottom = WIN_HEIGHT;

	AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szClassName, TEXT("D3D11 Orthographic Window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	if (hwnd == NULL)
	{
		MessageBox(NULL, TEXT("Could Not CreateWindowEx()"), TEXT("Error..!!"), MB_OK);
		exit(EXIT_FAILURE);
	}
	g_hWnd = hwnd;

	ShowWindow(hwnd, SW_NORMAL);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	hr = Initialize();
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize Failed.\nLeaving Now...!!!\n");
		fclose(g_pFile);
		UnInitialize();
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize Completed...!!!\n");
		fclose(g_pFile);
	}

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (g_bActiveWindow)
			{
				if (g_bEscapePressed)
				{
					bDone = true;
				}

				Update();
				Render();
			}
		}
	}

	UnInitialize();
	return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void ToggleFullScreen();
	void UnInitialize(void);
	HRESULT Resize(int iWidth, int iHeight);

	HRESULT hr = S_OK;
	switch (iMsg)
	{
	case WM_CREATE:
		PostMessage(hwnd, WM_KEYDOWN, 0x46, 0);
		break;
	case WM_SIZE:
		break;
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
		{
			g_bActiveWindow = true;
		}
		else
		{
			g_bActiveWindow = false;
		}
		break;

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:
			g_bEscapePressed = true;
			DestroyWindow(hwnd);
			break;

		case 0x46: // f or F
			ToggleFullScreen();
			break;
		}
		break;
	case WM_ERASEBKGND:
		//return 0;
		break;
	case WM_QUIT:
		break;
	case WM_DESTROY:
		UnInitialize();
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen()
{
	MONITORINFO mi = { sizeof(mi) };
	if (g_bFullScreen == false)
	{
		g_dwStyle = GetWindowLong(g_hWnd, GWL_STYLE);
		if (g_dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(g_hWnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(g_hWnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(g_hWnd, GWL_STYLE, g_dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(g_hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, (mi.rcMonitor.right - mi.rcMonitor.left), (mi.rcMonitor.bottom - mi.rcMonitor.top), SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			ShowCursor(FALSE);
		}
		g_bFullScreen = true;
	}
	else
	{
		SetWindowLong(g_hWnd, GWL_STYLE, g_dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(g_hWnd, &wpPrev);
		SetWindowPos(g_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOOWNERZORDER);
		ShowCursor(TRUE);
		g_bFullScreen = false;
	}
}

HRESULT Initialize(void)
{
	HRESULT CraeteDevice(void);
	HRESULT CreateCommandQueue(void);
	HRESULT CreateSwapchain(void);
	HRESULT Resize(int iWidth, int iHeight);
	HRESULT hr = S_OK;

	hr = CraeteDevice();
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize : CraeteDevice() FAILED.\n");
		fclose(g_pFile);
		return hr;
	}
	else
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize : CraeteDevice() SUCCEEDED.\n");
		fclose(g_pFile);
	}

	hr = CreateCommandQueue();
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize : CreateCommandQueue() FAILED.\n");
		fclose(g_pFile);
		return hr;
	}
	else
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize : CreateCommandQueue() SUCCEEDED.\n");
		fclose(g_pFile);
	}

	hr = CreateSwapchain();
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize : CreateSwapchain() FAILED.\n");
		fclose(g_pFile);
		return hr;
	}
	else
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "Initialize : CreateSwapchain() SUCCEEDED.\n");
		fclose(g_pFile);
	}
	
	// Create a RTV descriptor heap

	return hr;
}

HRESULT Resize(int iWidth, int iHeight)
{
	HRESULT hr = S_OK;
	
	return hr;
}

void Update(void)
{
}

void Render(void)
{
	
}

void UnInitialize(void)
{
	
	if (g_pID3D12CommandQueue)
	{
		g_pID3D12CommandQueue->Release();
		g_pID3D12CommandQueue = NULL;
	}

	if (g_pID3D12Device5)
	{
		g_pID3D12Device5->Release();
		g_pID3D12Device5 = NULL;
	}

	if (g_pIDXGIFactory)
	{
		g_pIDXGIFactory->Release();
		g_pIDXGIFactory = NULL;
	}

	if (g_pFile)
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "UnInitialize Completed.\n");
		fprintf_s(g_pFile, "Log File Closed.\n");
		fclose(g_pFile);
		g_pFile = NULL;
	}
}

HRESULT CraeteDevice(void)
{
	HRESULT hr = S_OK;

	IDXGIAdapter* pAdapter = NULL;
	IDXGIOutput* pOutput = NULL;
	IDXGIAdapter* pIDXGIAdapter_desiredAdapter = NULL;

	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&g_pIDXGIFactory);
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CraeteDevice : CreateDXGIFactory() FAILED.\n");
		fclose(g_pFile);

		return hr;
	}

	size_t bestVideoMemory = 0;
	BOOL bRayTracingSupported = FALSE;

	// Collect all Adapters and pick-up with the heighest VRAM
	for (unsigned int i = 0; g_pIDXGIFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		DXGI_ADAPTER_DESC desc{};
		pAdapter->GetDesc(&desc);

		// TO DO : Print Device Information

		if ((desc.DedicatedVideoMemory!=0) && (bestVideoMemory < desc.DedicatedVideoMemory))
		{
			bestVideoMemory = desc.DedicatedVideoMemory;
			pIDXGIAdapter_desiredAdapter = pAdapter;
		}
	}


	// Craete Device and Look for Ray TracingTire
	hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(g_pID3D12Device5), (void**)&g_pID3D12Device5);
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CraeteDevice : D3D12CreateDevice() FAILED.\n");
		fclose(g_pFile);
		return hr;
	}
	else
	{
		// Check Capabilities
		D3D12_FEATURE_DATA_D3D12_OPTIONS5 rayTracingFeature{};
		hr = g_pID3D12Device5->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &rayTracingFeature, sizeof(rayTracingFeature));
		if (SUCCEEDED(hr))
		{
			if (rayTracingFeature.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
			{
				fopen_s(&g_pFile, g_szLogFileName, "a+");
				fprintf_s(g_pFile, "CraeteDevice : D3D12CreateDevice() SUCCEEDED.\n");
				fprintf_s(g_pFile, "CraeteDevice : SAM: Congratulations you Have Ray Tracing Support!\n");
				fclose(g_pFile);
			}
			else
			{
				fopen_s(&g_pFile, g_szLogFileName, "a+");
				fprintf_s(g_pFile, "CraeteDevice : D3D12CreateDevice() SUCCEEDED. You DO NOT have Support for Ray Tracing\n");
				fclose(g_pFile);
				hr = S_FALSE;
			}
		}
	}

	return hr;
}

HRESULT CreateCommandQueue(void)
{
	HRESULT hr = S_OK;
	D3D12_COMMAND_QUEUE_DESC cqDesc{};
	cqDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	hr = g_pID3D12Device5->CreateCommandQueue(&cqDesc,__uuidof(g_pID3D12CommandQueue),(void**)&g_pID3D12CommandQueue);
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CreateCommandQueue : ID3D12Device5->CreateCommandQueue() FAILED.\n");
		fclose(g_pFile);
		return hr;
	}
	else
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CreateCommandQueue : ID3D12Device5->CreateCommandQueue() SUCCEEDED.\n");
		fclose(g_pFile);
	}

	return hr;
}

HRESULT CreateSwapchain(void)
{
	HRESULT hr = S_OK;

	IDXGISwapChain1* pIDXGISwapChain1 = NULL;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = SWAPCHAIN_BUFFER_COUNT;
	swapChainDesc.Width = WIN_WIDTH;
	swapChainDesc.Height = WIN_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Fixed format
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	hr = g_pIDXGIFactory->CreateSwapChainForHwnd(g_pID3D12CommandQueue,g_hWnd,&swapChainDesc,NULL,NULL,&pIDXGISwapChain1);
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CreateSwapchain : IDXGIFactory->CreateSwapChainForHwnd() FAILED.\n");
		fclose(g_pFile);
		return hr;
	}
	else
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CreateSwapchain : IDXGIFactory->CreateSwapChainForHwnd() SUCCEEDED.\n");
		fclose(g_pFile);
	}

	hr = pIDXGISwapChain1->QueryInterface(__uuidof(g_pIDXGISwapChain),(void**)&g_pIDXGISwapChain);
	if (FAILED(hr))
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CreateSwapchain : IDXGISwapChain1->QueryInterface() FAILED.\n");
		fclose(g_pFile);
		return hr;
	}
	else
	{
		fopen_s(&g_pFile, g_szLogFileName, "a+");
		fprintf_s(g_pFile, "CreateSwapchain : IDXGISwapChain1->QueryInterface() SUCCEEDED.\n");
		fclose(g_pFile);
	}

	return hr;
}
