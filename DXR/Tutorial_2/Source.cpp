#include <Windows.h>
#include <d3d12.h>
#include <comdef.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>

#include <iostream>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>

#define GLM_FORCE_CTOR_INIT
#include <GLM/glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/glm/gtx/transform.hpp>
#include <GLM/glm/gtx/euler_angles.hpp>

#pragma warning(disable : 4838)
#pragma warning(disable : 4996)

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment (lib,"d3d12.lib")
#pragma comment (lib,"DXGI.lib")


#define WIN_WIDTH                         800
#define WIN_HEIGHT                              600
#define SWAPCHAIN_BUFFER_COUNT            3
#define RENDER_TARGETVIEW_HEAP_SIZE SWAPCHAIN_BUFFER_COUNT

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
      ID3D12DescriptorHeap* pID3D12DescriptorHeap = NULL;
      uint32_t usedEntries = 0;
};
HeapData g_heapData{};

// Frame Objects
struct FrameObjects
{
      ID3D12CommandAllocator* pID3D12CommandAllocator = NULL;
      ID3D12Resource* pID3D12Resource_SwapchainBuffers = NULL;
      D3D12_CPU_DESCRIPTOR_HANDLE cpuDesc_RTVHandle{};
};
FrameObjects g_FrameObjects[SWAPCHAIN_BUFFER_COUNT] = {};

ID3D12GraphicsCommandList4* g_pID3D12GraphicsCommandList4 = NULL;

ID3D12Fence *g_pID3D12Fence = NULL;
HANDLE g_hFenceEvent = NULL;
uint64_t g_iFenceValue = 0;

BOOL g_bEnableDebugging = FALSE;

/* Acceleration Structure */
ID3D12Resource *g_pID3D12Resource_VertexBuffer = NULL;

typedef struct smBottomAcceleratrionStructureBuffers
{
      ID3D12Resource* pID3D12Resource_Scratch = NULL;                         // Scratch Buffer of Acceleration Structure
      ID3D12Resource* pID3D12Resource_Result = NULL;                          // Acceleration Structure Pointer
} BottomAcceleratrionStructureBuffers;

typedef struct smTopLevelAcceleratrionStructureBuffers//: public smBottomAcceleratrionStructureBuffers
{
      ID3D12Resource* pID3D12Resource_Scratch = NULL;                         // Scratch Buffer of Acceleration Structure
      ID3D12Resource* pID3D12Resource_Result = NULL;                          // Acceleration Structure Pointer
      ID3D12Resource* pID3D12Resource_InstanceDesc = NULL;              // Only For Top Level Acceleration Structure, Instancing.
} TopLevelAcceleratrionStructureBuffers;


BottomAcceleratrionStructureBuffers g_BottomLevelAccelerationStructure{};
TopLevelAcceleratrionStructureBuffers g_TopLevelAccelerationStructure{};
uint64_t g_iTLASSize = 0;

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

      hwnd = CreateWindowEx(WS_EX_APPWINDOW, szClassName, TEXT("D3D12 Window"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
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
            //PostMessage(hwnd, WM_KEYDOWN, 0x46, 0);
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
      //case WM_ERASEBKGND:
            //return 0;
            //break;
      case WM_QUIT:
            break;
      case WM_DESTROY:
            //UnInitialize();
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
      HRESULT CreateDescriotorHeap(void);
      //HRESULT Resize(int iWidth, int iHeight);
      HRESULT CreateAccelerationStructure(void);

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
      
      hr = CreateDescriotorHeap();
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : CreateDescriotorHeap() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : CreateDescriotorHeap() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      // Create per-frame objects
      for (uint32_t idx = 0; idx < SWAPCHAIN_BUFFER_COUNT; idx++)
      {
            hr = g_pID3D12Device5->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,__uuidof(g_FrameObjects[idx].pID3D12CommandAllocator),(void**)&g_FrameObjects[idx].pID3D12CommandAllocator);
            if (FAILED(hr))
            {
                  fopen_s(&g_pFile, g_szLogFileName, "a+");
                  fprintf_s(g_pFile, "Initialize : ID3D12Device5->CreateCommandAllocator() FAILED for %d.\n",idx);
                  fclose(g_pFile);
                  return hr;
            }
            else
            {
                  fopen_s(&g_pFile, g_szLogFileName, "a+");
                  fprintf_s(g_pFile, "Initialize : ID3D12Device5->CreateCommandAllocator() SUCCEEDED for %d.\n", idx);
                  fclose(g_pFile);
            }
            
            hr = g_pIDXGISwapChain->GetBuffer(idx, __uuidof(g_FrameObjects[idx].pID3D12Resource_SwapchainBuffers),(void**)&g_FrameObjects[idx].pID3D12Resource_SwapchainBuffers);
            if (FAILED(hr))
            {
                  fopen_s(&g_pFile, g_szLogFileName, "a+");
                  fprintf_s(g_pFile, "Initialize : IDXGISwapChain->GetBuffer() FAILED for %d.\n", idx);
                  fclose(g_pFile);
                  return hr;
            }
            else
            {
                  fopen_s(&g_pFile, g_szLogFileName, "a+");
                  fprintf_s(g_pFile, "Initialize : IDXGISwapChain->GetBuffer() SUCCEEDED for %d.\n", idx);
                  fclose(g_pFile);
            }

            //cpuDesc_RTVHandle
            // g_heapData
            D3D12_RENDER_TARGET_VIEW_DESC renderTargetViewDesc{};
            renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            renderTargetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            renderTargetViewDesc.Texture2D.MipSlice = 0;

            //g_FrameObjects[idx].cpuDesc_RTVHandle
            g_FrameObjects[idx].cpuDesc_RTVHandle = g_heapData.pID3D12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            g_FrameObjects[idx].cpuDesc_RTVHandle.ptr += g_heapData.usedEntries * g_pID3D12Device5->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            g_heapData.usedEntries++;
            g_pID3D12Device5->CreateRenderTargetView(g_FrameObjects[idx].pID3D12Resource_SwapchainBuffers,&renderTargetViewDesc, g_FrameObjects[idx].cpuDesc_RTVHandle);
      }

      hr = g_pID3D12Device5->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_FrameObjects[0].pID3D12CommandAllocator,nullptr,__uuidof(g_pID3D12GraphicsCommandList4),(void**)&g_pID3D12GraphicsCommandList4);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : ID3D12Device5->CreateCommandList() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : ID3D12Device5->CreateCommandList() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      hr = g_pID3D12Device5->CreateFence(0, D3D12_FENCE_FLAG_NONE,__uuidof(g_pID3D12Fence),(void**)&g_pID3D12Fence);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : ID3D12Device5->CreateFence() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : ID3D12Device5->CreateFence() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      g_hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
      if (g_hFenceEvent == NULL)
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : CreateEvent() FAILED.\n");
            fclose(g_pFile); hr = S_FALSE;
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : CreateEvent() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      hr = CreateAccelerationStructure();
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : CreateAccelerationStructure() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "Initialize : CreateAccelerationStructure() SUCCEEDED.\n");
            fclose(g_pFile);
      }



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
      // Begin Frame
      uint32_t iCurrentFrameIndex = g_pIDXGISwapChain->GetCurrentBackBufferIndex();

      D3D12_RESOURCE_BARRIER resourceBarrierBegin = {};
      resourceBarrierBegin.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      resourceBarrierBegin.Transition.pResource = g_FrameObjects[iCurrentFrameIndex].pID3D12Resource_SwapchainBuffers;
      resourceBarrierBegin.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      resourceBarrierBegin.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
      resourceBarrierBegin.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
      g_pID3D12GraphicsCommandList4->ResourceBarrier(1, &resourceBarrierBegin);

      g_pID3D12GraphicsCommandList4->ClearRenderTargetView(g_FrameObjects[iCurrentFrameIndex].cpuDesc_RTVHandle, g_fClearColor,0,NULL);

      // End Frame
      D3D12_RESOURCE_BARRIER resourceBarrierEnd = {};
      resourceBarrierEnd.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      resourceBarrierEnd.Transition.pResource = g_FrameObjects[iCurrentFrameIndex].pID3D12Resource_SwapchainBuffers;
      resourceBarrierEnd.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      resourceBarrierEnd.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
      resourceBarrierEnd.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
      g_pID3D12GraphicsCommandList4->ResourceBarrier(1,&resourceBarrierEnd);

      g_pID3D12GraphicsCommandList4->Close();
      ID3D12CommandList* pGraphicsList = NULL;  
      g_pID3D12GraphicsCommandList4->QueryInterface(__uuidof(pGraphicsList), (void**)&pGraphicsList);
      g_pID3D12CommandQueue->ExecuteCommandLists(1,&pGraphicsList);
      g_iFenceValue++;
      g_pID3D12CommandQueue->Signal(g_pID3D12Fence, g_iFenceValue);
      
      g_pIDXGISwapChain->Present(0, 0);


      if (g_iFenceValue > SWAPCHAIN_BUFFER_COUNT)
      {
            g_pID3D12Fence->SetEventOnCompletion(g_iFenceValue - SWAPCHAIN_BUFFER_COUNT +1,g_hFenceEvent);
            WaitForSingleObject(g_hFenceEvent,INFINITE);
      }

      if (pGraphicsList)
      {
            pGraphicsList->Release();
            pGraphicsList = NULL;
      }

      g_FrameObjects[iCurrentFrameIndex].pID3D12CommandAllocator->Reset();
      g_pID3D12GraphicsCommandList4->Reset(g_FrameObjects[iCurrentFrameIndex].pID3D12CommandAllocator,NULL);

}

void UnInitialize(void)
{
      // wait for command Queue to finish execution.
      if (g_pID3D12CommandQueue && g_pID3D12Fence)
      {
            g_iFenceValue++;
            g_pID3D12CommandQueue->Signal(g_pID3D12Fence, g_iFenceValue);
            g_pID3D12Fence->SetEventOnCompletion(g_iFenceValue, g_hFenceEvent);
            WaitForSingleObject(g_hFenceEvent, INFINITE);

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : Waiting for the workload to complete.\n");
            fclose(g_pFile);
      }

      if (g_pID3D12Fence)
      {
            g_pID3D12Fence->Release();
            g_pID3D12Fence = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12Fence->Release().\n");
            fclose(g_pFile);
      }

      if (g_pID3D12GraphicsCommandList4)
      {
            g_pID3D12GraphicsCommandList4->Release();
            g_pID3D12GraphicsCommandList4 = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12GraphicsCommandList4->Release().\n");
            fclose(g_pFile);
      }

      // top level Acceleration Structure
      if (g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc)
      {
            g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc->Release();
            g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc = NULL;
      }

      if (g_TopLevelAccelerationStructure.pID3D12Resource_Result)
      {
            g_TopLevelAccelerationStructure.pID3D12Resource_Result->Release();
            g_TopLevelAccelerationStructure.pID3D12Resource_Result = NULL;
      }

      if (g_TopLevelAccelerationStructure.pID3D12Resource_Scratch)
      {
            g_TopLevelAccelerationStructure.pID3D12Resource_Scratch->Release();
            g_TopLevelAccelerationStructure.pID3D12Resource_Scratch = NULL;
      }

      // Bottom Level Acceleration Structure
      if (g_BottomLevelAccelerationStructure.pID3D12Resource_Result)
      {
            g_BottomLevelAccelerationStructure.pID3D12Resource_Result->Release();
            g_BottomLevelAccelerationStructure.pID3D12Resource_Result = NULL;
      }

      if (g_BottomLevelAccelerationStructure.pID3D12Resource_Scratch)
      {
            g_BottomLevelAccelerationStructure.pID3D12Resource_Scratch->Release();
            g_BottomLevelAccelerationStructure.pID3D12Resource_Scratch = NULL;
      }

      if (g_pID3D12Resource_VertexBuffer)
      {
            g_pID3D12Resource_VertexBuffer->Release();
            g_pID3D12Resource_VertexBuffer = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12Resource->Release(). Vertex Buffer Released\n");
            fclose(g_pFile);
      }

      if (g_heapData.pID3D12DescriptorHeap)
      {
            g_heapData.pID3D12DescriptorHeap->Release();
            g_heapData.pID3D12DescriptorHeap = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12DescriptorHeap->Release().\n");
            fclose(g_pFile);
      }

      for (int idx = 0; idx < SWAPCHAIN_BUFFER_COUNT; idx++)
      {
            g_FrameObjects[idx].pID3D12CommandAllocator->Release();
            g_FrameObjects[idx].pID3D12CommandAllocator = NULL;
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12CommandAllocator->Release(), Iteration=%d.\n", idx);
            fclose(g_pFile);

            g_FrameObjects[idx].pID3D12Resource_SwapchainBuffers->Release();
            g_FrameObjects[idx].pID3D12Resource_SwapchainBuffers = NULL;
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12Resource_SwapchainBuffers->Release(), Iteration=%d.\n", idx);
            fclose(g_pFile);
      }

      if (g_pIDXGISwapChain)
      {
            g_pIDXGISwapChain->Release();
            g_pIDXGISwapChain = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : IDXGISwapChain->Release().\n");
            fclose(g_pFile);
      }
      
      
      if (g_pID3D12CommandQueue)
      {
            g_pID3D12CommandQueue->Release();
            g_pID3D12CommandQueue = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12CommandQueue->Release().\n");
            fclose(g_pFile);
      }

      if (g_pID3D12Device5)
      {
            g_pID3D12Device5->Release();
            g_pID3D12Device5 = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : ID3D12Device5->Release().\n");
            fclose(g_pFile);
      }

      if (g_pIDXGIFactory)
      {
            g_pIDXGIFactory->Release();
            g_pIDXGIFactory = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : IDXGIFactory->Release().\n");
            fclose(g_pFile);
      }

      if (g_hFenceEvent)
      {
            ResetEvent(g_hFenceEvent);
            CloseHandle(g_hFenceEvent);
            g_hFenceEvent = NULL;

            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "UnInitialize : CloseHandle().\n");
            fclose(g_pFile);
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
                  fprintf_s(g_pFile, "CraeteDevice : D3D12CreateDevice() SUCCEEDED.\nSAM: NA HO PAYI BHAIYA!!\n");
                  fclose(g_pFile);
                  hr = S_FALSE;
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

HRESULT CreateDescriotorHeap(void)
{
      HRESULT hr = S_OK;

      D3D12_DESCRIPTOR_HEAP_DESC desc = {};
      desc.NumDescriptors = RENDER_TARGETVIEW_HEAP_SIZE;
      desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

      hr = g_pID3D12Device5->CreateDescriptorHeap(&desc,__uuidof(g_heapData.pID3D12DescriptorHeap),(void**)&g_heapData.pID3D12DescriptorHeap);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateDescriotorHeap : ID3D12Device5->CreateDescriptorHeap() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateDescriotorHeap : ID3D12Device5->CreateDescriptorHeap() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      return hr;
}

HRESULT CreateAccelerationStructure(void)
{
      HRESULT CreateVertexBuffer(void);
      HRESULT CreateBottomLevelAccelerationStructure(void);
      HRESULT CreateTopLevelAccelerationStructure(void);

      HRESULT hr = S_OK;

      hr = CreateVertexBuffer();
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateAccelerationStructure : CreateVertexBuffer() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateAccelerationStructure : CreateVertexBuffer() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      hr = CreateBottomLevelAccelerationStructure();
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateAccelerationStructure : CreateBottomLevelAccelerationStructure() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateAccelerationStructure : CreateBottomLevelAccelerationStructure() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      hr = CreateTopLevelAccelerationStructure();
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateAccelerationStructure : CreateTopLevelAccelerationStructure() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateAccelerationStructure : CreateTopLevelAccelerationStructure() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      g_pID3D12GraphicsCommandList4->Close();
      ID3D12CommandList* pGraphicsList = NULL;
      g_pID3D12GraphicsCommandList4->QueryInterface(__uuidof(pGraphicsList), (void**)&pGraphicsList);
      g_pID3D12CommandQueue->ExecuteCommandLists(1, &pGraphicsList);
      g_iFenceValue++;
      g_pID3D12CommandQueue->Signal(g_pID3D12Fence, g_iFenceValue);

      g_pID3D12Fence->SetEventOnCompletion(g_iFenceValue, g_hFenceEvent);
      WaitForSingleObject(g_hFenceEvent, INFINITE);
      uint32_t bufferIndex = g_pIDXGISwapChain->GetCurrentBackBufferIndex();
      g_pID3D12GraphicsCommandList4->Reset(g_FrameObjects[0].pID3D12CommandAllocator, nullptr);

      return hr;
}

HRESULT CreateVertexBuffer(void)
{
      HRESULT hr = S_OK;

      const glm::vec3 triangle_position[] =
      {
            glm::vec3(        0,      1, 0.0f),// Apex
            glm::vec3(   1.0f,  -1.0f, 0.0f),// Right-Bottom
            glm::vec3(  -1.0f,  -1.0f, 0.0f),// Left-bottom
      };

      D3D12_RESOURCE_DESC bufferDescription{};
      bufferDescription.Alignment = 0;
      bufferDescription.DepthOrArraySize = 1;
      bufferDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      bufferDescription.Flags = D3D12_RESOURCE_FLAG_NONE;
      bufferDescription.Format = DXGI_FORMAT_UNKNOWN;
      bufferDescription.Height = 1;
      bufferDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      bufferDescription.MipLevels = 1;
      bufferDescription.SampleDesc.Count = 1;
      bufferDescription.SampleDesc.Quality = 0;
      bufferDescription.Width = sizeof(triangle_position);
      //bufferDescription.Width = sizeof(float) * 9;

      D3D12_HEAP_PROPERTIES uploadHeapProperties{};
      uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
      uploadHeapProperties.CPUPageProperty= D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      uploadHeapProperties.MemoryPoolPreference= D3D12_MEMORY_POOL_UNKNOWN;
      uploadHeapProperties.CreationNodeMask=0;
      uploadHeapProperties.VisibleNodeMask=0;

      hr = g_pID3D12Device5->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,&bufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,__uuidof(g_pID3D12Resource_VertexBuffer),(void**)&g_pID3D12Resource_VertexBuffer);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateVertexBuffer : ID3D12Device5->CreateCommittedResource() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateVertexBuffer : ID3D12Device5->CreateCommittedResource() SUCCEEDED.\n");
            fclose(g_pFile);
      }

      uint8_t* pData = NULL;
      hr = g_pID3D12Resource_VertexBuffer->Map(0,NULL,(void**)&pData);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateVertexBuffer : ID3D12Resource->Map() FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateVertexBuffer : ID3D12Resource->Map() SUCCEEDED.\n");
            fclose(g_pFile);
      }
      
      if (pData)
      {
            memcpy(pData, triangle_position, sizeof(triangle_position));
      }
      
      g_pID3D12Resource_VertexBuffer->Unmap(0, NULL);


      return hr;
}

HRESULT CreateBottomLevelAccelerationStructure(void)
      HRESULT hr = S_OK;

      D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc{};
      geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
      geometryDesc.Triangles.VertexBuffer.StartAddress = g_pID3D12Resource_VertexBuffer->GetGPUVirtualAddress();
      geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(glm::vec3);
      geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
      geometryDesc.Triangles.VertexCount = 3;
      geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

      // Requirement for scratch buffer and Assceleration buffer.
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelAccelerationStructureInputs{};
      bottomLevelAccelerationStructureInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
      bottomLevelAccelerationStructureInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
      bottomLevelAccelerationStructureInputs.NumDescs = 1;
      bottomLevelAccelerationStructureInputs.pGeometryDescs = &geometryDesc;
      bottomLevelAccelerationStructureInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;


      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo{};
      g_pID3D12Device5->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelAccelerationStructureInputs,&preBuildInfo);

      // allocate/Create buffers for acceleration structure. make seure to Create them With Unorederd Access Views.
      //g_BottomLevelAccelerationStructure

      // Scratch Buffer
      D3D12_RESOURCE_DESC scratchBufferDescription{};
      scratchBufferDescription.Alignment = 0;
      scratchBufferDescription.DepthOrArraySize = 1;
      scratchBufferDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      scratchBufferDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      scratchBufferDescription.Format = DXGI_FORMAT_UNKNOWN;
      scratchBufferDescription.Height = 1;
      scratchBufferDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      scratchBufferDescription.MipLevels = 1;
      scratchBufferDescription.SampleDesc.Count = 1;
      scratchBufferDescription.SampleDesc.Quality = 0;
      scratchBufferDescription.Width = preBuildInfo.ScratchDataSizeInBytes;
      
      D3D12_HEAP_PROPERTIES defaultHeapProperties{};
      defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
      defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      defaultHeapProperties.CreationNodeMask = 0;
      defaultHeapProperties.VisibleNodeMask = 0;
      
      hr = g_pID3D12Device5->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &scratchBufferDescription, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, __uuidof(g_BottomLevelAccelerationStructure.pID3D12Resource_Scratch), (void**)&g_BottomLevelAccelerationStructure.pID3D12Resource_Scratch);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateBottomLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Scratch Buffer) FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateBottomLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Scratch Buffer) SUCCEEDED.\n");
            fclose(g_pFile);
      }
      // Acceleration Structure
      D3D12_RESOURCE_DESC accelerationStructureBufferDescription{};
      accelerationStructureBufferDescription.Alignment = 0;
      accelerationStructureBufferDescription.DepthOrArraySize = 1;
      accelerationStructureBufferDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      accelerationStructureBufferDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      accelerationStructureBufferDescription.Format = DXGI_FORMAT_UNKNOWN;
      accelerationStructureBufferDescription.Height = 1;
      accelerationStructureBufferDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      accelerationStructureBufferDescription.MipLevels = 1;
      accelerationStructureBufferDescription.SampleDesc.Count = 1;
      accelerationStructureBufferDescription.SampleDesc.Quality = 0;
      accelerationStructureBufferDescription.Width = preBuildInfo.ResultDataMaxSizeInBytes;


      defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
      defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      defaultHeapProperties.CreationNodeMask = 0;
      defaultHeapProperties.VisibleNodeMask = 0;

      hr = g_pID3D12Device5->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &accelerationStructureBufferDescription, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, __uuidof(g_BottomLevelAccelerationStructure.pID3D12Resource_Result), (void**)&g_BottomLevelAccelerationStructure.pID3D12Resource_Result);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateBottomLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Acceleration Structure) FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateBottomLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Acceleration Structure) SUCCEEDED.\n");
            fclose(g_pFile);
      }

      // Build the Bottom Level Acceleration Structure
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC accelerationStructureDesc{};
      accelerationStructureDesc.Inputs = bottomLevelAccelerationStructureInputs;
      accelerationStructureDesc.DestAccelerationStructureData = g_BottomLevelAccelerationStructure.pID3D12Resource_Result->GetGPUVirtualAddress();
      accelerationStructureDesc.ScratchAccelerationStructureData = g_BottomLevelAccelerationStructure.pID3D12Resource_Scratch->GetGPUVirtualAddress();

      g_pID3D12GraphicsCommandList4->BuildRaytracingAccelerationStructure(&accelerationStructureDesc, 0, NULL);

      // Add UAV barrier
      D3D12_RESOURCE_BARRIER uavBarrier = {};
      uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
      uavBarrier.UAV.pResource = g_BottomLevelAccelerationStructure.pID3D12Resource_Result;
      g_pID3D12GraphicsCommandList4->ResourceBarrier(1, &uavBarrier);

      return hr;
}

HRESULT CreateTopLevelAccelerationStructure(void)
{
      HRESULT hr = S_OK;

      // Requirement for scratch buffer and Assceleration buffer.
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelAccelerationStructureInputs{};
      topLevelAccelerationStructureInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
      topLevelAccelerationStructureInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
      topLevelAccelerationStructureInputs.NumDescs = 1;
      topLevelAccelerationStructureInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;


      D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO preBuildInfo{};
      g_pID3D12Device5->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelAccelerationStructureInputs, &preBuildInfo);

      // allocate/Create buffers for acceleration structure. make seure to Create them With Unorederd Access Views.
      //g_TopLevelAccelerationStructure

      // Scratch Buffer
      D3D12_RESOURCE_DESC scratchBufferDescription{};
      scratchBufferDescription.Alignment = 0;
      scratchBufferDescription.DepthOrArraySize = 1;
      scratchBufferDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      scratchBufferDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      scratchBufferDescription.Format = DXGI_FORMAT_UNKNOWN;
      scratchBufferDescription.Height = 1;
      scratchBufferDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      scratchBufferDescription.MipLevels = 1;
      scratchBufferDescription.SampleDesc.Count = 1;
      scratchBufferDescription.SampleDesc.Quality = 0;
      scratchBufferDescription.Width = preBuildInfo.ScratchDataSizeInBytes;

      D3D12_HEAP_PROPERTIES defaultHeapProperties{};
      defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
      defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      defaultHeapProperties.CreationNodeMask = 0;
      defaultHeapProperties.VisibleNodeMask = 0;

      hr = g_pID3D12Device5->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &scratchBufferDescription, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL, __uuidof(g_TopLevelAccelerationStructure.pID3D12Resource_Scratch), (void**)&g_TopLevelAccelerationStructure.pID3D12Resource_Scratch);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateTopLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Scratch Buffer) FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateTopLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Scratch Buffer) SUCCEEDED.\n");
            fclose(g_pFile);
      }
      // Acceleration Structure
      D3D12_RESOURCE_DESC accelerationStructureBufferDescription{};
      accelerationStructureBufferDescription.Alignment = 0;
      accelerationStructureBufferDescription.DepthOrArraySize = 1;
      accelerationStructureBufferDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      accelerationStructureBufferDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
      accelerationStructureBufferDescription.Format = DXGI_FORMAT_UNKNOWN;
      accelerationStructureBufferDescription.Height = 1;
      accelerationStructureBufferDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      accelerationStructureBufferDescription.MipLevels = 1;
      accelerationStructureBufferDescription.SampleDesc.Count = 1;
      accelerationStructureBufferDescription.SampleDesc.Quality = 0;
      accelerationStructureBufferDescription.Width = preBuildInfo.ResultDataMaxSizeInBytes;
      g_iTLASSize = preBuildInfo.ResultDataMaxSizeInBytes;


      defaultHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
      defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      defaultHeapProperties.CreationNodeMask = 0;
      defaultHeapProperties.VisibleNodeMask = 0;

      hr = g_pID3D12Device5->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &accelerationStructureBufferDescription, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL, __uuidof(g_TopLevelAccelerationStructure.pID3D12Resource_Result), (void**)&g_TopLevelAccelerationStructure.pID3D12Resource_Result);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateTopLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Acceleration Structure) FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateTopLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Acceleration Structure) SUCCEEDED.\n");
            fclose(g_pFile);
      }
      

      // Instance Buffer
      D3D12_RESOURCE_DESC instanceBufferDescription{};
      instanceBufferDescription.Alignment = 0;
      instanceBufferDescription.DepthOrArraySize = 1;
      instanceBufferDescription.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      instanceBufferDescription.Flags = D3D12_RESOURCE_FLAG_NONE;
      instanceBufferDescription.Format = DXGI_FORMAT_UNKNOWN;
      instanceBufferDescription.Height = 1;
      instanceBufferDescription.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      instanceBufferDescription.MipLevels = 1;
      instanceBufferDescription.SampleDesc.Count = 1;
      instanceBufferDescription.SampleDesc.Quality = 0;
      instanceBufferDescription.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC);


      defaultHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
      defaultHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      defaultHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      defaultHeapProperties.CreationNodeMask = 0;
      defaultHeapProperties.VisibleNodeMask = 0;

      hr = g_pID3D12Device5->CreateCommittedResource(&defaultHeapProperties, D3D12_HEAP_FLAG_NONE, &instanceBufferDescription, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, __uuidof(g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc), (void**)&g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateTopLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Instance Buffer) FAILED.\n");
            fclose(g_pFile);
            return hr;
      }
      else
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateTopLevelAccelerationStructure : ID3D12Device5->CreateCommittedResource(Instance Buffer) SUCCEEDED.\n");
            fclose(g_pFile);
      }

      D3D12_RAYTRACING_INSTANCE_DESC* pInstanceDesc=NULL;
      g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc->Map(0, NULL, (void**)&pInstanceDesc);
      if (FAILED(hr))
      {
            fopen_s(&g_pFile, g_szLogFileName, "a+");
            fprintf_s(g_pFile, "CreateTopLevelAccelerationStructure : ID3D12Resource->Map(Instance Buffer) FAILED.\n");
            fclose(g_pFile);
            return hr;
      }

      if (pInstanceDesc)
      {
            pInstanceDesc->InstanceID = 0; //This value will be accessible to the shader via InstanceID()
            pInstanceDesc->InstanceContributionToHitGroupIndex = 0; // Offset of Shader Binding table!
            pInstanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
            glm::mat4 identityMatrix;
            memcpy(pInstanceDesc->Transform, &identityMatrix, sizeof(pInstanceDesc->Transform));
            pInstanceDesc->AccelerationStructure = g_BottomLevelAccelerationStructure.pID3D12Resource_Result->GetGPUVirtualAddress();
            pInstanceDesc->InstanceMask = 0xFF;
      }
      g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc->Unmap(0, NULL);

      // Build the Top Level Acceleration Structure
      D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC accelerationStructureDesc{};
      accelerationStructureDesc.Inputs = topLevelAccelerationStructureInputs;
      accelerationStructureDesc.Inputs.InstanceDescs = g_TopLevelAccelerationStructure.pID3D12Resource_InstanceDesc->GetGPUVirtualAddress();
      accelerationStructureDesc.DestAccelerationStructureData = g_TopLevelAccelerationStructure.pID3D12Resource_Result->GetGPUVirtualAddress();
      accelerationStructureDesc.ScratchAccelerationStructureData = g_TopLevelAccelerationStructure.pID3D12Resource_Scratch->GetGPUVirtualAddress();

      g_pID3D12GraphicsCommandList4->BuildRaytracingAccelerationStructure(&accelerationStructureDesc, 0, NULL);

      // Add UAV barrier
      D3D12_RESOURCE_BARRIER uavBarrier = {};
      uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
      uavBarrier.UAV.pResource = g_TopLevelAccelerationStructure.pID3D12Resource_Result;
      g_pID3D12GraphicsCommandList4->ResourceBarrier(1, &uavBarrier);

      return hr;
}
