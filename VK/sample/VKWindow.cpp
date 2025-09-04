// Header Files
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
// Vulkan related header files
// Needs to define a macro to direct the OS(environment) we are in.
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>

// glm related macro and header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// Macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define CHECKER_BOARD_WIDTH 64
#define CHECKER_BOARD_HEIGHT 64

// Vulkan Related Libraries
#pragma comment(lib, "vulkan-1.lib")
#pragma warning(disable : 4996)

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global Variables
HWND g_hwnd = 0;
BOOL g_bFullScreen = FALSE;
BOOL g_bWindowMinimized = FALSE;
BOOL g_bEscapeKeyPressed = FALSE;
BOOL g_bActiveWindow = FALSE;
FILE* g_pFile = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev;
MONITORINFO mi;


const char* g_pszAppName = "ARTR";

// Vulkan replatedGlobal Variables.
// Instance Extensio related Variables.
uint32_t g_iEnaledInstanceExtensionCount = 0;
// VK_KHR_SURFACE_EXTENSION_NAME and VK_KHR_WIN32_SURFACE_EXTENSION_NAME and VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* g_pchEnableInstanceExtensionNames_array[3];

// Vulkan Instance
VkInstance g_vkInstance = VK_NULL_HANDLE;

// Presentation Surface
VkSurfaceKHR g_vkSurfaceKHR = VK_NULL_HANDLE;

// Physical Device and Queue Families
uint32_t g_iPhysicalDeviceCount = 0;
VkPhysicalDevice* g_pvkPhysicalDevice_array = NULL;
VkPhysicalDevice g_vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t g_iGraphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties; // Staging buffer and Non-Staging buffer

// Device Extensions
// VK_KHR_SWAPCHAIN_EXTENSION_NAME
uint32_t g_iEnabledDeviceExtensionCount = 0;
const char* g_pchEnableDeviceExtensionNames_array[1];

// Logical Device
VkDevice g_vkDevice = VK_NULL_HANDLE;
VkQueue g_vkQueue = VK_NULL_HANDLE;

// Swapchain
// ColorFormat and ColorSpace
VkFormat g_vkFormat_Color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR g_vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
// PresentationMode
VkPresentModeKHR g_vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
// Swapchain
uint32_t g_iWinWidth = WIN_WIDTH;
uint32_t g_iWinHeight = WIN_HEIGHT;
VkSwapchainKHR g_vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D g_vkExtent2D_swapchain;

// Swapchain Images and Swapchain Image Views
uint32_t g_iSwapchainImageCount = UINT32_MAX; // Desired Image Count.
VkImage* g_SwapchainImage_array = NULL;
VkImageView* g_SwapchainImageView_array = NULL;

// For Depth Image
VkFormat g_vkFormat_Depth = VK_FORMAT_UNDEFINED;
VkImage g_vkImage_Depth = VK_NULL_HANDLE;
VkDeviceMemory g_vkDeviceMemory_Depth = VK_NULL_HANDLE;
VkImageView g_vkImageView_Depth = VK_NULL_HANDLE;

// Command Pool
VkCommandPool g_vkCommandPool = VK_NULL_HANDLE;

// Command Buffer
VkCommandBuffer* g_vkCommandBuffer_array = NULL;

// Renderpass and Subpass
VkRenderPass g_vkRenderPass = VK_NULL_HANDLE;

// Framebuffers
VkFramebuffer* g_vkFramebuffer_array = NULL;

// Fences
VkFence* g_pvkFence_array = NULL;

// Semaphore
VkSemaphore g_VkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore g_VkSemaphore_renderComplete = VK_NULL_HANDLE;

// Build Command Buffer
VkClearColorValue g_vkClearColorValue; // Clear color
VkClearDepthStencilValue g_vkClearDepthStencilValue;

// Render
BOOL g_bInitialized = FALSE;
uint32_t g_iCurrentImageIndex = UINT32_MAX;

// Validation
BOOL g_bEnableValidation = TRUE;

uint32_t g_iEnaledInstanceLayerCount = 0;
// VK_LAYER_KHRONOS_VALIDATION
const char* g_pchEnableInstanceLayerNames_array[1]; // For VK_LAYER_KHRONOS_validation

VkDebugReportCallbackEXT g_vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT g_vkDestroyDebugReportCallbackEXT_fnptr = VK_NULL_HANDLE;

// Pipeline
VkViewport g_vkViewport;
VkRect2D g_vkRect2D_scissor;
VkPipeline g_vkPipeline = VK_NULL_HANDLE;


// Vertex Buffer
typedef struct myVertexData
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
}VertexData;

VertexData g_VertexData_position;
VertexData g_VertexData_texcoord;

// Shaders
VkShaderModule g_vkShaderModule_Vertex_Shader = VK_NULL_HANDLE;
VkShaderModule g_vkShaderModule_Fragment_Shader = VK_NULL_HANDLE;

// DescriptorSetLayout
// SAM 1. Define descriptor set layouts for each set
std::vector<VkDescriptorSetLayout> descriptorSetLayouts; // SAM

// Pipeline Layout
VkPipelineLayout g_vkPipelineLayout = VK_NULL_HANDLE;

// Descriptor Pool
VkDescriptorPool g_vkDescriptorPool = VK_NULL_HANDLE;

// Descriptor Set
std::vector<VkDescriptorSet> descriptorSets(3);//SAM

// Uniform
typedef struct
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
}UniformBuffer;

UniformBuffer g_UniformData_straight;
UniformBuffer g_UniformData_straight_model;

typedef struct
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
}myUniformData;

//glm::mat4 modelMatrix;
typedef struct
{
	glm::mat4 modelMatrix;
}myModelMatData;

// For Checker Board
BYTE g_bytCheckerBoard[CHECKER_BOARD_HEIGHT][CHECKER_BOARD_WIDTH][4];
// Texture Related Code Start
VkImage g_vkImage_texture = VK_NULL_HANDLE;
VkDeviceMemory g_vkDeviceMemory_texture = VK_NULL_HANDLE;
VkImageView g_vkImageView_texture = VK_NULL_HANDLE;
VkSampler g_vkSampler_texture = VK_NULL_HANDLE;
// Texture Related Code End

// Entry Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// Function Declarations
	VkResult Initialize(void);
	VkResult Display(void);
	void Update(void);
	void UnInitialize(void);

	// Variable Declarations
	WNDCLASSEX wndclass;
	MSG msg;
	HWND hwnd = NULL;
	TCHAR szClassName[255];
	RECT windowRect;

	// Game Loop Control
	BOOL bDone = FALSE;

	// Initialization Status
	VkResult vkResult = VK_SUCCESS;

	// Set-up file
	g_pFile = fopen("SamLogFile.txt", "w+");
	if (g_pFile == NULL)
	{
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could not open Log File"), MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(g_pFile, "WinMain -> File Opened Successfully. \n");
	}

	wsprintf(szClassName, TEXT("%s"), g_pszAppName);

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
	wndclass.hCursor = LoadCursor(hInstance, IDC_ARROW);

	if (!RegisterClassEx(&wndclass))
	{
		fprintf(g_pFile, "Could Not RegisterClass().\n");
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could Not RegisterClass() "), MB_OK | MB_ICONERROR);
		UnInitialize();
		exit(EXIT_FAILURE);
	}

	// Adjust the required size, including non-client area.
	SecureZeroMemory((void*)&windowRect, sizeof(windowRect));
	windowRect.left = 0;
	windowRect.top = 0;
	windowRect.bottom = WIN_HEIGHT;
	windowRect.right = WIN_WIDTH;
	AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW);
	// System Parameter Info
	hwnd = CreateWindowEx(WS_EX_APPWINDOW, szClassName,
		TEXT("Samarth Mabrukarr : Vulkan"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(GetSystemMetrics(SM_CXSCREEN) - (windowRect.right - windowRect.left)) / 2,
		(GetSystemMetrics(SM_CYSCREEN) - (windowRect.bottom - windowRect.top)) / 2,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL, NULL, hInstance, NULL);

	if (hwnd == NULL)
	{
		fprintf(g_pFile, "Could Not CreateWindowEx().\n");
		MessageBox(NULL, TEXT("Issue...!!!"), TEXT("Could Not CreateWindow() "), MB_OK | MB_ICONERROR);
		UnInitialize();
		exit(EXIT_FAILURE);
	}

	g_hwnd = hwnd;

	// Initialization
	vkResult = Initialize();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "WinMain -> Initialize() Failed. Error Code => %d\n", vkResult);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fprintf(g_pFile, "WinMain -> Initialize() Successful.\n");
	}

	// Show Window
	ShowWindow(hwnd, iCmdShow);

	// Bring Window at top of all existing windows and set focus on it.
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if ((msg.message == WM_QUIT) || (g_bEscapeKeyPressed == TRUE))
			{
				bDone = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (g_bWindowMinimized == FALSE)
			{
				// Render the scene
				vkResult = Display();
				if ((vkResult != VK_SUCCESS) && (vkResult != VK_FALSE) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR) && (vkResult != VK_SUBOPTIMAL_KHR))
				{
					fprintf(g_pFile, "WinMain -> Display() Failed. Error Code => %d\n", vkResult);
					bDone = TRUE;
				}
				// Update the scene
				if (g_bActiveWindow)
				{
					Update();
				}

			}

		}
	}

	UnInitialize();

	return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// Function Declarations
	void FullScreen(void);
	VkResult Resize(int, int);
	void UnInitialize(void);

	switch (iMsg)
	{
	case WM_CREATE:
		memset((void*)&mi, 0, sizeof(mi));
		memset((void*)&wpPrev, 0, sizeof(wpPrev));
		wpPrev.length = sizeof(wpPrev);
		break;
	case WM_SETFOCUS:
		g_bActiveWindow = TRUE;
		break;

	case WM_KILLFOCUS:
		g_bActiveWindow = FALSE;
		break;

	case WM_ERASEBKGND:
		// Will repaint the client area while using WM_PAINT
		// return(0) when we are using rendering.
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'f':
		case 'F':
			FullScreen();
			break;
		default:
			break;
		}
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27: // Escape Key
			g_bEscapeKeyPressed = TRUE;
			// check escape key
			break;
		default:
			break;
		}
		break;

	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) //If our windows is not minimized!
		{
			g_bWindowMinimized = TRUE;
		}
		else
		{
			g_bWindowMinimized = FALSE;
			Resize(LOWORD(lParam), HIWORD(lParam));
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		UnInitialize();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return (DefWindowProc(hWnd, iMsg, wParam, lParam));
}

void FullScreen(void)
{
	if (g_bFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(g_hwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(mi);
			if (GetWindowPlacement(g_hwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(g_hwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(g_hwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW));
				SetWindowPos(g_hwnd, HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			ShowCursor(FALSE);
			g_bFullScreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(g_hwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(g_hwnd, &wpPrev);
		SetWindowPos(g_hwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
		g_bFullScreen = FALSE;
	}
}

VkResult Initialize(void)
{
	// Function Declarations.
	VkResult createVulkanInstance(void);
	VkResult GetSupourtedSurface(void);
	VkResult GetPhysicalDevice(void);
	VkResult PrintVKInfo(void);
	VkResult CreateVulkanDevice(void);
	void getDeviceQueue(void);
	VkResult CreateSwapchain(VkBool32);
	VkResult CreateSwapchainImagesAndSwapchainImageViews(void);
	VkResult CreateCommandPool(void);
	VkResult AllocateCommandBuffer(void);
	VkResult CreateVertexBuffer(void); // Order is Crucial as this place will work Both for staging and Non-staging buffers.
	VkResult CreateTexture(void); // Command Pool Must be allocated for this step
	VkResult CreateUniformBuffer(void);
	VkResult CreateShaders(void);
	VkResult CreateDescriptorSetLayout(void);
	VkResult CreatePipelineLayout(void);
	VkResult CreateDescriptorPool(void);
	VkResult CreateDescriptorSet(void);
	VkResult CreateRenderpass(void);
	VkResult CreatePipeline(void);
	VkResult CreateFrameBuffers(void);
	VkResult CreateSemaphores(void);
	VkResult CreateFences(void);
	VkResult BuildCommandBuffers(void);

	// Variable Declarations.
	VkResult vkResult = VK_SUCCESS;

	// Code
	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> createVulkanInstance() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> createVulkanInstance() Successful.\n");
	}

	// Create Vulkan Presentable Surface
	vkResult = GetSupourtedSurface();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> GetSupourtedSurface() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> GetSupourtedSurface() Successful.\n");
	}

	// Select Required Physical Device and it's queue family Index.
	vkResult = GetPhysicalDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> GetPhysicalDevice() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> GetPhysicalDevice() Successful.\n");
	}

	// Print Vulkan info
	vkResult = PrintVKInfo();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> PrintVKInfo() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> PrintVKInfo() Successful.\n");
	}

	vkResult = CreateVulkanDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateVulkanDevice() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateVulkanDevice() Successful.\n");
	}

	getDeviceQueue();

	vkResult = CreateSwapchain(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateSwapchain() Failed %d.\n", vkResult);
		return (VK_ERROR_INITIALIZATION_FAILED);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateSwapchain() Successful.\n");
	}

	vkResult = CreateSwapchainImagesAndSwapchainImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateSwapchainImagesAndSwapchainImageViews() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateSwapchainImagesAndSwapchainImageViews() Successful.\n");
	}

	vkResult = CreateCommandPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateCommandPool() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateCommandPool() Successful.\n");
	}

	vkResult = AllocateCommandBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> AllocateCommandBuffer() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> AllocateCommandBuffer() Successful.\n");
	}

	vkResult = CreateVertexBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateVertexBuffer() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateVertexBuffer() Successful.\n");
	}

	vkResult = CreateTexture();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateTexture() Failed% d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateTexture() Successful.\n");
	}

	vkResult = CreateUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateUniformBuffer() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateUniformBuffer() Successful.\n");
	}

	vkResult = CreateShaders();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateShaders() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateShaders() Successful.\n");
	}

	vkResult = CreateDescriptorSetLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateDescriptorSetLayout() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateDescriptorSetLayout() Successful.\n");
	}

	vkResult = CreatePipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreatePipelineLayout() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreatePipelineLayout() Successful.\n");
	}

	vkResult = CreateDescriptorPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateDescriptorPool() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateDescriptorPool() Successful.\n");
	}

	vkResult = CreateDescriptorSet();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateDescriptorSet() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateDescriptorSet() Successful.\n");
	}

	vkResult = CreateRenderpass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateRenderpass() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateRenderpass() Successful.\n");
	}

	vkResult = CreatePipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreatePipeline() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreatePipeline() Successful.\n");
	}

	vkResult = CreateFrameBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateFrameBuffers() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateFrameBuffers() Successful.\n");
	}

	vkResult = CreateSemaphores();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateSemaphores() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateSemaphores() Successful.\n");
	}

	vkResult = CreateFences();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> CreateFences() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> CreateFences() Successful.\n");
	}

	// initialize clear  color values.
	// Akin to glClearColor(); More Akin to DirectX
	memset((void*)&g_vkClearColorValue, 0, sizeof(g_vkClearColorValue));
	g_vkClearColorValue.float32[0] = 0.25f;
	g_vkClearColorValue.float32[1] = 0.25f;
	g_vkClearColorValue.float32[2] = 0.25f;
	g_vkClearColorValue.float32[3] = 1.0f;

	// Clear Depth Values
	memset((void*)&g_vkClearDepthStencilValue, 0, sizeof(g_vkClearDepthStencilValue));
	// Set default Clear Depth value
	g_vkClearDepthStencilValue.depth = 1.0f; // float
	// Set default clear stencil value
	g_vkClearDepthStencilValue.stencil = 0; // uint32_t

	vkResult = BuildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Initialize -> BuildCommandBuffers() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "Initialize -> BuildCommandBuffers() Successful.\n");
	}

	// Initialization is completed
	g_bInitialized = TRUE;
	return (vkResult);
}

VkResult Resize(int iWidth, int iHeight)
{
	// Function Declaration.
	VkResult CreateSwapchain(VkBool32);
	VkResult CreateSwapchainImagesAndSwapchainImageViews(void);
	VkResult AllocateCommandBuffer(void);
	VkResult CreatePipelineLayout(void);
	VkResult CreateRenderpass(void);
	VkResult CreatePipeline(void);
	VkResult CreateFrameBuffers(void);
	VkResult BuildCommandBuffers(void);

	// Variable Declaration.
	VkResult vkResult = VK_SUCCESS;

	fprintf(g_pFile, "Resize -> Inside Resize Width=%d Height=%d.\n", iWidth, iHeight);

	// Code
	// check g_bInitialized variable.
	if (g_bInitialized == FALSE)
	{
		fprintf(g_pFile, "Resize -> Initialization yet Not complete OR Failed.\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	// As re-creation of swapchain is needed we are going to repeat many steps of Initialize() again
	// Hence set g_bInitialized to FALSE again.
	g_bInitialized = FALSE;

	// Set global win width and win height variables
	g_iWinWidth = iWidth;
	g_iWinHeight = iHeight;

	// Before re-creation we need to destroy some objects.

	// Wait for device to complete in hand tasks
	if (g_vkDevice)
	{
		vkDeviceWaitIdle(g_vkDevice);
		fprintf(g_pFile, "Resize -> vkDeviceWaitIdle() Is Done.\n");
	}

	// check presence of swapchain
	if (g_vkSwapchainKHR == VK_NULL_HANDLE)
	{
		fprintf(g_pFile, "Resize -> Swapchain is already NULL, Can not proscede further.\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	// Destroy Framebuffer
	if (g_vkFramebuffer_array)
	{
		for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkDestroyFramebuffer(g_vkDevice, g_vkFramebuffer_array[i], NULL);
			g_vkFramebuffer_array[i] = VK_NULL_HANDLE;
			fprintf(g_pFile, "Resize -> vkDestroyFramebuffer() Successful.\n");
		}

		free(g_vkFramebuffer_array);
		g_vkFramebuffer_array = NULL;
	}

	// Destroy Command Buffer
	if (g_vkCommandBuffer_array)
	{
		for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkFreeCommandBuffers(g_vkDevice, g_vkCommandPool, 1, &g_vkCommandBuffer_array[i]);
			fprintf(g_pFile, "Resize -> vkFreeCommandBuffers() Successful.\n");
			g_vkCommandBuffer_array[i] = VK_NULL_HANDLE;
		}

		free(g_vkCommandBuffer_array);
		g_vkCommandBuffer_array = NULL;
	}

	// Destroy Pipeline
	if (g_vkPipeline)
	{
		vkDestroyPipeline(g_vkDevice, g_vkPipeline, NULL);
		g_vkPipeline = NULL;
		fprintf(g_pFile, "Resize -> vkDestroyPipeline() Successful.\n");
	}

	// Destroy Pipeline Layout
	if (g_vkPipelineLayout)
	{
		vkDestroyPipelineLayout(g_vkDevice, g_vkPipelineLayout, NULL);
		g_vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(g_pFile, "Resize -> vkDestroyPipelineLayout() Successful.\n");
	}

	// Destroy RenderPass
	if (g_vkRenderPass)
	{
		vkDestroyRenderPass(g_vkDevice, g_vkRenderPass, NULL);
		g_vkRenderPass = VK_NULL_HANDLE;
		fprintf(g_pFile, "Resize -> vkDestroyRenderPass() Successful.\n");
	}

	if (g_vkImageView_Depth)
	{
		vkDestroyImageView(g_vkDevice, g_vkImageView_Depth, NULL);
		fprintf(g_pFile, "Resize -> vkDestroyImageView(g_vkImageView_Depth) Successful.\n");
		g_vkImageView_Depth = VK_NULL_HANDLE;
	}

	if (g_vkImage_Depth)
	{
		vkDestroyImage(g_vkDevice, g_vkImage_Depth, NULL);
		fprintf(g_pFile, "Resize -> vkDestroyImage(g_vkImage_Depth) Successful.\n");
		g_vkImage_Depth = VK_NULL_HANDLE;
	}

	if (g_vkDeviceMemory_Depth)
	{
		vkFreeMemory(g_vkDevice, g_vkDeviceMemory_Depth, NULL);
		g_vkDeviceMemory_Depth = VK_NULL_HANDLE;
		fprintf(g_pFile, "Resize -> vkFreeMemory(g_vkDeviceMemory_Depth) Successful.\n");
	}

	// Destroy Swapchain Image View
	if (g_SwapchainImageView_array)
	{
		for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkDestroyImageView(g_vkDevice, g_SwapchainImageView_array[i], NULL);
			g_SwapchainImageView_array[i] = NULL;
			fprintf(g_pFile, "Resize -> Iteration %d vkDestroyImageView() Successful.\n", i);
		}

		free(g_SwapchainImageView_array);
		g_SwapchainImageView_array = NULL;
	}

	// Destroy Swapchain Image
	if (g_SwapchainImage_array)
	{
		/*for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkDestroyImage(g_vkDevice, g_SwapchainImage_array[i], NULL);
			fprintf(g_pFile, "Resize -> Iteration %d vkDestroyImageView() Successful.\n",i);
			g_SwapchainImage_array[i]=NULL;
		}*/
		free(g_SwapchainImage_array);
		g_SwapchainImage_array = NULL;
	}

	// Destroy Swapchain
	if (g_vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(g_vkDevice, g_vkSwapchainKHR, NULL);
		g_vkSwapchainKHR = NULL;
		fprintf(g_pFile, "Resize -> vkDestroySwapchainKHR() Successful.\n");
	}

	// RE-CREATE FOR RESIZE

	// Swapchain
	vkResult = CreateSwapchain(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> CreateSwapchain() Failed %d.\n", vkResult);
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	// Swapchain Images and Image Views
	vkResult = CreateSwapchainImagesAndSwapchainImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> CreateSwapchainImagesAndSwapchainImageViews() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// Renderpass
	vkResult = CreateRenderpass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> CreateRenderpass() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// Pipeline Layout
	vkResult = CreatePipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> CreatePipelineLayout() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// Pipeline => ViewPort and Scissor
	vkResult = CreatePipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> CreatePipeline() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// Framebuffer
	vkResult = CreateFrameBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> CreateFrameBuffers() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// Command Buffers
	vkResult = AllocateCommandBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> AllocateCommandBuffer() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// Build Command Buffers
	vkResult = BuildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Resize -> BuildCommandBuffers() Failed %d.\n", vkResult);
		return (vkResult);
	}

	fprintf(g_pFile, "Resize -> Resize() Completed Successfully.\n");
	g_bInitialized = TRUE;
	return (vkResult);
}

VkResult Display(void)
{
	VkResult Resize(int, int);
	VkResult UpdateUniformBuffer(void);

	// Variable Declaration.
	VkResult vkResult = VK_SUCCESS;

	// Code
	// if Control comes herebefore initialization gets completed, return FALSE.
	if (g_bInitialized == FALSE)
	{
		fprintf(g_pFile, "Display -> Initialization yet Not complete.\n");
		return ((VkResult)VK_FALSE);
	}

	// Acquire index of next swapchain Image
	// If this function does not return the image in given timeout the function will return VK_DEVICE_NOT_READY.
	vkResult = vkAcquireNextImageKHR(g_vkDevice, g_vkSwapchainKHR, UINT64_MAX, g_VkSemaphore_backBuffer, VK_NULL_HANDLE, &g_iCurrentImageIndex);
	if (vkResult != VK_SUCCESS)
	{
		if ((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
		{
			Resize(g_iWinWidth, g_iWinHeight);
		}
		else
		{
			fprintf(g_pFile, "Display -> vkAcquireNextImageKHR() Failed %d.\n", vkResult);
			return (vkResult);
		}
	}
	/*else
	{
		fprintf(g_pFile, "Display -> vkAcquireNextImageKHR() gave Image Index => %d.\n", g_iCurrentImageIndex);
	}*/

	// Use fence to allow host to wait for completion of execution of previous command buffer
	vkResult = vkWaitForFences(g_vkDevice, 1, &g_pvkFence_array[g_iCurrentImageIndex], VK_TRUE, UINT64_MAX);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Display -> vkWaitForFences() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// prepare the fences for next commandbuffer
	vkResult = vkResetFences(g_vkDevice, 1, &g_pvkFence_array[g_iCurrentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Display -> vkResetFences() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// one of the member of VkSubmitInfo structure requires array of pipeline stages, we have only one of completion of color aattachment output.
	// Still we need one member array.
	const VkPipelineStageFlags vkPipelineStageFlags_waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// Declare memeset and Initialize VkSubmitInfo structure.
	VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.pWaitDstStageMask = &vkPipelineStageFlags_waitDstStageMask;// Inter queue operation.
	vkSubmitInfo.waitSemaphoreCount = 1;
	vkSubmitInfo.pWaitSemaphores = &g_VkSemaphore_backBuffer;
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &g_vkCommandBuffer_array[g_iCurrentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &g_VkSemaphore_renderComplete;

	// Submit above work to the queue.
	vkResult = vkQueueSubmit(g_vkQueue, 1, &vkSubmitInfo, g_pvkFence_array[g_iCurrentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Display -> vkQueueSubmit() Failed %d.\n", vkResult);
		return (vkResult);
	}

	// we are going to present rendered image after declaring, and initializing VkPresentInfoKHR structure
	VkPresentInfoKHR vkPresentInfoKHR;
	memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
	vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	vkPresentInfoKHR.pNext = NULL;
	vkPresentInfoKHR.swapchainCount = 1;
	vkPresentInfoKHR.pSwapchains = &g_vkSwapchainKHR;
	vkPresentInfoKHR.pImageIndices = &g_iCurrentImageIndex;
	vkPresentInfoKHR.waitSemaphoreCount = 1;
	vkPresentInfoKHR.pWaitSemaphores = &g_VkSemaphore_renderComplete;

	vkResult = vkQueuePresentKHR(g_vkQueue, &vkPresentInfoKHR);
	if (vkResult != VK_SUCCESS)
	{
		if ((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
		{
			Resize(g_iWinWidth, g_iWinHeight);
		}
		else
		{
			fprintf(g_pFile, "Display -> vkQueuePresentKHR() Failed %d.\n", vkResult);
			return (vkResult);
		}
	}

	vkResult = UpdateUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "Display -> UpdateUniformBuffer() Failed %d.\n", vkResult);
		return (vkResult);
	}

	vkDeviceWaitIdle(g_vkDevice);


	return vkResult;
}

void Update(void)
{
}

void UnInitialize(void)
{
	// Function declarations
	void FullScreen(void);

	// Restore the window
	if (g_bFullScreen == TRUE)
	{
		FullScreen();
	}

	if (g_hwnd)
	{
		DestroyWindow(g_hwnd);
		g_hwnd = NULL;
	}

	// g_vkQueue
	// No need to destroy/uninitialize device queue.
	if (g_vkDevice)
	{
		vkDeviceWaitIdle(g_vkDevice);
		fprintf(g_pFile, "UnInitialize -> vkDeviceWaitIdle() Is Done.\n");
	}

	// Fence
	if (g_pvkFence_array)
	{
		for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkDestroyFence(g_vkDevice, g_pvkFence_array[i], NULL);
			g_pvkFence_array[i] = VK_NULL_HANDLE;
			fprintf(g_pFile, "UnInitialize -> vkDestroyFences() Successful.\n");
		}

		free(g_pvkFence_array);
		g_pvkFence_array = NULL;
	}

	// Semaphore
	if (g_VkSemaphore_renderComplete)
	{
		vkDestroySemaphore(g_vkDevice, g_VkSemaphore_renderComplete, NULL);
		g_VkSemaphore_renderComplete = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroySemaphore(g_VkSemaphore_renderComplete) Successful.\n");
	}

	if (g_VkSemaphore_backBuffer)
	{
		vkDestroySemaphore(g_vkDevice, g_VkSemaphore_backBuffer, NULL);
		g_VkSemaphore_backBuffer = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroySemaphore(g_VkSemaphore_backBuffer) Successful.\n");
	}

	// Framebuffer
	if (g_vkFramebuffer_array)
	{
		for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkDestroyFramebuffer(g_vkDevice, g_vkFramebuffer_array[i], NULL);
			g_vkFramebuffer_array[i] = VK_NULL_HANDLE;
			fprintf(g_pFile, "UnInitialize -> vkDestroyFramebuffer() Successful.\n");
		}

		free(g_vkFramebuffer_array);
		g_vkFramebuffer_array = NULL;
	}

	if (g_vkPipeline)
	{
		vkDestroyPipeline(g_vkDevice, g_vkPipeline, NULL);
		g_vkPipeline = NULL;
		fprintf(g_pFile, "UnInitialize -> vkDestroyPipeline() Successful.\n");
	}

	// RenderPass
	if (g_vkRenderPass)
	{
		vkDestroyRenderPass(g_vkDevice, g_vkRenderPass, NULL);
		g_vkRenderPass = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyRenderPass() Successful.\n");
	}

	if (g_vkDescriptorPool)
	{
		vkDestroyDescriptorPool(g_vkDevice, g_vkDescriptorPool, NULL);
		g_vkDescriptorPool = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyDescriptorPool() Successful.\n");
		fprintf(g_pFile, "UnInitialize -> vkDestroyDescriptorPool() Descriptor Sets are automatically destroyed with Descriptor Pool.\n");
	}

	descriptorSets.clear();
	descriptorSets.shrink_to_fit();
	// When descriptor pool is destroyed then Descriptor Sets created by it is automatically destroyed

	if (g_vkPipelineLayout)
	{
		vkDestroyPipelineLayout(g_vkDevice, g_vkPipelineLayout, NULL);
		g_vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyPipelineLayout() Successful.\n");
	}

	for (uint32_t idx = 0; idx <= descriptorSetLayouts.size(); idx++)
	{
		vkDestroyDescriptorSetLayout(g_vkDevice, descriptorSetLayouts[idx], NULL);
		descriptorSetLayouts[idx] = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> descriptorSetLayouts[%d] Successful.\n", idx);
	}

	descriptorSetLayouts.clear();
	descriptorSetLayouts.shrink_to_fit();

	// Shader Modules
	if (g_vkShaderModule_Fragment_Shader)
	{
		vkDestroyShaderModule(g_vkDevice, g_vkShaderModule_Fragment_Shader, NULL);
		g_vkShaderModule_Fragment_Shader = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyShaderModule(Fragment Shader) Successful.\n");
	}

	if (g_vkShaderModule_Vertex_Shader)
	{
		vkDestroyShaderModule(g_vkDevice, g_vkShaderModule_Vertex_Shader, NULL);
		g_vkShaderModule_Vertex_Shader = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyShaderModule(Vertex Shader) Successful.\n");
	}

	// Uniform Buffer
	if (g_UniformData_straight.vkDeviceMemory)
	{
		vkFreeMemory(g_vkDevice, g_UniformData_straight.vkDeviceMemory, NULL);
		g_UniformData_straight.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkFreeMemory(g_UniformData_straight.vkDeviceMemory) Successful.\n");
	}

	if (g_UniformData_straight.vkBuffer)
	{
		vkDestroyBuffer(g_vkDevice, g_UniformData_straight.vkBuffer, NULL);
		g_UniformData_straight.vkBuffer = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyBuffer(g_UniformData_straight.vkBuffer) Successful.\n");
	}

	if (g_UniformData_straight_model.vkDeviceMemory)
	{
		vkFreeMemory(g_vkDevice, g_UniformData_straight_model.vkDeviceMemory, NULL);
		g_UniformData_straight_model.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkFreeMemory(g_UniformData_straight_model.vkDeviceMemory) Successful.\n");
	}

	if (g_UniformData_straight_model.vkBuffer)
	{
		vkDestroyBuffer(g_vkDevice, g_UniformData_straight_model.vkBuffer, NULL);
		g_UniformData_straight_model.vkBuffer = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyBuffer(g_UniformData_straight_model.vkBuffer) Successful.\n");
	}

	// Textures
	if (g_vkSampler_texture)
	{
		vkDestroySampler(g_vkDevice, g_vkSampler_texture, NULL);
		g_vkSampler_texture = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroySampler(g_vkSampler_texture) Successful.\n");
	}

	if (g_vkImageView_texture)
	{
		vkDestroyImageView(g_vkDevice, g_vkImageView_texture, NULL);
		g_vkImageView_texture = NULL;
		fprintf(g_pFile, "UnInitialize -> vkDestroyImageView(g_vkImageView_texture) Successful.\n");
	}

	if (g_vkDeviceMemory_texture)
	{
		vkFreeMemory(g_vkDevice, g_vkDeviceMemory_texture, NULL);
		g_vkDeviceMemory_texture = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkFreeMemory(g_vkDeviceMemory_texture) Successful.\n");
	}

	if (g_vkImage_texture)
	{
		vkDestroyImage(g_vkDevice, g_vkImage_texture, NULL);
		g_vkImage_texture = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyImage(g_vkImage_texture) Successful.\n");
	}

	// Vertex Buffer TexCoord
	if (g_VertexData_texcoord.vkDeviceMemory)
	{
		vkFreeMemory(g_vkDevice, g_VertexData_texcoord.vkDeviceMemory, NULL);
		g_VertexData_texcoord.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkFreeMemory(g_VertexData_texcoord.vkDeviceMemory) Successful.\n");
	}

	if (g_VertexData_texcoord.vkBuffer)
	{
		vkDestroyBuffer(g_vkDevice, g_VertexData_texcoord.vkBuffer, NULL);
		g_VertexData_texcoord.vkBuffer = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyBuffer(g_VertexData_texcoord.vkBuffer) Successful.\n");
	}

	// Vertex Buffer Position
	if (g_VertexData_position.vkDeviceMemory)
	{
		vkFreeMemory(g_vkDevice, g_VertexData_position.vkDeviceMemory, NULL);
		g_VertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkFreeMemory(g_VertexData_position.vkDeviceMemory) Successful.\n");
	}

	if (g_VertexData_position.vkBuffer)
	{
		vkDestroyBuffer(g_vkDevice, g_VertexData_position.vkBuffer, NULL);
		g_VertexData_position.vkBuffer = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyBuffer(g_VertexData_position.vkBuffer) Successful.\n");
	}

	// Command Buffer
	if (g_vkCommandBuffer_array)
	{
		for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkFreeCommandBuffers(g_vkDevice, g_vkCommandPool, 1, &g_vkCommandBuffer_array[i]);
			fprintf(g_pFile, "UnInitialize -> vkFreeCommandBuffers() Successful.\n");
			g_vkCommandBuffer_array[i] = VK_NULL_HANDLE;
		}

		free(g_vkCommandBuffer_array);
		g_vkCommandBuffer_array = NULL;
	}

	if (g_vkCommandPool)
	{
		vkDestroyCommandPool(g_vkDevice, g_vkCommandPool, NULL);
		fprintf(g_pFile, "UnInitialize -> vkDestroyCommandPool() Successful.\n");
		fflush(g_pFile);
		g_vkCommandPool = VK_NULL_HANDLE;
	}

	if (g_vkImageView_Depth)
	{
		vkDestroyImageView(g_vkDevice, g_vkImageView_Depth, NULL);
		fprintf(g_pFile, "UnInitialize -> vkDestroyImageView(g_vkImageView_Depth) Successful.\n");
		g_vkImageView_Depth = VK_NULL_HANDLE;
	}

	if (g_vkImage_Depth)
	{
		vkDestroyImage(g_vkDevice, g_vkImage_Depth, NULL);
		fprintf(g_pFile, "UnInitialize -> vkDestroyImage(g_vkImage_Depth) Successful.\n");
		g_vkImage_Depth = VK_NULL_HANDLE;
	}

	if (g_vkDeviceMemory_Depth)
	{
		vkFreeMemory(g_vkDevice, g_vkDeviceMemory_Depth, NULL);
		g_vkDeviceMemory_Depth = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkFreeMemory(g_vkDeviceMemory_Depth) Successful.\n");
	}

	// Swapchain ImageView
	for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
	{
		vkDestroyImageView(g_vkDevice, g_SwapchainImageView_array[i], NULL);
		g_SwapchainImageView_array[i] = NULL;
		fprintf(g_pFile, "UnInitialize -> Iteration %d vkDestroyImageView() Successful.\n", i);
	}

	if (g_SwapchainImageView_array)
	{
		free(g_SwapchainImageView_array);
		g_SwapchainImageView_array = NULL;
	}

	// Swapchain Image
	if (g_SwapchainImage_array)
	{
		/*
		* We dont need to destroy presentable Image as it will be desroyed by vkDestroySwapchainKHR().
		*  Howerver will need to destroy Regular Texture Images!
		*/
		/*for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
		{
			vkDestroyImage(g_vkDevice, g_SwapchainImage_array[i], NULL);
			fprintf(g_pFile, "UnInitialize -> Iteration %d vkDestroyImageView() Successful.\n",i);
			g_SwapchainImage_array[i]=NULL;
		}*/
		free(g_SwapchainImage_array);
		g_SwapchainImage_array = NULL;
	}

	// Physical Device Step: no need to destroy selected physical device.
	if (g_vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(g_vkDevice, g_vkSwapchainKHR, NULL);
		g_vkSwapchainKHR = NULL;
		fprintf(g_pFile, "UnInitialize -> vkDestroySwapchainKHR() Successful.\n");
	}

	if (g_vkDevice)
	{
		vkDestroyDevice(g_vkDevice, NULL);
		g_vkDevice = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyDevice() Successful.\n");
	}

	if (g_vkSurfaceKHR)
	{
		// Destroy function is generic, unlike creation process.
		vkDestroySurfaceKHR(g_vkInstance, g_vkSurfaceKHR, NULL);
		g_vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroySurfaceKHR() Successful.\n");
	}

	if (g_vkDebugReportCallbackEXT && g_vkDestroyDebugReportCallbackEXT_fnptr)
	{
		g_vkDestroyDebugReportCallbackEXT_fnptr(g_vkInstance, g_vkDebugReportCallbackEXT, NULL);
		g_vkDebugReportCallbackEXT = VK_NULL_HANDLE;
		g_vkDestroyDebugReportCallbackEXT_fnptr = NULL;
		fprintf(g_pFile, "UnInitialize -> g_vkDestroyDebugReportCallbackEXT_fnptr() Successful.\n");
	}

	if (g_vkInstance)
	{
		vkDestroyInstance(g_vkInstance, NULL);
		g_vkInstance = VK_NULL_HANDLE;
		fprintf(g_pFile, "UnInitialize -> vkDestroyInsatnce() Successful.\n");
	}

	if (g_pFile)
	{
		fprintf(g_pFile, "UnInitialize -> Log File Closed Successfully.\n");
		fflush(g_pFile);
		fclose(g_pFile);
		g_pFile = NULL;
	}
}

// Definitionsof all Vulkan Related Functions.
VkResult createVulkanInstance(void)
{
	// Function Declarations.
	VkResult FillInstanceExtensionNames(void);
	VkResult FillInstanceValidationLayerNames(void);
	VkResult CreateValidationCallback(void);


	// Variable Declarations.
	VkResult vkResult = VK_SUCCESS;

	// Code
	vkResult = FillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "createVulkanInstance -> FillInstanceExtensionNames() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "createVulkanInstance -> FillInstanceExtensionNames() Successful.\n");
	}

	// Validation

	if (g_bEnableValidation == TRUE)
	{
		vkResult = FillInstanceValidationLayerNames();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "createVulkanInstance -> FillInstanceValidationLayerNames() Failed.\n");
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "createVulkanInstance -> FillInstanceValidationLayerNames() Successful.\n");
		}
	}

	// Initialize VkApplicationInfo
	VkApplicationInfo vkApplicationInfo;
	memset((void*)&vkApplicationInfo, 0, sizeof(vkApplicationInfo));
	// Typesafety and generic ness across the OS.
	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.pNext = NULL;
	vkApplicationInfo.pApplicationName = g_pszAppName;
	vkApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vkApplicationInfo.pEngineName = g_pszAppName;
	vkApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

	// Initialize struct Instance Info.
	VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void*)&vkInstanceCreateInfo, 0, sizeof(vkInstanceCreateInfo));
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = g_iEnaledInstanceExtensionCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = g_pchEnableInstanceExtensionNames_array;
	if (g_bEnableValidation == TRUE)
	{
		vkInstanceCreateInfo.enabledLayerCount = g_iEnaledInstanceLayerCount;
		vkInstanceCreateInfo.ppEnabledLayerNames = g_pchEnableInstanceLayerNames_array;
	}
	else
	{
		vkInstanceCreateInfo.enabledLayerCount = 0;
		vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
	}

	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL /*No custom Memory allocator*/, &g_vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(g_pFile, "createVulkanInstance -> vkCreateInstance() Failed, Due to incompatible driver.%d.\n", vkResult);
		return (vkResult);
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(g_pFile, "createVulkanInstance -> vkCreateInstance() Failed, Due to Absence of Extension.%d.\n", vkResult);
		return (vkResult);
	}
	else if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "createVulkanInstance -> vkCreateInstance() Failed, Due to UnknownError.%d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "createVulkanInstance -> vkCreateInstance() Successful.\n");
	}

	// Validation Callbacks
	if (g_bEnableValidation)
	{
		vkResult = CreateValidationCallback();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "createVulkanInstance -> CreateValidationCallback() Failed.\n");
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "createVulkanInstance -> CreateValidationCallback() Successful.\n");
		}
	}

	return vkResult;
}

VkResult FillInstanceExtensionNames(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// Reteive count of Instances and keep this in local variable.
	uint32_t iInstanceExtensionCount = 0;
	vkResult = vkEnumerateInstanceExtensionProperties(NULL /*Which Layer's Extension*/,
		&iInstanceExtensionCount /*count varibale*/,
		NULL /*Array of VkExtensionProperties*/);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> 1st call to vkEnumerateInstanceExtensionProperties() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> 1st call to vkEnumerateInstanceExtensionProperties() Successful.\n");
	}

	// 2. Allocate and fill struct VkExtensionProperties

	VkExtensionProperties* vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * iInstanceExtensionCount);
	if (vkExtensionProperties_array == NULL)
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> malloc() for VkExtensionProperties Failed.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	vkResult = vkEnumerateInstanceExtensionProperties(NULL,
		&iInstanceExtensionCount,
		vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> 2nd call to vkEnumerateInstanceExtensionProperties() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> 2nd call to vkEnumerateInstanceExtensionProperties() Successful.\n");
	}

	// 3. Fill a string array obtained from VkExtensionProperties (Names of Extension)
	char** instanceExtensionNames_array = NULL;
	instanceExtensionNames_array = (char**)malloc(sizeof(char*) * iInstanceExtensionCount);
	if (instanceExtensionNames_array == NULL)
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> MALLOC failed to initialize memory for instanceExtensionNames_array.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	for (uint32_t i = 0; i < iInstanceExtensionCount; i++)
	{
		instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		if (instanceExtensionNames_array[i] == NULL)
		{
			fprintf(g_pFile, "FillInstanceExtensionNames -> MALLOC failed to initialize memory for instanceExtensionNames_array[%d].\n", i);
			return (VK_ERROR_INITIALIZATION_FAILED);
		}
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(g_pFile, "FillInstanceExtensionNames -> Vulkan Extension Name= %s\n", instanceExtensionNames_array[i]);
	}

	// 4. Free vkExtensionProperties_array
	if (vkExtensionProperties_array)
	{
		free(vkExtensionProperties_array);
		vkExtensionProperties_array = NULL;
	}

	// 5.
	// 5. Fill Global varibales g_pchEnableInstanceExtensionNames_array
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 win32SurfaceExtensionFound = VK_FALSE;
	VkBool32 debugReportExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < iInstanceExtensionCount; i++)
	{
		if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanSurfaceExtensionFound = VK_TRUE;
			g_pchEnableInstanceExtensionNames_array[g_iEnaledInstanceExtensionCount] = VK_KHR_SURFACE_EXTENSION_NAME;
			g_iEnaledInstanceExtensionCount++;
		}
		if (strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			win32SurfaceExtensionFound = VK_TRUE;
			g_pchEnableInstanceExtensionNames_array[g_iEnaledInstanceExtensionCount] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
			g_iEnaledInstanceExtensionCount++;
		}
		if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
		{
			debugReportExtensionFound = VK_TRUE;
			if (g_bEnableValidation)
			{
				g_pchEnableInstanceExtensionNames_array[g_iEnaledInstanceExtensionCount] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
				g_iEnaledInstanceExtensionCount++;
			}
			else
			{
				// Array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
				fprintf(g_pFile, "FillInstanceExtensionNames -> Array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME \n");
			}
		}
	}

	// 6. free local arrays
	for (uint32_t i = 0; i < iInstanceExtensionCount; i++)
	{
		free(instanceExtensionNames_array[i]);
		instanceExtensionNames_array[i] = NULL;
	}

	if (instanceExtensionNames_array)
	{
		free(instanceExtensionNames_array);
		instanceExtensionNames_array = NULL;
	}

	// 7.Display Wether required Instance extension Names are supourted or not
	if (vulkanSurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; // Return hard coded failure.
		fprintf(g_pFile, "FillInstanceExtensionNames -> VK_KHR_SURFACE_EXTENSION_NAME Not found \n");
		return vkResult;
	}
	else
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> VK_KHR_SURFACE_EXTENSION_NAME found \n");
	}

	if (win32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; // Return hard coded failure.
		fprintf(g_pFile, "FillInstanceExtensionNames -> VK_KHR_WIN32_SURFACE_EXTENSION_NAME Not found \n");
		return vkResult;
	}
	else
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found \n");
	}

	if (debugReportExtensionFound == VK_FALSE)
	{
		if (g_bEnableValidation == TRUE)
		{
			vkResult = VK_ERROR_INITIALIZATION_FAILED; // Return hard coded failure.
			fprintf(g_pFile, "FillInstanceExtensionNames -> Validation is ON but VK_EXT_DEBUG_REPORT_EXTENSION_NAME is NOT supported \n");
			return vkResult;
		}
		else
		{
			fprintf(g_pFile, "FillInstanceExtensionNames -> Validation is OFF and VK_EXT_DEBUG_REPORT_EXTENSION_NAME is NOT supported\n");
		}
	}
	else
	{
		if (g_bEnableValidation == TRUE)
		{
			fprintf(g_pFile, "FillInstanceExtensionNames -> Validation is ON and VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
		}
		else
		{
			fprintf(g_pFile, "FillInstanceExtensionNames -> Validation is OFF and VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
		}
	}

	// Print only enabled extension names
	for (uint32_t i = 0; i < g_iEnaledInstanceExtensionCount; i++)
	{
		fprintf(g_pFile, "FillInstanceExtensionNames -> Enabled Vulkan Instance Extension Name= %s\n", g_pchEnableInstanceExtensionNames_array[i]);
	}

	return vkResult;
}

VkResult FillInstanceValidationLayerNames(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	uint32_t iInstanceValidationLayerCount = 0;
	//SAM
	vkResult = vkEnumerateInstanceLayerProperties(/*NULL Which Layer's Extension,*/
		&iInstanceValidationLayerCount /*count varibale*/,
		NULL /*Array of VkExtensionProperties*/);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> 1st call to vkEnumerateInstanceLayerProperties() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> 1st call to vkEnumerateInstanceLayerProperties() Successful.\n");
	}

	// 2. Allocate and fill struct VkLayerProperties
	VkLayerProperties* vkLayerProperties_array = NULL;
	vkLayerProperties_array = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * iInstanceValidationLayerCount);
	if (vkLayerProperties_array == NULL)
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> malloc() for VkLayerProperties Failed.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	//SAM
	vkResult = vkEnumerateInstanceLayerProperties(/*NULL,*/&iInstanceValidationLayerCount, vkLayerProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> 2nd call to vkEnumerateInstanceLayerProperties() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> 2nd call to vkEnumerateInstanceLayerProperties() Successful.\n");
	}

	// 3. Fill a string array obtained from VkLayerProperties (Names of Layer)
	char** instanceLayerNames_array = NULL;
	instanceLayerNames_array = (char**)malloc(sizeof(char*) * iInstanceValidationLayerCount);
	if (instanceLayerNames_array == NULL)
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> MALLOC failed to initialize memory for instanceLayerNames_array.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	for (uint32_t i = 0; i < iInstanceValidationLayerCount; i++)
	{
		instanceLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
		if (instanceLayerNames_array[i] == NULL)
		{
			fprintf(g_pFile, "FillInstanceValidationLayerNames -> MALLOC failed to initialize memory for instanceLayerNames_array[%d].\n", i);
			return (VK_ERROR_INITIALIZATION_FAILED);
		}
		memcpy(instanceLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> Vulkan Extension Name= %s\n", instanceLayerNames_array[i]);
	}

	// 4. Free vkExtensionProperties_array
	if (vkLayerProperties_array)
	{
		free(vkLayerProperties_array);
		vkLayerProperties_array = NULL;
	}

	// 5. Fill Global varibales g_pchEnableInstanceLayerNames_array
	VkBool32 validationLayerFound = VK_FALSE;

	for (uint32_t i = 0; i < iInstanceValidationLayerCount; i++)
	{
		if (strcmp(instanceLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
		{
			validationLayerFound = VK_TRUE;
			g_pchEnableInstanceLayerNames_array[g_iEnaledInstanceLayerCount] = "VK_LAYER_KHRONOS_validation";
			g_iEnaledInstanceLayerCount++;
		}
	}

	// 6. free local arrays
	for (uint32_t i = 0; i < iInstanceValidationLayerCount; i++)
	{
		free(instanceLayerNames_array[i]);
		instanceLayerNames_array[i] = NULL;
	}

	if (instanceLayerNames_array)
	{
		free(instanceLayerNames_array);
		instanceLayerNames_array = NULL;
	}

	// 7.Display Wether required Instance Layer Names are supourted or not
	if (validationLayerFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; // Return hard coded failure.
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> VK_LAYER_KHRONOS_validation Not Supourted \n");
		return vkResult;
	}
	else
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> VK_LAYER_KHRONOS_validation Supourted \n");
	}

	// Print only enabled layer names
	for (uint32_t i = 0; i < g_iEnaledInstanceLayerCount; i++)
	{
		fprintf(g_pFile, "FillInstanceValidationLayerNames -> Enabled Vulkan Instance Layer Name= %s\n", g_pchEnableInstanceLayerNames_array[i]);
	}

	return vkResult;
}

VkResult CreateValidationCallback(void)
{
	// Function Declarations
	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*);

	// VKAPI_ATTR=> controller for calling convention for GCC and clang. convention for C++ 11 and beyond.
	// VKAPI_CALL => MSVC (Win32) calling convention.

	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

	//1. Get the required function Pointers.
	vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g_vkInstance, "vkCreateDebugReportCallbackEXT");
	if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; // Return hard coded failure.
		fprintf(g_pFile, "CreateValidationCallback -> vkGetInstanceProcAddr(vkCreateDebugReportCallbackEXT) NOT Found. \n");
		return vkResult;
	}
	else
	{
		fprintf(g_pFile, "CreateValidationCallback -> vkGetInstanceProcAddr(vkCreateDebugReportCallbackEXT) Found. \n");
	}

	g_vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g_vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (g_vkDestroyDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; // Return hard coded failure.
		fprintf(g_pFile, "CreateValidationCallback -> vkGetInstanceProcAddr(vkDestroyDebugReportCallbackEXT) NOT Found. \n");
		return vkResult;
	}
	else
	{
		fprintf(g_pFile, "CreateValidationCallback -> vkGetInstanceProcAddr(vkDestroyDebugReportCallbackEXT) Found. \n");
	}

	//2. Obtain VkDebugReportCallbackEXT (vulkan debug report callback object)
	VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
	memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(vkDebugReportCallbackCreateInfoEXT));
	vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
	vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
	vkDebugReportCallbackCreateInfoEXT.pUserData = (void*)g_pFile;

	vkResult = vkCreateDebugReportCallbackEXT_fnptr(g_vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &g_vkDebugReportCallbackEXT);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateValidationCallback -> vkCreateDebugReportCallbackEXT_fnptr() Failed, Due to %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateValidationCallback -> vkCreateDebugReportCallbackEXT_fnptr() Successful.\n");
	}

	return vkResult;
}

VkResult GetSupourtedSurface(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// 2. Declare and memset() platform specifiec Surface CreateInfo Structure
	VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
	memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(vkWin32SurfaceCreateInfoKHR));
	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	// vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetModuleHandle(NULL); // It can be used like this as well.
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(g_hwnd, GWLP_HINSTANCE); // 64-bit compatible Win32 function to get HINSTANCE
	vkWin32SurfaceCreateInfoKHR.hwnd = g_hwnd;

	// 3. Call the Function
	vkResult = vkCreateWin32SurfaceKHR(g_vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &g_vkSurfaceKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "GetSupourtedSurface -> vkCreateWin32SurfaceKHR() Failed, Due to %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "GetSupourtedSurface -> vkCreateWin32SurfaceKHR() Successful.\n");
	}

	return vkResult;
}

VkResult GetPhysicalDevice(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	vkResult = vkEnumeratePhysicalDevices(g_vkInstance, &g_iPhysicalDeviceCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "GetPhysicalDevice -> 1st call to vkEnumeratePhysicalDevices() Failed => %d.\n", vkResult);
		return (vkResult);
	}
	else if (g_iPhysicalDeviceCount == 0)
	{
		fprintf(g_pFile, "GetPhysicalDevice -> 1st call to vkEnumeratePhysicalDevices() resulted in 0 physical Devices.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}
	else
	{
		fprintf(g_pFile, "GetPhysicalDevice -> 1st call to vkEnumeratePhysicalDevices() Successful.\n");
	}

	g_pvkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * g_iPhysicalDeviceCount);
	if (g_pvkPhysicalDevice_array == NULL)
	{
		fprintf(g_pFile, "GetPhysicalDevice -> MALLOC failed for g_pvkPhysicalDevice_array.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}
	memset((void*)g_pvkPhysicalDevice_array, 0, sizeof(VkPhysicalDevice) * g_iPhysicalDeviceCount);

	vkResult = vkEnumeratePhysicalDevices(g_vkInstance, &g_iPhysicalDeviceCount, g_pvkPhysicalDevice_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "GetPhysicalDevice -> 2nd call to vkEnumeratePhysicalDevices() Failed => %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "GetPhysicalDevice -> 2nd call to vkEnumeratePhysicalDevices() Successful.\n");
	}

	VkBool32 bFound = VK_FALSE;
	for (uint32_t i = 0; i < g_iPhysicalDeviceCount; i++)
	{
		uint32_t iQueueCount = UINT32_MAX;
		VkQueueFamilyProperties* pvkQueueFamilyProperties_array = NULL;
		VkBool32* pIsQueueSurfaceSupported_array = NULL;
		// if physical device ie present then it MUST have at least 1 Queue family.
		vkGetPhysicalDeviceQueueFamilyProperties(g_pvkPhysicalDevice_array[i], &iQueueCount, NULL);
		pvkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * iQueueCount);

		// 5 (d)
		vkGetPhysicalDeviceQueueFamilyProperties(g_pvkPhysicalDevice_array[i], &iQueueCount, pvkQueueFamilyProperties_array);

		pIsQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * iQueueCount);
		for (uint32_t j = 0; j < iQueueCount; j++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(g_pvkPhysicalDevice_array[i], j, g_vkSurfaceKHR, &pIsQueueSurfaceSupported_array[j]);
		}

		for (uint32_t j = 0; j < iQueueCount; j++)
		{
			if (pvkQueueFamilyProperties_array[j].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT))
			{
				if (pIsQueueSurfaceSupported_array[j] == VK_TRUE)
				{
					g_vkPhysicalDevice_selected = g_pvkPhysicalDevice_array[i];
					g_iGraphicsQueueFamilyIndex_selected = j;
					bFound = VK_TRUE;
					break;
				}
			}
		}

		fprintf(g_pFile, "GetPhysicalDevice -> g_iGraphicsQueueFamilyIndex_selected=>%d.\n", g_iGraphicsQueueFamilyIndex_selected);

		if (pIsQueueSurfaceSupported_array)
		{
			free(pIsQueueSurfaceSupported_array);
			pIsQueueSurfaceSupported_array = NULL;
			fprintf(g_pFile, "GetPhysicalDevice -> pIsQueueSurfaceSupported_array freed.\n");
		}

		if (pvkQueueFamilyProperties_array)
		{
			free(pvkQueueFamilyProperties_array);
			pvkQueueFamilyProperties_array = NULL;
			fprintf(g_pFile, "GetPhysicalDevice -> pvkQueueFamilyProperties_array freed.\n");
		}

		if (bFound == VK_TRUE)
		{
			break;
		}
	}

	// Selected Device is
	VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
	memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(vkPhysicalDeviceProperties));

	vkGetPhysicalDeviceProperties(g_vkPhysicalDevice_selected, &vkPhysicalDeviceProperties);

	// Device Name
	fprintf(g_pFile, "SAM : Selected Device Name => %s \n", vkPhysicalDeviceProperties.deviceName);

	memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

	vkGetPhysicalDeviceMemoryProperties(g_vkPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties);

	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
	memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

	vkGetPhysicalDeviceFeatures(g_vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

	if (vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(g_pFile, "GetPhysicalDevice -> Selected Physical Device supports Tessilation Shader.\n");
	}
	else
	{
		fprintf(g_pFile, "GetPhysicalDevice -> Selected Physical Device does NOT supports Tessilation Shader.\n");
	}

	if (vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(g_pFile, "GetPhysicalDevice -> Selected Physical Device supports Geometry Shader.\n");
	}
	else
	{
		fprintf(g_pFile, "GetPhysicalDevice -> Selected Physical Device does NOT supports Geometry Shader.\n");
	}

	return vkResult;
}

VkResult PrintVKInfo(void)
{
	VkResult vkResult = VK_SUCCESS;
	fprintf(g_pFile, "*********************** Vulakn Info ***********************\n");

	for (uint32_t i = 0; i < g_iPhysicalDeviceCount; i++)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(vkPhysicalDeviceProperties));

		vkGetPhysicalDeviceProperties(g_pvkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

		uint32_t iMajorversion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t iMinorversion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t iPatchversion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

		// API vaersion
		fprintf(g_pFile, "API Version = %d.%d.%d\n", iMajorversion, iMinorversion, iPatchversion);

		// Device Name
		fprintf(g_pFile, "Device Name => %s \n", vkPhysicalDeviceProperties.deviceName);

		// device type
		fprintf(g_pFile, "Device Type => ");
		switch (vkPhysicalDeviceProperties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			fprintf(g_pFile, "Integrated GPU (iGPU)\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			fprintf(g_pFile, "Discrete GPU (dGPU)\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			fprintf(g_pFile, "Virtual GPU\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			fprintf(g_pFile, "CPU\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			fprintf(g_pFile, "Other\n");
			break;

		default:
			fprintf(g_pFile, "UNKNOWN\n");
			break;
		}

		// Vendoe ID
		fprintf(g_pFile, "Vendor ID = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);

		// Device ID
		fprintf(g_pFile, "Device ID = 0x%04x\n", vkPhysicalDeviceProperties.deviceID);
	}

	if (g_pvkPhysicalDevice_array)
	{
		fprintf(g_pFile, "PrintVKInfo -> Successful Obtained Physical Device with Graphics Bit.\n");
		if (g_pvkPhysicalDevice_array)
		{
			free(g_pvkPhysicalDevice_array);
			g_pvkPhysicalDevice_array = NULL;
			fprintf(g_pFile, "PrintVKInfo -> g_pvkPhysicalDevice_array freed.\n");
		}
	}
	else
	{
		fprintf(g_pFile, "PrintVKInfo -> Failed to Obtained Physical Device with Graphics Bit.\n");
		if (g_pvkPhysicalDevice_array)
		{
			free(g_pvkPhysicalDevice_array);
			g_pvkPhysicalDevice_array = NULL;
			fprintf(g_pFile, "PrintVKInfo -> g_pvkPhysicalDevice_array freed.\n");
		}
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	fprintf(g_pFile, "***********************************************************\n");
	return vkResult;
}

VkResult CreateSwapchain(VkBool32 vsync)
{
	// Function Declarations
	VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void);
	VkResult GetPresentationMode(void);

	// Variable Declarations
	VkResult vkResult = VK_SUCCESS;

	vkResult = GetPhysicalDeviceSurfaceFormatAndColorSpace();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchain -> GetPhysicalDeviceSurfaceFormatAndColorSpace() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchain -> GetPhysicalDeviceSurfaceFormatAndColorSpace() Successful.\n");
	}

	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(vkSurfaceCapabilitiesKHR));
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_vkPhysicalDevice_selected, g_vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchain -> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchain -> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Successful.\n");
	}

	// 3. Find out desired Number of Swapchain Images
	uint32_t iTestNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
	uint32_t iDesiredNumberOfSwapchainImages = 0;

	fprintf(g_pFile, "CreateSwapchain -> vkSurfaceCapabilitiesKHR.minImageCount = %d.\n", vkSurfaceCapabilitiesKHR.minImageCount);
	fprintf(g_pFile, "CreateSwapchain -> vkSurfaceCapabilitiesKHR.maxImageCount = %d.\n", vkSurfaceCapabilitiesKHR.maxImageCount);
	fprintf(g_pFile, "CreateSwapchain -> vkSurfaceCapabilitiesKHR.maxImageArrayLayers = %d.\n", vkSurfaceCapabilitiesKHR.maxImageArrayLayers);
	fprintf(g_pFile, "CreateSwapchain -> vkSurfaceCapabilitiesKHR.currentExtent = %d x %d.\n", vkSurfaceCapabilitiesKHR.currentExtent.width, vkSurfaceCapabilitiesKHR.currentExtent.height);
	fprintf(g_pFile, "CreateSwapchain -> vkSurfaceCapabilitiesKHR.minImageExtent = %d x %d.\n", vkSurfaceCapabilitiesKHR.minImageExtent.width, vkSurfaceCapabilitiesKHR.minImageExtent.height);
	fprintf(g_pFile, "CreateSwapchain -> vkSurfaceCapabilitiesKHR.maxImageExtent = %d x %d.\n", vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkSurfaceCapabilitiesKHR.maxImageExtent.height);

	if ((vkSurfaceCapabilitiesKHR.maxImageCount > 0) && (vkSurfaceCapabilitiesKHR.maxImageCount < iTestNumberOfSwapchainImages))
	{
		iDesiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
		fprintf(g_pFile, "CreateSwapchain -> iDesiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount(%d).\n", iDesiredNumberOfSwapchainImages);
	}
	else
	{
		iDesiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
		fprintf(g_pFile, "CreateSwapchain -> iDesiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1(%d).\n", iDesiredNumberOfSwapchainImages);
	}

	// 4. Choose Swapchain Image size.
	memset((void*)&g_vkExtent2D_swapchain, 0, sizeof(g_vkExtent2D_swapchain));
	if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX) // Not already created.
	{
		g_vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		g_vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(g_pFile, "CreateSwapchain -> Swapchain Image Size (Unknown Values) = %d x %d.\n", vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkSurfaceCapabilitiesKHR.maxImageExtent.height);
	}
	else
	{
		// If surface size is already defined than swapchain image size MUST match with it.
		VkExtent2D vkExtent2D;
		memset((void*)&vkExtent2D, 0, sizeof(vkExtent2D));
		vkExtent2D.width = g_iWinWidth;
		vkExtent2D.height = g_iWinHeight;

		g_vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
		g_vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
	}

	// 5. Set Swapchain Image usage Flag.
	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; // Why to use the swapchain Imgaes.

	// 6. Wheather to consider pre-transform/flipping or not ?
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
	if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
	}

	// 7. Presentation Mode.
	vkResult = GetPresentationMode();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchain -> GetPresentationMode() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchain -> GetPresentationMode() Successful.\n");
	}

	// 8. Fill the structure and Create Swapchain.
	VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
	memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(vkSwapchainCreateInfoKHR));
	vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapchainCreateInfoKHR.pNext = NULL;
	vkSwapchainCreateInfoKHR.flags = 0;
	vkSwapchainCreateInfoKHR.surface = g_vkSurfaceKHR;
	vkSwapchainCreateInfoKHR.minImageCount = iDesiredNumberOfSwapchainImages;
	vkSwapchainCreateInfoKHR.imageFormat = g_vkFormat_Color;
	vkSwapchainCreateInfoKHR.imageColorSpace = g_vkColorSpaceKHR;
	vkSwapchainCreateInfoKHR.imageExtent.width = g_vkExtent2D_swapchain.width;
	vkSwapchainCreateInfoKHR.imageExtent.height = g_vkExtent2D_swapchain.height;
	vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
	vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
	vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
	vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // For only one Queue, Do we need to shae swapchain image?
	vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCreateInfoKHR.presentMode = g_vkPresentModeKHR;
	vkSwapchainCreateInfoKHR.clipped = VK_TRUE;
	vkResult = vkCreateSwapchainKHR(g_vkDevice, &vkSwapchainCreateInfoKHR, NULL, &g_vkSwapchainKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchain -> vkCreateSwapchainKHR() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchain -> vkCreateSwapchainKHR() Successful.\n");
	}

	return vkResult;
}

VkResult CreateVulkanDevice(void)
{
	// Function Declaration
	VkResult FillDeviceExtensionNames(void);

	VkResult vkResult = VK_SUCCESS;

	vkResult = FillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVulkanDevice -> FillDeviceExtensionNames() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVulkanDevice -> FillDeviceExtensionNames() Successful.\n");
	}

	// Newly, Added Code.
	float fQueuePriorirtes[1] = { 1.0f };
	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
	memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(vkDeviceQueueCreateInfo));
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.pNext = NULL;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.queueCount = 1;
	vkDeviceQueueCreateInfo.queueFamilyIndex = g_iGraphicsQueueFamilyIndex_selected;
	vkDeviceQueueCreateInfo.pQueuePriorities = fQueuePriorirtes;

	// Initialize VkDeviceCreateInfo structure
	VkDeviceCreateInfo vkDeviceCreateInfo;
	memset((void*)&vkDeviceCreateInfo, 0, sizeof(vkDeviceCreateInfo));
	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.enabledExtensionCount = g_iEnabledDeviceExtensionCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = g_pchEnableDeviceExtensionNames_array;
	vkDeviceCreateInfo.pEnabledFeatures = NULL;
	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;
	// Below two are Depricated and ignored.
	// In latest specification we need to enable this in Instance Creation only.
	vkDeviceCreateInfo.enabledLayerCount = 0;
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;

	vkResult = vkCreateDevice(g_vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &g_vkDevice);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVulkanDevice -> vkCreateDevice() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVulkanDevice -> vkCreateDevice() Successful.\n");
	}

	return vkResult;
}

void getDeviceQueue(void)
{
	// vkGetDeviceQueue
	vkGetDeviceQueue(g_vkDevice, g_iGraphicsQueueFamilyIndex_selected, 0, &g_vkQueue);
	if (g_vkQueue == VK_NULL_HANDLE)
	{
		fprintf(g_pFile, "getDeviceQueue -> vkGetDeviceQueue() Failed.\n");
	}
	else
	{
		fprintf(g_pFile, "getDeviceQueue -> vkGetDeviceQueue() Successful.\n");
	}
	return;
}

VkResult GetPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
	VkResult vkResult = VK_SUCCESS;
	// code

	// Get the count of Supourted color fomrats (VkFormats)
	uint32_t iColorFomatCount = 0;
	VkSurfaceFormatKHR* pvkSurfaceFormatKHR_array = NULL;

	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(g_vkPhysicalDevice_selected, g_vkSurfaceKHR, &iColorFomatCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace -> vkGetPhysicalDeviceSurfaceFormatsKHR() 1st call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace -> vkGetPhysicalDeviceSurfaceFormatsKHR() 1st call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Successful.\n");
	}

	// Allocate and Intialize
	pvkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(iColorFomatCount * sizeof(VkSurfaceFormatKHR));

	// Fill the Array
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(g_vkPhysicalDevice_selected, g_vkSurfaceKHR, &iColorFomatCount, pvkSurfaceFormatKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace -> vkGetPhysicalDeviceSurfaceFormatsKHR() 2nd call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Failed.\n");
		if (pvkSurfaceFormatKHR_array)
		{
			free(pvkSurfaceFormatKHR_array);
			pvkSurfaceFormatKHR_array = NULL;
		}
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace -> vkGetPhysicalDeviceSurfaceFormatsKHR() 2nd call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Successful.\n");
	}

	// Decide the SurfaceColorFormat First.
	if (iColorFomatCount == 1 && pvkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
	{
		g_vkFormat_Color = VK_FORMAT_B8G8R8_UNORM;
	}
	else
	{
		g_vkFormat_Color = pvkSurfaceFormatKHR_array[0].format;
	}

	// Decide the color Space
	g_vkColorSpaceKHR = pvkSurfaceFormatKHR_array[0].colorSpace;

	if (pvkSurfaceFormatKHR_array)
	{
		free(pvkSurfaceFormatKHR_array);
		pvkSurfaceFormatKHR_array = NULL;
		fprintf(g_pFile, "GetPhysicalDeviceSurfaceFormatAndColorSpace -> pvkSurfaceFormatKHR_array is freed.\n");
	}
	return vkResult;
}

VkResult GetPresentationMode(void)
{
	VkResult vkResult = VK_SUCCESS;
	uint32_t iPresentModesCount = 0;
	VkPresentModeKHR* pvkPresentModeKHR_array = NULL;
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(g_vkPhysicalDevice_selected, g_vkSurfaceKHR, &iPresentModesCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "GetPresentationMode -> vkGetPhysicalDeviceSurfacePresentModesKHR() 1st call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Failed.\n");
		return (vkResult);
	}
	else if (iPresentModesCount == 0)
	{
		fprintf(g_pFile, "GetPresentationMode -> vkGetPhysicalDeviceSurfacePresentModesKHR() 1st call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Failed.\nGot zero Presentation Mode Count");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(g_pFile, "GetPresentationMode -> vkGetPhysicalDeviceSurfacePresentModesKHR() 1st call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Successful.\n");
	}

	pvkPresentModeKHR_array = (VkPresentModeKHR*)malloc(iPresentModesCount * sizeof(VkPresentModeKHR));

	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(g_vkPhysicalDevice_selected, g_vkSurfaceKHR, &iPresentModesCount, pvkPresentModeKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "GetPresentationMode -> vkGetPhysicalDeviceSurfacePresentModesKHR() 2nd call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Failed.\n");
		if (pvkPresentModeKHR_array)
		{
			free(pvkPresentModeKHR_array);
			pvkPresentModeKHR_array = NULL;
		}
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "GetPresentationMode -> vkGetPhysicalDeviceSurfacePresentModesKHR() 2nd call to GetPhysicalDeviceSurfaceFormatAndColorSpace() Successful.\n");
	}

	// Decide the presentation mode
	for (uint32_t i = 0; i < iPresentModesCount; i++)
	{
		if (pvkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			g_vkPresentModeKHR = pvkPresentModeKHR_array[i];
			break;
		}
	}

	if (g_vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		g_vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR; // If not supourted get atleast bare minimum
	}

	fprintf(g_pFile, "GetPresentationMode -> Finaly Obtained g_vkPresentModeKHR = %d.\n", g_vkPresentModeKHR);

	if (pvkPresentModeKHR_array)
	{
		free(pvkPresentModeKHR_array);
		pvkPresentModeKHR_array = NULL;
		fprintf(g_pFile, "GetPresentationMode -> pvkPresentModeKHR_array cleared.\n");
	}
	return (vkResult);
}

VkResult CreateSwapchainImagesAndSwapchainImageViews(void)
{
	VkResult GetSupportedDepthFormat(void);

	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// 1. Get swapchain Image Count.
	vkResult = vkGetSwapchainImagesKHR(g_vkDevice, g_vkSwapchainKHR, &g_iSwapchainImageCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> 1st call to vkGetSwapchainImagesKHR() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else if (g_iSwapchainImageCount == 0)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> 1st call to vkGetSwapchainImagesKHR() Failed [%d].\n", vkResult);
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> *Image Count is Zero*.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> 1st call to vkGetSwapchainImagesKHR() Successful.\n");
	}

	fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> g_iSwapchainImageCount = %d.\n", g_iSwapchainImageCount);

	// 2. Allocate the Swapchain Image Array.
	g_SwapchainImage_array = (VkImage*)malloc(g_iSwapchainImageCount * sizeof(VkImage));
	if (g_SwapchainImage_array == NULL)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> MALLOC can not allocate memory for g_SwapchainImage_array.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	// 3. Fill the above array with Swapchain Images.
	vkResult = vkGetSwapchainImagesKHR(g_vkDevice, g_vkSwapchainKHR, &g_iSwapchainImageCount, g_SwapchainImage_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> 2nd call to vkGetSwapchainImagesKHR() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> 2nd call to vkGetSwapchainImagesKHR() Successful.\n");
	}

	// 4. Allocate the array of Swapchain Images.
	g_SwapchainImageView_array = (VkImageView*)malloc(g_iSwapchainImageCount * sizeof(VkImageView));
	if (g_SwapchainImageView_array == NULL)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> MALLOC can not allocate memory for g_SwapchainImageView_array.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	// 5. Fill VkImageViewCreateInfo
	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(vkImageViewCreateInfo));
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = g_vkFormat_Color;
	// VkComponentMapping
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	// Subresource some part of the actual resource.
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// AspectMask => which part of the Image or whole of the Image is going to be affected by barrier.
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	// 6. Create ImageViews
	for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
	{
		vkImageViewCreateInfo.image = g_SwapchainImage_array[i];
		vkResult = vkCreateImageView(g_vkDevice, &vkImageViewCreateInfo, NULL, &g_SwapchainImageView_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> Iteration %d vkCreateImageView() Failed %d.\n", i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> Iteration %d vkCreateImageView() Successful.\n", i);
		}
	}

	// For Depth Image.
	vkResult = GetSupportedDepthFormat();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> GetSupportedDepthFormat() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> GetSupportedDepthFormat() Successful.\n");
	}

	// For depth Image Initialize vkImageCreateInfo
	VkImageCreateInfo vkImageCreateInfo;
	memset((void*)&vkImageCreateInfo, 0, sizeof(vkImageCreateInfo));
	vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vkImageCreateInfo.flags = 0;
	vkImageCreateInfo.pNext = NULL;
	vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	vkImageCreateInfo.format = g_vkFormat_Depth;
	vkImageCreateInfo.extent.width = g_vkExtent2D_swapchain.width;
	vkImageCreateInfo.extent.height = g_vkExtent2D_swapchain.height;
	vkImageCreateInfo.extent.depth = 1;
	vkImageCreateInfo.mipLevels = 1;
	vkImageCreateInfo.arrayLayers = 1;
	vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	vkResult = vkCreateImage(g_vkDevice, &vkImageCreateInfo, NULL, &g_vkImage_Depth);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkCreateImage() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkCreateImage() Successful.\n");
	}

	// Memory Requirements for depth Image
	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

	vkGetImageMemoryRequirements(g_vkDevice, g_vkImage_Depth, &vkMemoryRequirements); // Region wise allocation which is alligned!

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0; // Initial value before entring into loop!

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(g_vkDevice, &vkMemoryAllocateInfo, NULL, &g_vkDeviceMemory_Depth);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkAllocateMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkAllocateMemory() Successful.\n");
	}

	// It binds vulkan device memory object habdle with vulkan buffer object handle.
	vkResult = vkBindImageMemory(g_vkDevice, g_vkImage_Depth, g_vkDeviceMemory_Depth, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkBindBufferMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkBindBufferMemory() Successful.\n");
	}

	// Create Image View For above Depth Image.
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(vkImageViewCreateInfo));
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = g_vkFormat_Depth;
	vkImageViewCreateInfo.image = g_vkImage_Depth;
	// Subresource some part of the actual resource.
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	// AspectMask => which part of the Image or whole of the Image is going to be affected by barrier.
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	vkResult = vkCreateImageView(g_vkDevice, &vkImageViewCreateInfo, NULL, &g_vkImageView_Depth);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkCreateImageView() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSwapchainImagesAndSwapchainImageViews -> vkCreateImageView() Successful.\n");
	}

	return (vkResult);
}

VkResult GetSupportedDepthFormat(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	VkFormat vkFormat_Depth_array[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM };

	for (uint32_t i = 0; i < (sizeof(vkFormat_Depth_array) / sizeof(vkFormat_Depth_array[0])); i++)
	{
		VkFormatProperties vkFormatProperties;
		memset((void*)&vkFormatProperties, 0, sizeof(vkFormatProperties));
		vkGetPhysicalDeviceFormatProperties(g_vkPhysicalDevice_selected, vkFormat_Depth_array[i], &vkFormatProperties);
		if (vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			g_vkFormat_Depth = vkFormat_Depth_array[i];
			vkResult = VK_SUCCESS;
			break;
		}
	}

	return (vkResult);
}

VkResult CreateCommandPool(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;
	VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
	memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(vkCommandPoolCreateInfo));
	vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext = NULL;
	vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCommandPoolCreateInfo.queueFamilyIndex = g_iGraphicsQueueFamilyIndex_selected;

	vkResult = vkCreateCommandPool(g_vkDevice, &vkCommandPoolCreateInfo, NULL, &g_vkCommandPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateCommandPool -> vkCreateCommandPool() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateCommandPool -> vkCreateCommandPool() Successful.\n");
	}
	return vkResult;
}

VkResult AllocateCommandBuffer(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(vkCommandBufferAllocateInfo));
	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = g_vkCommandPool;
	vkCommandBufferAllocateInfo.commandBufferCount = 1;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	g_vkCommandBuffer_array = (VkCommandBuffer*)malloc(g_iSwapchainImageCount * sizeof(VkCommandBuffer));
	if (g_vkCommandBuffer_array == NULL)
	{
		fprintf(g_pFile, "AllocateCommandBuffer() -> MALLOC for g_vkCommandBuffer_array failed.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
	{
		vkResult = vkAllocateCommandBuffers(g_vkDevice, &vkCommandBufferAllocateInfo, &g_vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "AllocateCommandBuffer -> Iteration %d vkAllocateCommandBuffers() Failed [%d].\n", i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "AllocateCommandBuffer -> Iteration %d vkAllocateCommandBuffers() Successful.\n", i);
		}
	}

	return vkResult;
}

VkResult CreateVertexBuffer(void)
{
	VkResult vkResult = VK_SUCCESS;

	float quad_position[] =
	{
		// First Triangle
	   1.0f, 1.0f, 0.0f,	// right top
	   -1.0f, 1.0f, 0.0f,	// left top
	   -1.0f, -1.0f, 0.0f,	// left bottom

	   // Second Triangle
	   -1.0f, -1.0f, 0.0f,	// left bottom
	   1.0f, -1.0f, 0.0f,	// right bottom
	   1.0f, 1.0f, 0.0f		// right top
	};

	float quad_texcoord[] =
	{
		// First Triangle
		1.0f,1.0f,// Right Top
		0.0f,1.0f,// left top
		0.0f,0.0f,// left bottom

		// Second Triangle
		0.0f,0.0f,// left bottom
		1.0f,0.0f,// right bottom
		1.0f,1.0f,// Right Top
	};

	// Vertex Buffer Position
	memset((void*)&g_VertexData_position, 0, sizeof(g_VertexData_position));

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(vkBufferCreateInfo));
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; // Valid Flags are used in scattered buffer/ Sparse buffer.
	vkBufferCreateInfo.size = sizeof(quad_position);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	/*We need following memebers when we have to share same buffer to multiple queues */
	/*vkBufferCreateInfo.queueFamilyIndexCount = 1;
	vkBufferCreateInfo.pQueueFamilyIndices = &g_iGraphicsQueueFamilyIndex_selected;
	vkBufferCreateInfo.sharingMode= VK_SHARING_MODE_EXCLUSIVE;*/
	/*
	* In Vulkan Memory allocation is not done in buytes, but It is done in Regions !
	*/

	vkResult = vkCreateBuffer(g_vkDevice, &vkBufferCreateInfo, NULL, &g_VertexData_position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkCreateBuffer() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkCreateBuffer() Successful.\n");
	}

	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

	vkGetBufferMemoryRequirements(g_vkDevice, g_VertexData_position.vkBuffer, &vkMemoryRequirements); // Region wise allocation which is alligned!

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0;// Initial value before entring into loop!

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(g_vkDevice, &vkMemoryAllocateInfo, NULL, &g_VertexData_position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkAllocateMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkAllocateMemory() Successful.\n");
	}

	// It binds vulkan device memory object habdle with vulkan buffer object handle.
	vkResult = vkBindBufferMemory(g_vkDevice, g_VertexData_position.vkBuffer, g_VertexData_position.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkBindBufferMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkBindBufferMemory() Successful.\n");
	}

	void* data = NULL;
	vkResult = vkMapMemory(g_vkDevice, g_VertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkMapMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkMapMemory() Successful.\n");
	}

	// Actual memeory mapped I/O
	memcpy(data, quad_position, sizeof(quad_position));

	vkUnmapMemory(g_vkDevice, g_VertexData_position.vkDeviceMemory);

	// Vertex Buffer Color
	memset((void*)&g_VertexData_texcoord, 0, sizeof(g_VertexData_texcoord));

	memset((void*)&vkBufferCreateInfo, 0, sizeof(vkBufferCreateInfo));
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; // Valid Flags are used in scattered buffer/ Sparse buffer.
	vkBufferCreateInfo.size = sizeof(quad_texcoord);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	vkResult = vkCreateBuffer(g_vkDevice, &vkBufferCreateInfo, NULL, &g_VertexData_texcoord.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkCreateBuffer(Color Buffer) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkCreateBuffer(Color Buffer) Successful.\n");
	}

	memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

	vkGetBufferMemoryRequirements(g_vkDevice, g_VertexData_texcoord.vkBuffer, &vkMemoryRequirements);

	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0;

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(g_vkDevice, &vkMemoryAllocateInfo, NULL, &g_VertexData_texcoord.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkAllocateMemory(Color Buffer) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkAllocateMemory(Color Buffer) Successful.\n");
	}

	// It binds vulkan device memory object habdle with vulkan buffer object handle.
	vkResult = vkBindBufferMemory(g_vkDevice, g_VertexData_texcoord.vkBuffer, g_VertexData_texcoord.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkBindBufferMemory(Color Buffer) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkBindBufferMemory(Color Buffer) Successful.\n");
	}

	data = NULL;
	vkResult = vkMapMemory(g_vkDevice, g_VertexData_texcoord.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkMapMemory(Color Buffer) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateVertexBuffer -> vkMapMemory(Color Buffer) Successful.\n");
	}

	// Actual memeory mapped I/O
	memcpy(data, quad_texcoord, sizeof(quad_texcoord));

	vkUnmapMemory(g_vkDevice, g_VertexData_texcoord.vkDeviceMemory);

	return vkResult;
}

VkResult CreateTexture(void)
{
	// Function Declaration
	void MakeCheckImage(void);
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// Code.
	// 1.Get image info like width, height, number of channels and actual image data.
	MakeCheckImage();

	uint8_t* pImageData = (uint8_t*)g_bytCheckerBoard;
	int iTextureWidth = CHECKER_BOARD_WIDTH, iTextureHeight = CHECKER_BOARD_HEIGHT, iTextureChannels = 4;


	// 2. Put above image data into a staging buffer.
	VkDeviceSize vkDeviceSize_ImageSize = 0;
	VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vkDeviceMemory_StagingBuffer = VK_NULL_HANDLE;
	vkDeviceSize_ImageSize = iTextureWidth * iTextureHeight * iTextureChannels; // 4=>RGBA

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(vkBufferCreateInfo));
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0;
	vkBufferCreateInfo.size = vkDeviceSize_ImageSize;
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // this buffer is our source of empty image buffer
	vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkResult = vkCreateBuffer(g_vkDevice, &vkBufferCreateInfo, NULL, &vkBuffer_stagingBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateBuffer() Staging Buffer Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateBuffer() Staging Buffer Successful.\n");
	}

	VkMemoryRequirements vkMemoryRequirements_StagingBuffer;
	memset((void*)&vkMemoryRequirements_StagingBuffer, 0, sizeof(vkMemoryRequirements_StagingBuffer));

	vkGetBufferMemoryRequirements(g_vkDevice, vkBuffer_stagingBuffer, &vkMemoryRequirements_StagingBuffer);

	VkMemoryAllocateInfo vkMemoryAllocateInfo_StagingBuffer;
	memset((void*)&vkMemoryAllocateInfo_StagingBuffer, 0, sizeof(vkMemoryAllocateInfo_StagingBuffer));
	vkMemoryAllocateInfo_StagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo_StagingBuffer.pNext = NULL;
	vkMemoryAllocateInfo_StagingBuffer.allocationSize = vkMemoryRequirements_StagingBuffer.size;
	vkMemoryAllocateInfo_StagingBuffer.memoryTypeIndex = 0;// Initial value before entring into loop!

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements_StagingBuffer.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			{
				vkMemoryAllocateInfo_StagingBuffer.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements_StagingBuffer.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(g_vkDevice, &vkMemoryAllocateInfo_StagingBuffer, NULL, &vkDeviceMemory_StagingBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateMemory() for Staging Buffer Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateMemory() for Staging Buffer Successful.\n");
	}

	vkResult = vkBindBufferMemory(g_vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_StagingBuffer, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkBindBufferMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkBindBufferMemory() Successful.\n");
	}

	void* data = NULL;
	vkResult = vkMapMemory(g_vkDevice, vkDeviceMemory_StagingBuffer, 0, vkDeviceSize_ImageSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkMapMemory(Staging Buffer) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkMapMemory(Staging Buffer) Successful.\n");
	}

	// Actual memeory mapped I/O
	memcpy(data, pImageData, vkDeviceSize_ImageSize);

	vkUnmapMemory(g_vkDevice, vkDeviceMemory_StagingBuffer);


	// 3. Create "Device only visible", empty BUT enough sized "VkImage".
	VkImageCreateInfo vkImageCreateInfo;
	memset((void*)&vkImageCreateInfo, 0, sizeof(vkImageCreateInfo));
	vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vkImageCreateInfo.pNext = NULL;
	vkImageCreateInfo.flags = 0;
	vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	vkImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;// Some samples use SRGB, we are using UNORM for having portability in mobile and desktops
	vkImageCreateInfo.extent.width = iTextureWidth;
	vkImageCreateInfo.extent.height = iTextureHeight;
	vkImageCreateInfo.extent.depth = 1;
	vkImageCreateInfo.mipLevels = 1;
	vkImageCreateInfo.arrayLayers = 1;
	vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkResult = vkCreateImage(g_vkDevice, &vkImageCreateInfo, NULL, &g_vkImage_texture);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateImage() Texture Image Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateImage() Texture Image Successful.\n");
	}

	VkMemoryRequirements vkMemoryRequirements_Image;
	memset((void*)&vkMemoryRequirements_Image, 0, sizeof(vkMemoryRequirements_Image));
	vkGetImageMemoryRequirements(g_vkDevice, g_vkImage_texture, &vkMemoryRequirements_Image);

	VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
	memset((void*)&vkMemoryAllocateInfo_image, 0, sizeof(vkMemoryAllocateInfo_image));
	vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo_image.pNext = NULL;
	vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_Image.size;
	vkMemoryAllocateInfo_image.memoryTypeIndex = 0;// Initial value before entring into loop!

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements_Image.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				vkMemoryAllocateInfo_image.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements_Image.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(g_vkDevice, &vkMemoryAllocateInfo_image, NULL, &g_vkDeviceMemory_texture);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateMemory(DeviceMemory_texture) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateMemory(DeviceMemory_texture) Successful.\n");
	}

	vkResult = vkBindImageMemory(g_vkDevice, g_vkImage_texture, g_vkDeviceMemory_texture, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkBindImageMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkBindImageMemory() Successful.\n");
	}

	// 4. Send "Image Transition Layout" to the Vulkan API\GPU before the actual Image copy!!
	// Command Pool Must be allocated for this step
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_Transition_Image_Layout;
	memset((void*)&vkCommandBufferAllocateInfo_Transition_Image_Layout, 0, sizeof(vkCommandBufferAllocateInfo_Transition_Image_Layout));
	vkCommandBufferAllocateInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo_Transition_Image_Layout.commandPool = g_vkCommandPool;
	vkCommandBufferAllocateInfo_Transition_Image_Layout.commandBufferCount = 1;
	vkCommandBufferAllocateInfo_Transition_Image_Layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;
	vkResult = vkAllocateCommandBuffers(g_vkDevice, &vkCommandBufferAllocateInfo_Transition_Image_Layout, &vkCommandBuffer_Transition_Image_Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateCommandBuffers(CommandBuffer_Transition_Image_Layout) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateCommandBuffers(CommandBuffer_Transition_Image_Layout) Successful.\n");
	}

	VkCommandBufferBeginInfo vkCommandBufferBeginInfo_Transition_Image_Layout;
	memset((void*)&vkCommandBufferBeginInfo_Transition_Image_Layout, 0, sizeof(vkCommandBufferBeginInfo_Transition_Image_Layout));
	vkCommandBufferBeginInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo_Transition_Image_Layout.pNext = NULL;
	vkCommandBufferBeginInfo_Transition_Image_Layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkResult = vkBeginCommandBuffer(vkCommandBuffer_Transition_Image_Layout, &vkCommandBufferBeginInfo_Transition_Image_Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkBeginCommandBuffer(Transition_Image_Layout) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkBeginCommandBuffer(Transition_Image_Layout) Successful.\n");
	}

	VkPipelineStageFlags vkPipelineStageFlags_Source = 0;
	VkPipelineStageFlags vkPipelineStageFlags_Destination = 0;
	VkImageMemoryBarrier vkImageMemoryBarrier;
	memset((void*)&vkImageMemoryBarrier, 0, sizeof(vkImageMemoryBarrier));
	vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	vkImageMemoryBarrier.pNext = NULL;
	vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.image = g_vkImage_texture;
	vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	vkImageMemoryBarrier.subresourceRange.layerCount = 1;
	vkImageMemoryBarrier.subresourceRange.levelCount = 1;

	if ((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask = 0;
		vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TRANSFER_BIT;
		vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> Unsuported Texture Layout Transition.\n");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	vkCmdPipelineBarrier(vkCommandBuffer_Transition_Image_Layout, vkPipelineStageFlags_Source, vkPipelineStageFlags_Destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);

	vkResult = vkEndCommandBuffer(vkCommandBuffer_Transition_Image_Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkEndCommandBuffer() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkEndCommandBuffer() Successful.\n");
	}

	VkSubmitInfo vkSubmitInfo_Transition_Image_Layout;
	memset((void*)&vkSubmitInfo_Transition_Image_Layout, 0, sizeof(vkSubmitInfo_Transition_Image_Layout));
	vkSubmitInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo_Transition_Image_Layout.pNext = NULL;
	vkSubmitInfo_Transition_Image_Layout.commandBufferCount = 1;
	vkSubmitInfo_Transition_Image_Layout.pCommandBuffers = &vkCommandBuffer_Transition_Image_Layout;

	vkResult = vkQueueSubmit(g_vkQueue, 1, &vkSubmitInfo_Transition_Image_Layout, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueSubmit() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueSubmit() Successful.\n");
	}

	vkResult = vkQueueWaitIdle(g_vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueWaitIdle() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueWaitIdle() Successful.\n");
	}

	if (vkCommandBuffer_Transition_Image_Layout)
	{
		vkFreeCommandBuffers(g_vkDevice, g_vkCommandPool, 1, &vkCommandBuffer_Transition_Image_Layout);
		vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;
	}

	//5. Now actually copy the image data from Staging Buffer to the Empty VkImage.
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_Buffer_To_Image_Copy;
	memset((void*)&vkCommandBufferAllocateInfo_Buffer_To_Image_Copy, 0, sizeof(vkCommandBufferAllocateInfo_Buffer_To_Image_Copy));
	vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.commandPool = g_vkCommandPool;
	vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.commandBufferCount = 1;
	vkCommandBufferAllocateInfo_Buffer_To_Image_Copy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer vkCommandBuffer_Buffer_To_Image_Copy = VK_NULL_HANDLE;
	vkResult = vkAllocateCommandBuffers(g_vkDevice, &vkCommandBufferAllocateInfo_Buffer_To_Image_Copy, &vkCommandBuffer_Buffer_To_Image_Copy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateCommandBuffers(CommandBuffer_Buffer_To_Image_Copy) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateCommandBuffers(CommandBuffer_Buffer_To_Image_Copy) Successful.\n");
	}

	VkCommandBufferBeginInfo vkCommandBufferBeginInfo_Buffer_To_Image_Copy;
	memset((void*)&vkCommandBufferBeginInfo_Buffer_To_Image_Copy, 0, sizeof(vkCommandBufferBeginInfo_Buffer_To_Image_Copy));
	vkCommandBufferBeginInfo_Buffer_To_Image_Copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo_Buffer_To_Image_Copy.pNext = NULL;
	vkCommandBufferBeginInfo_Buffer_To_Image_Copy.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkResult = vkBeginCommandBuffer(vkCommandBuffer_Buffer_To_Image_Copy, &vkCommandBufferBeginInfo_Buffer_To_Image_Copy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkBeginCommandBuffer(Transition_Image_Layout) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkBeginCommandBuffer(Transition_Image_Layout) Successful.\n");
	}

	VkBufferImageCopy vkBufferImageCopy;
	memset((void*)&vkBufferImageCopy, 0, sizeof(vkBufferImageCopy));
	vkBufferImageCopy.bufferOffset = 0;
	vkBufferImageCopy.bufferRowLength = 0;
	vkBufferImageCopy.bufferImageHeight = 0;
	vkBufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkBufferImageCopy.imageSubresource.mipLevel = 0;
	vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
	vkBufferImageCopy.imageSubresource.layerCount = 1;
	vkBufferImageCopy.imageOffset.x = 0;
	vkBufferImageCopy.imageOffset.y = 0;
	vkBufferImageCopy.imageOffset.z = 0;
	vkBufferImageCopy.imageExtent.width = iTextureWidth;
	vkBufferImageCopy.imageExtent.height = iTextureHeight;
	vkBufferImageCopy.imageExtent.depth = 1;

	vkCmdCopyBufferToImage(vkCommandBuffer_Buffer_To_Image_Copy, vkBuffer_stagingBuffer, g_vkImage_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vkBufferImageCopy);

	vkResult = vkEndCommandBuffer(vkCommandBuffer_Buffer_To_Image_Copy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkEndCommandBuffer(Buffer_To_Image_Copy) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkEndCommandBuffer(Buffer_To_Image_Copy) Successful.\n");
	}

	VkSubmitInfo vkSubmitInfo_Buffer_To_Image_Copy;
	memset((void*)&vkSubmitInfo_Buffer_To_Image_Copy, 0, sizeof(vkSubmitInfo_Buffer_To_Image_Copy));
	vkSubmitInfo_Buffer_To_Image_Copy.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo_Buffer_To_Image_Copy.pNext = NULL;
	vkSubmitInfo_Buffer_To_Image_Copy.commandBufferCount = 1;
	vkSubmitInfo_Buffer_To_Image_Copy.pCommandBuffers = &vkCommandBuffer_Buffer_To_Image_Copy;

	vkResult = vkQueueSubmit(g_vkQueue, 1, &vkSubmitInfo_Buffer_To_Image_Copy, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueSubmit(Buffer_To_Image_Copy) Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueSubmit(Buffer_To_Image_Copy) Successful.\n");
	}

	vkResult = vkQueueWaitIdle(g_vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueWaitIdle(Buffer_To_Image_Copy) Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueWaitIdle(Buffer_To_Image_Copy) Successful.\n");
	}

	if (vkCommandBuffer_Buffer_To_Image_Copy)
	{
		vkFreeCommandBuffers(g_vkDevice, g_vkCommandPool, 1, &vkCommandBuffer_Buffer_To_Image_Copy);
		vkCommandBuffer_Buffer_To_Image_Copy = VK_NULL_HANDLE;
	}

	//6. Now again do "Image Transition Layout" similar to Step 4, for correct retrieval(reading / writing) of Image data by shaders.

	memset((void*)&vkCommandBufferAllocateInfo_Transition_Image_Layout, 0, sizeof(vkCommandBufferAllocateInfo_Transition_Image_Layout));
	vkCommandBufferAllocateInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo_Transition_Image_Layout.commandPool = g_vkCommandPool;
	vkCommandBufferAllocateInfo_Transition_Image_Layout.commandBufferCount = 1;
	vkCommandBufferAllocateInfo_Transition_Image_Layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;
	vkResult = vkAllocateCommandBuffers(g_vkDevice, &vkCommandBufferAllocateInfo_Transition_Image_Layout, &vkCommandBuffer_Transition_Image_Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateCommandBuffers(CommandBuffer_Transition_Image_Layout2) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkAllocateCommandBuffers(CommandBuffer_Transition_Image_Layout2) Successful.\n");
	}


	memset((void*)&vkCommandBufferBeginInfo_Transition_Image_Layout, 0, sizeof(vkCommandBufferBeginInfo_Transition_Image_Layout));
	vkCommandBufferBeginInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo_Transition_Image_Layout.pNext = NULL;
	vkCommandBufferBeginInfo_Transition_Image_Layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkResult = vkBeginCommandBuffer(vkCommandBuffer_Transition_Image_Layout, &vkCommandBufferBeginInfo_Transition_Image_Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkBeginCommandBuffer(Transition_Image_Layout2) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkBeginCommandBuffer(Transition_Image_Layout2) Successful.\n");
	}

	vkPipelineStageFlags_Source = 0;
	vkPipelineStageFlags_Destination = 0;

	memset((void*)&vkImageMemoryBarrier, 0, sizeof(vkImageMemoryBarrier));
	vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	vkImageMemoryBarrier.pNext = NULL;
	vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.image = g_vkImage_texture;
	vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	vkImageMemoryBarrier.subresourceRange.layerCount = 1;
	vkImageMemoryBarrier.subresourceRange.levelCount = 1;

	if ((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask = 0;
		vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkPipelineStageFlags_Source = VK_PIPELINE_STAGE_TRANSFER_BIT;
		vkPipelineStageFlags_Destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> Unsuported Texture Layout Transition.\n");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	vkCmdPipelineBarrier(vkCommandBuffer_Transition_Image_Layout, vkPipelineStageFlags_Source, vkPipelineStageFlags_Destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);

	vkResult = vkEndCommandBuffer(vkCommandBuffer_Transition_Image_Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkEndCommandBuffer(Transition_Image_Layout2) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkEndCommandBuffer(Transition_Image_Layout2) Successful.\n");
	}

	memset((void*)&vkSubmitInfo_Transition_Image_Layout, 0, sizeof(vkSubmitInfo_Transition_Image_Layout));
	vkSubmitInfo_Transition_Image_Layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo_Transition_Image_Layout.pNext = NULL;
	vkSubmitInfo_Transition_Image_Layout.commandBufferCount = 1;
	vkSubmitInfo_Transition_Image_Layout.pCommandBuffers = &vkCommandBuffer_Transition_Image_Layout;

	vkResult = vkQueueSubmit(g_vkQueue, 1, &vkSubmitInfo_Transition_Image_Layout, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueSubmit(Transition_Image_Layout2) Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueSubmit(Transition_Image_Layout2) Successful.\n");
	}

	vkResult = vkQueueWaitIdle(g_vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueWaitIdle(Transition_Image_Layout2) Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkQueueWaitIdle(Transition_Image_Layout2) Successful.\n");
	}

	if (vkCommandBuffer_Transition_Image_Layout)
	{
		vkFreeCommandBuffers(g_vkDevice, g_vkCommandPool, 1, &vkCommandBuffer_Transition_Image_Layout);
		vkCommandBuffer_Transition_Image_Layout = VK_NULL_HANDLE;
	}

	// 7. Now staging buffer is not needed, hence release it's memory and it's handle.
	if (vkBuffer_stagingBuffer)
	{
		vkDestroyBuffer(g_vkDevice, vkBuffer_stagingBuffer, NULL);
		vkBuffer_stagingBuffer = VK_NULL_HANDLE;
	}

	if (vkDeviceMemory_StagingBuffer)
	{
		vkFreeMemory(g_vkDevice, vkDeviceMemory_StagingBuffer, NULL);
		vkDeviceMemory_StagingBuffer = NULL;
	}

	//8. Create Image View for above Image.
	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(vkImageViewCreateInfo));
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	// VkComponentMapping
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	// Subresource some part of the actual resource.
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// AspectMask => which part of the Image or whole of the Image is going to be affected by barrier.
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	vkImageViewCreateInfo.image = g_vkImage_texture;
	vkResult = vkCreateImageView(g_vkDevice, &vkImageViewCreateInfo, NULL, &g_vkImageView_texture);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateImageView(g_vkImageView_texture) Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateImageView(g_vkImageView_texture) Successful.\n");
	}

	//9. Create Texture sampler for above Image.
	VkSamplerCreateInfo vkSamplerCreateInfo;
	memset((void*)&vkSamplerCreateInfo, 0, sizeof(vkSamplerCreateInfo));
	vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	vkSamplerCreateInfo.pNext = NULL;
	vkSamplerCreateInfo.flags = 0;
	vkSamplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	vkSamplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
	vkSamplerCreateInfo.maxAnisotropy = 16;
	vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	vkSamplerCreateInfo.compareEnable = VK_FALSE;
	vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

	vkResult = vkCreateSampler(g_vkDevice, &vkSamplerCreateInfo, NULL, &g_vkSampler_texture);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateSampler(g_vkSampler_texture) Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateTexture -> vkCreateSampler(g_vkSampler_texture) Successful.\n");
	}


	return vkResult;
}

void MakeCheckImage(void)
{
	int iHeightCounter = 0;
	int iWidthCounter = 0;
	int iColorWhite = 0;

	for (iHeightCounter = 0; iHeightCounter < CHECKER_BOARD_HEIGHT; ++iHeightCounter)
	{
		for (iWidthCounter = 0; iWidthCounter < CHECKER_BOARD_WIDTH; ++iWidthCounter)
		{
			iColorWhite = (((iHeightCounter & 0x8) == 0) ^ ((iWidthCounter & 0x8) == 0)) * 255;
			g_bytCheckerBoard[iHeightCounter][iWidthCounter][0] = (byte)iColorWhite;
			g_bytCheckerBoard[iHeightCounter][iWidthCounter][1] = (byte)iColorWhite;
			g_bytCheckerBoard[iHeightCounter][iWidthCounter][2] = (byte)iColorWhite;
			g_bytCheckerBoard[iHeightCounter][iWidthCounter][3] = (byte)255;
		}
	}
}

VkResult CreateUniformBuffer(void)
{
	// Function Declarations
	VkResult UpdateUniformBuffer(void);

	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// Code
	memset((void*)&g_UniformData_straight, 0, sizeof(g_UniformData_straight));

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(vkBufferCreateInfo));
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0;
	vkBufferCreateInfo.size = sizeof(myUniformData);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	vkResult = vkCreateBuffer(g_vkDevice, &vkBufferCreateInfo, NULL, &g_UniformData_straight.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkCreateBuffer() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkCreateBuffer() Successful.\n");
	}

	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

	vkGetBufferMemoryRequirements(g_vkDevice, g_UniformData_straight.vkBuffer, &vkMemoryRequirements); // Region wise allocation which is alligned!

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0;// Initial value before entring into loop!

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(g_vkDevice, &vkMemoryAllocateInfo, NULL, &g_UniformData_straight.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkAllocateMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkAllocateMemory() Successful.\n");
	}

	// It binds vulkan device memory object habdle with vulkan buffer object handle.
	vkResult = vkBindBufferMemory(g_vkDevice, g_UniformData_straight.vkBuffer, g_UniformData_straight.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkBindBufferMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkBindBufferMemory() Successful.\n");
	}

	/// Buffer2
	memset((void*)&g_UniformData_straight_model, 0, sizeof(g_UniformData_straight_model));
	memset((void*)&vkBufferCreateInfo, 0, sizeof(vkBufferCreateInfo));
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0;
	vkBufferCreateInfo.size = sizeof(myModelMatData);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	vkResult = vkCreateBuffer(g_vkDevice, &vkBufferCreateInfo, NULL, &g_UniformData_straight_model.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkCreateBuffer() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkCreateBuffer() Successful.\n");
	}

	memset((void*)&vkMemoryRequirements, 0, sizeof(vkMemoryRequirements));

	vkGetBufferMemoryRequirements(g_vkDevice, g_UniformData_straight_model.vkBuffer, &vkMemoryRequirements); // Region wise allocation which is alligned!

	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(vkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0;// Initial value before entring into loop!

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(g_vkDevice, &vkMemoryAllocateInfo, NULL, &g_UniformData_straight_model.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkAllocateMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkAllocateMemory() Successful.\n");
	}

	// It binds vulkan device memory object habdle with vulkan buffer object handle.
	vkResult = vkBindBufferMemory(g_vkDevice, g_UniformData_straight_model.vkBuffer, g_UniformData_straight_model.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkBindBufferMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateUniformBuffer -> vkBindBufferMemory() Successful.\n");
	}

	return vkResult;
}

VkResult UpdateUniformBuffer(void)
{
	VkResult vkResult = VK_SUCCESS;
	myUniformData uniformData;
	myModelMatData modelMatData;
	memset((void*)&uniformData, 0, sizeof(uniformData));
	memset((void*)&modelMatData, 0, sizeof(modelMatData));

	// Straight
	modelMatData.modelMatrix = glm::mat4(1.0f);
	uniformData.viewMatrix = glm::mat4(1.0f);
	uniformData.projectionMatrix = glm::mat4(1.0f);

	glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);
	glm::mat4 translationMatrixMatrix = glm::mat4(1.0f);
	glm::mat4 rotationMatrixMatrix = glm::mat4(1.0f);

	perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)g_iWinHeight / (float)g_iWinHeight, 0.1f, 100.0f);

	translationMatrixMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -8.0f));

	perspectiveProjectionMatrix[1][1] = -1.0f * perspectiveProjectionMatrix[1][1];// (2n/t-d)

	uniformData.projectionMatrix = perspectiveProjectionMatrix;
	modelMatData.modelMatrix = translationMatrixMatrix * rotationMatrixMatrix;

	// Map Uniform buffer
	void* data = NULL;
	vkResult = vkMapMemory(g_vkDevice, g_UniformData_straight.vkDeviceMemory, 0, sizeof(myUniformData), 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "UpdateUniformBuffer -> vkMapMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	/*else
	{
		fprintf(g_pFile, "UpdateUniformBuffer -> vkMapMemory() Successful.\n");
	}*/

	// Actual memeory mapped I/O
	memcpy(data, (void*)&uniformData, sizeof(myUniformData));

	vkUnmapMemory(g_vkDevice, g_UniformData_straight.vkDeviceMemory);

	// g_UniformData_straight_model
	data = NULL;
	vkResult = vkMapMemory(g_vkDevice, g_UniformData_straight_model.vkDeviceMemory, 0, sizeof(myUniformData), 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "UpdateUniformBuffer -> vkMapMemory() Failed [%d].\n", vkResult);
		return (vkResult);
	}

	memcpy(data, (void*)&modelMatData, sizeof(myModelMatData));

	vkUnmapMemory(g_vkDevice, g_UniformData_straight_model.vkDeviceMemory);

	return vkResult;
}

VkResult CreateShaders(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	//Code
	// For Vertex Shader
	const char* szFileNameVS = "Shader.vert.spv";
	FILE* fp = NULL;
	size_t iSize = 0;
	char* pchShaderData = NULL;

	fp = fopen(szFileNameVS, "rb");
	if (fp == NULL)
	{
		fprintf(g_pFile, "CreateShaders -> fopen(Vertex Shader) Failed .\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateShaders -> fopen(Vertex Shader) Successful .\n");
	}

	fseek(fp, 0L, SEEK_END);

	iSize = ftell(fp);
	if (iSize == 0)
	{
		fprintf(g_pFile, "CreateShaders -> ftell(Vertex Shader Bin File) Failed, File Size %zd .\n", iSize);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	fseek(fp, 0L, SEEK_SET);

	pchShaderData = (char*)malloc(iSize * sizeof(char));
	if (pchShaderData == NULL)
	{
		fprintf(g_pFile, "CreateShaders -> MALLOC(pchShaderData) Failed .\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	size_t iRetVal = fread(pchShaderData, iSize, 1, fp);
	if (iRetVal != 1)
	{
		fprintf(g_pFile, "CreateShaders -> fread(Vertex Shader Data) Failed .\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateShaders -> fread(Vertex Shader Data) Successful .\n");
	}

	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}

	VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(vkShaderModuleCreateInfo));
	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0;// Reserved for future use.
	vkShaderModuleCreateInfo.codeSize = iSize;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)pchShaderData;

	vkResult = vkCreateShaderModule(g_vkDevice, &vkShaderModuleCreateInfo, NULL, &g_vkShaderModule_Vertex_Shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateShaders -> vkCreateShaderModule(Vertex Shader) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateShaders -> vkCreateShaderModule(Vertex Shader) Successful.\n");
	}

	if (pchShaderData)
	{
		free(pchShaderData);
		pchShaderData = NULL;
	}

	fprintf(g_pFile, "CreateShaders -> Vertex Shader Module Successfully Created.\n");

	// For Fragment Shader
	const char* szFileNameFS = "Shader.frag.spv";
	fp = NULL;
	iSize = 0;
	pchShaderData = NULL;

	fp = fopen(szFileNameFS, "rb");
	if (fp == NULL)
	{
		fprintf(g_pFile, "CreateShaders -> fopen(Fragment Shader) Failed .\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateShaders -> fopen(Fragment Shader) Successful .\n");
	}

	fseek(fp, 0L, SEEK_END);

	iSize = ftell(fp);
	if (iSize == 0)
	{
		fprintf(g_pFile, "CreateShaders -> ftell(Fragment Shader Bin File) Failed, File Size %zd .\n", iSize);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	fseek(fp, 0L, SEEK_SET);

	pchShaderData = (char*)malloc(iSize * sizeof(char));
	if (pchShaderData == NULL)
	{
		fprintf(g_pFile, "CreateShaders -> MALLOC(pchShaderData) Fragment Shader Failed .\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	iRetVal = fread(pchShaderData, iSize, 1, fp);
	if (iRetVal != 1)
	{
		fprintf(g_pFile, "CreateShaders -> fread(Fragment Shader Data) Failed .\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateShaders -> fread(Fragment Shader Data) Successful .\n");
	}

	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}

	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(vkShaderModuleCreateInfo));
	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0;// Reserved for future use.
	vkShaderModuleCreateInfo.codeSize = iSize;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)pchShaderData;

	vkResult = vkCreateShaderModule(g_vkDevice, &vkShaderModuleCreateInfo, NULL, &g_vkShaderModule_Fragment_Shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateShaders -> vkCreateShaderModule(Fragment Shader) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateShaders -> vkCreateShaderModule(Fragment Shader) Successful.\n");
	}

	if (pchShaderData)
	{
		free(pchShaderData);
		pchShaderData = NULL;
	}

	fprintf(g_pFile, "CreateShaders -> Fragment Shader Module Successfully Created.\n");


	return vkResult;
}


// SAM
VkResult CreateDescriptorSetLayout(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// Set 0: Camera matrices (UBO) - Vertex stage only
	VkDescriptorSetLayoutBinding set0Binding = {};
	set0Binding.binding = 0;
	set0Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set0Binding.descriptorCount = 1;
	set0Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	set0Binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo set0LayoutInfo = {};
	set0LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set0LayoutInfo.bindingCount = 1;
	set0LayoutInfo.pBindings = &set0Binding;

	VkDescriptorSetLayout set0Layout;
	vkResult = vkCreateDescriptorSetLayout(g_vkDevice, &set0LayoutInfo, nullptr, &set0Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateDescriptorSetLayout -> vkCreateDescriptorSetLayout(set0Layout) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateDescriptorSetLayout -> vkCreateDescriptorSetLayout(set0Layout) Successful.\n");
	}
	descriptorSetLayouts.push_back(set0Layout);

	// Set 1: Model matrix (UBO) - Vertex stage only  
	VkDescriptorSetLayoutBinding set1Binding = {};
	set1Binding.binding = 0;
	set1Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	set1Binding.descriptorCount = 1;
	set1Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	set1Binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo set1LayoutInfo = {};
	set1LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set1LayoutInfo.bindingCount = 1;
	set1LayoutInfo.pBindings = &set1Binding;

	VkDescriptorSetLayout set1Layout;
	vkResult = vkCreateDescriptorSetLayout(g_vkDevice, &set1LayoutInfo, nullptr, &set1Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateDescriptorSetLayout -> vkCreateDescriptorSetLayout(set1Layout) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateDescriptorSetLayout -> vkCreateDescriptorSetLayout(set1Layout) Successful.\n");
	}
	descriptorSetLayouts.push_back(set1Layout);

	// Set 2: Texture sampler (Combined Image Sampler) - Fragment stage only
	VkDescriptorSetLayoutBinding set2Binding = {};
	set2Binding.binding = 0;
	set2Binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	set2Binding.descriptorCount = 1;
	set2Binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	set2Binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo set2LayoutInfo = {};
	set2LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	set2LayoutInfo.bindingCount = 1;
	set2LayoutInfo.pBindings = &set2Binding;

	VkDescriptorSetLayout set2Layout;
	vkResult = vkCreateDescriptorSetLayout(g_vkDevice, &set2LayoutInfo, nullptr, &set2Layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateDescriptorSetLayout -> vkCreateDescriptorSetLayout(set2Layout) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateDescriptorSetLayout -> vkCreateDescriptorSetLayout(set2Layout) Successful.\n");
	}
	descriptorSetLayouts.push_back(set2Layout);

	return vkResult;
}

// SAM
VkResult CreatePipelineLayout(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(vkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0;
	vkPipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
	vkPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();// SAM
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	vkResult = vkCreatePipelineLayout(g_vkDevice, &vkPipelineLayoutCreateInfo, NULL, &g_vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreatePipelineLayout -> vkCreatePipelineLayout() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreatePipelineLayout -> vkCreatePipelineLayout() Successful.\n");
	}

	return vkResult;
}

// SAM
VkResult CreateDescriptorPool(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// Before creating actual descriptor pool Vulkan Expects Descriptor pool size.
	// 0th - mvp Uniform
	// 1st - texture and sampler
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},      // For sets 0 and 1
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1} // For set 2
	};

	// Create the POOL!
	VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
	memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(vkDescriptorPoolCreateInfo));
	vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	vkDescriptorPoolCreateInfo.pNext = NULL;
	vkDescriptorPoolCreateInfo.flags = 0;
	vkDescriptorPoolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
	vkDescriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
	vkDescriptorPoolCreateInfo.maxSets = 3;// SAM : We need 3 descriptor sets

	vkResult = vkCreateDescriptorPool(g_vkDevice, &vkDescriptorPoolCreateInfo, NULL, &g_vkDescriptorPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateDescriptorPool -> vkCreateDescriptorPool() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateDescriptorPool -> vkCreateDescriptorPool() Successful.\n");
	}

	return vkResult;
}

// Prepares Uniform Properly!
// SAM 
VkResult CreateDescriptorSet(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// Initialize Desctiptor Set Allocation Info
	VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
	memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(vkDescriptorSetAllocateInfo));
	vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	vkDescriptorSetAllocateInfo.pNext = NULL;
	vkDescriptorSetAllocateInfo.descriptorPool = g_vkDescriptorPool;
	vkDescriptorSetAllocateInfo.descriptorSetCount = (uint32_t)descriptorSetLayouts.size(); //NOTE : though we have 3 descriptors, two for mvp unifom ans another for texture sampler both are in one same descriptor set.
	vkDescriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();//SAM

	vkResult = vkAllocateDescriptorSets(g_vkDevice, &vkDescriptorSetAllocateInfo, descriptorSets.data());// SAM
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateDescriptorSet -> vkAllocateDescriptorSets() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateDescriptorSet -> vkAllocateDescriptorSets() Successful.\n");
	}

	VkDescriptorSet cameraSet = descriptorSets[0];    // Set 0
	VkDescriptorSet modelSet = descriptorSets[1];     // Set 1  
	VkDescriptorSet textureSet = descriptorSets[2];   // Set 2

	// 5. Update descriptor sets with actual resources
	std::vector<VkWriteDescriptorSet> descriptorWrites;

	// Set 0: Camera UBO
	VkDescriptorBufferInfo cameraBufferInfo = {};
	cameraBufferInfo.buffer = g_UniformData_straight.vkBuffer;
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = sizeof(myUniformData);

	VkWriteDescriptorSet cameraWrite = {};
	cameraWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	cameraWrite.dstSet = cameraSet;
	cameraWrite.dstBinding = 0;
	cameraWrite.dstArrayElement = 0;
	cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	cameraWrite.descriptorCount = 1;
	cameraWrite.pBufferInfo = &cameraBufferInfo;
	descriptorWrites.push_back(cameraWrite);

	// Set 1: Model UBO
	VkDescriptorBufferInfo modelBufferInfo = {};
	modelBufferInfo.buffer = g_UniformData_straight_model.vkBuffer;
	modelBufferInfo.offset = 0;
	modelBufferInfo.range = sizeof(myModelMatData);

	VkWriteDescriptorSet modelWrite = {};
	modelWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	modelWrite.dstSet = modelSet;
	modelWrite.dstBinding = 0;
	modelWrite.dstArrayElement = 0;
	modelWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelWrite.descriptorCount = 1;
	modelWrite.pBufferInfo = &modelBufferInfo;
	descriptorWrites.push_back(modelWrite);

	// Set 2: Texture Sampler
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = g_vkImageView_texture;
	imageInfo.sampler = g_vkSampler_texture;

	VkWriteDescriptorSet textureWrite = {};
	textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	textureWrite.dstSet = textureSet;
	textureWrite.dstBinding = 0;
	textureWrite.dstArrayElement = 0;
	textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	textureWrite.descriptorCount = 1;
	textureWrite.pImageInfo = &imageInfo;
	descriptorWrites.push_back(textureWrite);

	// Apply all updates
	fprintf(g_pFile, "CreateDescriptorSet -> vkCreateRenderPass() Just before Writing All Descriptors.\n");
	fflush(g_pFile);
	vkUpdateDescriptorSets(g_vkDevice, (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	fprintf(g_pFile, "CreateDescriptorSet -> vkCreateRenderPass() Finished Writing All Descriptors.\n");
	fflush(g_pFile);

	return vkResult;
}

VkResult CreateRenderpass(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// 1. Declare and initialize VkAttachmentDescription
	VkAttachmentDescription vkAttachmentDescription_array[2]; // Color + Depth
	memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * ARRAYSIZE(vkAttachmentDescription_array));
	// For color
	vkAttachmentDescription_array[0].flags = 0; // Use when on embedded board when there is very less Memory. VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT.
	vkAttachmentDescription_array[0].format = g_vkFormat_Color;
	vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;	   // Multi-sampling or not
	vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Akin to glClear(GL_COLOR_BIT)
	vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Applicable for depth buffer as well.
	vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	// What to do while operating on the Data arrangement, Remember about unpacking in OpenGL.
	vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// For Depth
	vkAttachmentDescription_array[1].flags = 0; // Use when on embedded board when there is very less Memory. VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT.
	vkAttachmentDescription_array[1].format = g_vkFormat_Depth;
	vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;	   // Multi-sampling or not
	vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Akin to glClear(GL_DEPTH_BIT)
	vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Applicable for depth buffer as well.
	vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// 2. Declare and Initialize VkAttachmentReference
	// For color Attachment
	VkAttachmentReference vkAttachmentReference_color;
	memset((void*)&vkAttachmentReference_color, 0, sizeof(vkAttachmentReference_color));
	vkAttachmentReference_color.attachment = 0; // Refer to 0th index of the above Structure Array.
	vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// For Depth Attachment
	VkAttachmentReference vkAttachmentReference_depth;
	memset((void*)&vkAttachmentReference_depth, 0, sizeof(vkAttachmentReference_depth));
	vkAttachmentReference_depth.attachment = 1; // Refer to 1st index of the above Structure Array.
	vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// 3. Declare and initialize VkSubpassDescription.
	// every process is made up of atleast main Thread.
	// every render pass is MUST have atleast one MAIN Thread.
	VkSubpassDescription vkSubpassDescription;
	memset((void*)&vkSubpassDescription, 0, sizeof(vkSubpassDescription));
	vkSubpassDescription.flags = 0;
	vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkSubpassDescription.inputAttachmentCount = 0;
	vkSubpassDescription.pInputAttachments = NULL;
	vkSubpassDescription.colorAttachmentCount = (uint32_t)1; // this count MUST be the count of VkAttachmentReference used for color!
	vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
	vkSubpassDescription.pResolveAttachments = NULL;
	vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth;
	vkSubpassDescription.preserveAttachmentCount = 0;
	vkSubpassDescription.pPreserveAttachments = NULL;

	// 4. Declare and Initialize VkRenderPassCreateInfo
	VkRenderPassCreateInfo vkRenderPassCreateInfo;
	memset((void*)&vkRenderPassCreateInfo, 0, sizeof(vkRenderPassCreateInfo));
	vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassCreateInfo.flags = 0;
	vkRenderPassCreateInfo.pNext = NULL;
	vkRenderPassCreateInfo.attachmentCount = (uint32_t)_ARRAYSIZE(vkAttachmentDescription_array);
	vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
	vkRenderPassCreateInfo.subpassCount = 1;
	vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
	vkRenderPassCreateInfo.dependencyCount = 0;
	vkRenderPassCreateInfo.pDependencies = NULL;

	vkResult = vkCreateRenderPass(g_vkDevice, &vkRenderPassCreateInfo, NULL, &g_vkRenderPass);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateRenderpass -> vkCreateRenderPass() Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateRenderpass -> vkCreateRenderPass() Successful.\n");
	}

	return vkResult;
}

// Creation of Pipeline State Object (PSO)
VkResult CreatePipeline(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// glGenBuffers(GL_ARRAY_BUFFER,...)
	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2];
	memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
	// For Position
	vkVertexInputBindingDescription_array[0].binding = 0;// corresponding to Location=0 in Vertex Shader
	vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3; //
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // GL_ARRAY_BUFFER/GL_ELEMENT_BUFFER what to consider the input Vertex or Index?
	// For TexCoord
	vkVertexInputBindingDescription_array[1].binding = 1;// corresponding to Location=1 in Vertex Shader
	vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 2; //
	vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2];
	memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
	// For Position
	vkVertexInputAttributeDescription_array[0].binding = 0; // About shader attribute
	vkVertexInputAttributeDescription_array[0].location = 0; // layout(location=0)
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // consider as x,y,z
	vkVertexInputAttributeDescription_array[0].offset = 0;

	// For TexCoord
	vkVertexInputAttributeDescription_array[1].binding = 1; // About shader attribute
	vkVertexInputAttributeDescription_array[1].location = 1; // layout(location=1)
	vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT; // consider as u,v
	vkVertexInputAttributeDescription_array[1].offset = 0;

	// Pipeline State Object
	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(vkPipelineVertexInputStateCreateInfo));
	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.flags = 0;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;


	// Input Assebmly State
	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
	memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(vkPipelineInputAssemblyStateCreateInfo));
	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0;
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	//vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	// Rasterizer State
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(vkPipelineRasterizationStateCreateInfo));
	vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0;
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;//Why do we need to give it?=> it is implementation dependent, at times rendering cal also be discarded if minimum value is not given,

	// Color Blend State
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
	//vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xf; // If we don't give this we will get No validation issue and No Output as well!
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
	memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(vkPipelineColorBlendStateCreateInfo));
	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.flags = 0;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

	// ViewPort and scissor state
	memset((void*)&g_vkViewport, 0, sizeof(g_vkViewport));
	g_vkViewport.x = 0;
	g_vkViewport.y = 0;
	g_vkViewport.width = (float)g_vkExtent2D_swapchain.width;
	g_vkViewport.height = (float)g_vkExtent2D_swapchain.height;
	g_vkViewport.minDepth = 0.0f;
	g_vkViewport.maxDepth = 1.0f;

	memset((void*)&g_vkRect2D_scissor, 0, sizeof(g_vkRect2D_scissor));
	g_vkRect2D_scissor.offset.x = 0;
	g_vkRect2D_scissor.offset.y = 0;
	g_vkRect2D_scissor.extent.width = g_vkExtent2D_swapchain.width;
	g_vkRect2D_scissor.extent.height = g_vkExtent2D_swapchain.height;

	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(vkPipelineViewportStateCreateInfo));
	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.flags = 0;
	vkPipelineViewportStateCreateInfo.viewportCount = 1; // We can give multiple viewports, 24 Sphere!!
	vkPipelineViewportStateCreateInfo.pViewports = &g_vkViewport;
	vkPipelineViewportStateCreateInfo.scissorCount = 1;
	vkPipelineViewportStateCreateInfo.pScissors = &g_vkRect2D_scissor;


	// Depth Stincil State
	// Depth Stincil State
	VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
	memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(vkPipelineDepthStencilStateCreateInfo));
	vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	vkPipelineDepthStencilStateCreateInfo.flags = 0;
	vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
	vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
	vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back;

	// Dynamic State
	// We don't have any dynamic state.

	// Multi-Sample State.
	// In red book it is null beacse they are not using Fragment Shader
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
	memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(vkPipelineMultisampleStateCreateInfo));
	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = 0;
	vkPipelineMultisampleStateCreateInfo.flags = 0;
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; //if we don't do this we get Validation Error. It is MUST!

	// Shader State
	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
	memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

	// Vertex shader
	vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;// IMP: It is important, as this has extensions API!
	vkPipelineShaderStageCreateInfo_array[0].flags = 0;
	vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkPipelineShaderStageCreateInfo_array[0].module = g_vkShaderModule_Vertex_Shader;
	vkPipelineShaderStageCreateInfo_array[0].pName = "main"; // Entry Point Function name.
	vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL; // If there are any constants, it will be pre-compiled constants. Standard Includes in DirectX

	// Fragment shader
	vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[1].flags = 0;
	vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkPipelineShaderStageCreateInfo_array[1].module = g_vkShaderModule_Fragment_Shader;
	vkPipelineShaderStageCreateInfo_array[1].pName = "main";
	vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;


	// Tessilation State:
	// We don't have tessilation Shaders so we can omit this!

	// as pipelines are created from pileinc cache, Now we will create Pipeline Cache!
	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(vkPipelineCacheCreateInfo));
	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;
	vkPipelineCacheCreateInfo.initialDataSize;
	vkPipelineCacheCreateInfo.pInitialData;

	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

	vkResult = vkCreatePipelineCache(g_vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreatePipeline -> vkCreatePipelineCache() Failed %d.\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreatePipeline -> vkCreatePipelineCache() Successful.\n");
	}

	// Create the actual Graphics Pipeline !!!!
	VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
	memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(vkGraphicsPipelineCreateInfo));
	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0;
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
	vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
	vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
	vkGraphicsPipelineCreateInfo.layout = g_vkPipelineLayout;
	vkGraphicsPipelineCreateInfo.renderPass = g_vkRenderPass;
	vkGraphicsPipelineCreateInfo.subpass = 0;
	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;

	vkResult = vkCreateGraphicsPipelines(g_vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &g_vkPipeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreatePipeline -> vkCreateGraphicsPipelines() Failed %d.\n", vkResult);
		if (vkPipelineCache)
		{
			vkDestroyPipelineCache(g_vkDevice, vkPipelineCache, NULL);
			vkPipelineCache = VK_NULL_HANDLE;
			fprintf(g_pFile, "CreatePipeline -> vkDestroyPipelineCache() Successful.\n");
		}
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreatePipeline -> vkCreateGraphicsPipelines() Successful.\n");
	}

	// We are done with Pipeline cache so destroy it!
	if (vkPipelineCache)
	{
		vkDestroyPipelineCache(g_vkDevice, vkPipelineCache, NULL);
		vkPipelineCache = VK_NULL_HANDLE;
		fprintf(g_pFile, "CreatePipeline -> vkDestroyPipelineCache() Successful.\n");
	}


	return vkResult;
}

VkResult CreateFrameBuffers(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	g_vkFramebuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * g_iSwapchainImageCount);
	if (g_vkFramebuffer_array == NULL)
	{
		fprintf(g_pFile, "CreateFrameBuffers() -> MALLOC for g_vkFramebuffer_array failed.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
	{
		// 1. Declare Array of VkImageView
		VkImageView vkImageView_attachments_array[2]; // 0->Color, 1-> Depth
		memset((void*)vkImageView_attachments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachments_array));

		// 2. CreateFramebuffer
		VkFramebufferCreateInfo vkFramebufferCreateInfo;
		memset((void*)&vkFramebufferCreateInfo, 0, sizeof(vkFramebufferCreateInfo));
		vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		vkFramebufferCreateInfo.flags = 0;
		vkFramebufferCreateInfo.pNext = NULL;
		vkFramebufferCreateInfo.renderPass = g_vkRenderPass;
		vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachments_array);
		vkFramebufferCreateInfo.pAttachments = vkImageView_attachments_array;
		vkFramebufferCreateInfo.width = g_vkExtent2D_swapchain.width;
		vkFramebufferCreateInfo.height = g_vkExtent2D_swapchain.height;
		vkFramebufferCreateInfo.layers = 1; // Case 2

		vkImageView_attachments_array[0] = g_SwapchainImageView_array[i];   // Color View
		vkImageView_attachments_array[1] = g_vkImageView_Depth;             // Depth View
		vkResult = vkCreateFramebuffer(g_vkDevice, &vkFramebufferCreateInfo, NULL, &g_vkFramebuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "CreateFrameBuffers -> Iteration %d vkCreateFramebuffer() Failed [%d].\n", i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "CreateFrameBuffers -> Iteration %d vkCreateFramebuffer() Successful.\n", i);
		}
	}

	return vkResult;
}

VkResult CreateSemaphores(void)
{
	/*
	 *    By default if no type is specified then the Semaphore which is created is binary Semaphore.
	 */
	 // Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
	memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(vkSemaphoreCreateInfo));
	vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo.flags = 0;
	vkSemaphoreCreateInfo.pNext = NULL;

	vkResult = vkCreateSemaphore(g_vkDevice, &vkSemaphoreCreateInfo, NULL, &g_VkSemaphore_backBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSemaphores -> vkCreateSemaphore(g_VkSemaphore_backBuffer) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSemaphores -> vkCreateSemaphore(g_VkSemaphore_backBuffer) Successful.\n");
	}

	vkResult = vkCreateSemaphore(g_vkDevice, &vkSemaphoreCreateInfo, NULL, &g_VkSemaphore_renderComplete);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "CreateSemaphores -> vkCreateSemaphore(g_VkSemaphore_renderComplete) Failed [%d].\n", vkResult);
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "CreateSemaphores -> vkCreateSemaphore(g_VkSemaphore_renderComplete) Successful.\n");
	}

	return vkResult;
}

// When the user wants to wait for some batch of commands then use Fences.
VkResult CreateFences(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	VkFenceCreateInfo vkFenceCreateInfo;
	memset((void*)&vkFenceCreateInfo, 0, sizeof(vkFenceCreateInfo));
	vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo.pNext = NULL;
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Not willing to wait for anything now.

	g_pvkFence_array = (VkFence*)malloc(sizeof(VkFence) * g_iSwapchainImageCount);
	if (g_pvkFence_array == NULL)
	{
		fprintf(g_pFile, "CreateFences() -> MALLOC for g_pvkFence_array failed.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
	{
		vkResult = vkCreateFence(g_vkDevice, &vkFenceCreateInfo, NULL, &g_pvkFence_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "CreateFences -> Iteration %d vkCreateFence() Failed [%d].\n", i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "CreateFences -> Iteration %d vkCreateFence() Successful.\n", i);
		}
	}

	return vkResult;
}

VkResult BuildCommandBuffers(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// 1. Start Loop per swapchain Image
	for (uint32_t i = 0; i < g_iSwapchainImageCount; i++)
	{
		// Reset Commandbuffers
		// 0=> Don't release the resources created by commandPool Created for these command buffers
		vkResult = vkResetCommandBuffer(g_vkCommandBuffer_array[i], 0);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "BuildCommandBuffers -> Iteration %d vkResetCommandBuffer() Failed [%d].\n", i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "BuildCommandBuffers -> Iteration %d vkResetCommandBuffer() Successful.\n", i);
		}

		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(vkCommandBufferBeginInfo));
		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0; // 1. Will use only Primary commandBuffer. 2. we are not going to usew this simultaniously with multiple commanBuffers.
		vkResult = vkBeginCommandBuffer(g_vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "BuildCommandBuffers -> Iteration %d vkBeginCommandBuffer() Failed [%d].\n", i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "BuildCommandBuffers -> Iteration %d vkBeginCommandBuffer() Successful.\n", i);
		}

		// Set clear values
		VkClearValue vkClearValue_array[2];
		memset((void*)&vkClearValue_array, 0, sizeof(VkClearColorValue) * _ARRAYSIZE(vkClearValue_array));
		vkClearValue_array[0].color = g_vkClearColorValue;
		vkClearValue_array[1].depthStencil = g_vkClearDepthStencilValue;

		VkRenderPassBeginInfo vkRenderPassBeginInfo;
		memset((void*)&vkRenderPassBeginInfo, 0, sizeof(vkRenderPassBeginInfo));
		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBeginInfo.pNext = NULL;
		vkRenderPassBeginInfo.renderPass = g_vkRenderPass;
		// Akin to D3D viewport
		vkRenderPassBeginInfo.renderArea.offset.x = 0;
		vkRenderPassBeginInfo.renderArea.offset.y = 0;
		vkRenderPassBeginInfo.renderArea.extent.width = g_vkExtent2D_swapchain.width;
		vkRenderPassBeginInfo.renderArea.extent.height = g_vkExtent2D_swapchain.height;
		vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
		vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
		vkRenderPassBeginInfo.framebuffer = g_vkFramebuffer_array[i];

		// Begin the render pass
		// VK_SUBPASS_CONTENTS_INLINE=> Contents of this render pass is inline with this render pass and part of primary Commandbuffer.
		vkCmdBeginRenderPass(g_vkCommandBuffer_array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		// Bind With Pipeline
		vkCmdBindPipeline(g_vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g_vkPipeline);

		// Straight
		// Bind Descriptotr Set to the pipeline
		vkCmdBindDescriptorSets(g_vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g_vkPipelineLayout, 0, 1, &descriptorSets[0], 0, nullptr);     // Set 0

		vkCmdBindDescriptorSets(g_vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g_vkPipelineLayout, 1, 1, &descriptorSets[1], 0, nullptr);      // Set 1

		vkCmdBindDescriptorSets(g_vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, g_vkPipelineLayout, 2, 1, &descriptorSets[2], 0, nullptr);    // Set 2

		// Bind with the vertex Position Buffer
		VkDeviceSize vkDeviceSize_offset_position[1];
		memset((void*)vkDeviceSize_offset_position, 0, _ARRAYSIZE(vkDeviceSize_offset_position) * sizeof(VkDeviceSize));
		vkCmdBindVertexBuffers(g_vkCommandBuffer_array[i], 0, 1, &g_VertexData_position.vkBuffer, vkDeviceSize_offset_position);

		// Bind with the vertex Color Buffer
		VkDeviceSize vkDeviceSize_offset_color[1];
		memset((void*)vkDeviceSize_offset_color, 0, _ARRAYSIZE(vkDeviceSize_offset_color) * sizeof(VkDeviceSize));
		vkCmdBindVertexBuffers(g_vkCommandBuffer_array[i], 1, 1, &g_VertexData_texcoord.vkBuffer, vkDeviceSize_offset_color);

		// Here we Must call Drawing functions.
		vkCmdDraw(g_vkCommandBuffer_array[i], 6, 1, 0, 0);

		vkCmdEndRenderPass(g_vkCommandBuffer_array[i]);

		vkResult = vkEndCommandBuffer(g_vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(g_pFile, "BuildCommandBuffers -> Iteration %d vkEndCommandBuffer() Failed [%d].\n", i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(g_pFile, "BuildCommandBuffers -> Iteration %d vkEndCommandBuffer() Successful.\n", i);
		}
	}

	return vkResult;
}

VkResult FillDeviceExtensionNames(void)
{
	// Varibale declarations
	VkResult vkResult = VK_SUCCESS;

	// Reteive count of Instances and keep this in local variable.
	uint32_t iDeviceExtensionCount = 0;
	vkResult = vkEnumerateDeviceExtensionProperties(g_vkPhysicalDevice_selected, NULL, &iDeviceExtensionCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "FillDeviceExtensionNames -> 1st call to vkEnumerateDeviceExtensionProperties() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "FillDeviceExtensionNames -> 1st call to vkEnumerateDeviceExtensionProperties() Successful.\n");
	}

	// 2. Allocate and fill struct VkExtensionProperties

	VkExtensionProperties* vkDeviceExtensionProperties_array = NULL;
	vkDeviceExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * iDeviceExtensionCount);
	if (vkDeviceExtensionProperties_array == NULL)
	{
		fprintf(g_pFile, "FillDeviceExtensionNames -> malloc() for VkExtensionProperties Failed.\n");
		return (VK_ERROR_INITIALIZATION_FAILED);
	}

	vkDeviceExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * iDeviceExtensionCount);
	vkResult = vkEnumerateDeviceExtensionProperties(g_vkPhysicalDevice_selected, NULL, &iDeviceExtensionCount, vkDeviceExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(g_pFile, "FillDeviceExtensionNames -> 2nd call to vkEnumerateInstanceExtensionProperties() Failed.\n");
		return (vkResult);
	}
	else
	{
		fprintf(g_pFile, "FillDeviceExtensionNames -> 2nd call to vkEnumerateInstanceExtensionProperties() Successful.\n");
	}

	// 3. Fill a string array obtained from VkExtensionProperties (Names of Extension)
	char** chDeviceExtensionNames_array = NULL;
	chDeviceExtensionNames_array = (char**)malloc(sizeof(char*) * iDeviceExtensionCount);
	if (chDeviceExtensionNames_array == NULL)
	{
	}

	for (uint32_t i = 0; i < iDeviceExtensionCount; i++)
	{
		chDeviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkDeviceExtensionProperties_array[i].extensionName) + 1);
		memcpy(chDeviceExtensionNames_array[i], vkDeviceExtensionProperties_array[i].extensionName, strlen(vkDeviceExtensionProperties_array[i].extensionName) + 1);
		fprintf(g_pFile, "FillDeviceExtensionNames -> Vulkan Extension Name= %s\n", chDeviceExtensionNames_array[i]);
	}

	// 4. Free vkExtensionProperties_array
	if (vkDeviceExtensionProperties_array)
	{
		free(vkDeviceExtensionProperties_array);
		vkDeviceExtensionProperties_array = NULL;
	}

	// 5. Fill Global varibales g_pchEnableInstanceExtensionNames_array
	VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < iDeviceExtensionCount; i++)
	{
		if (strcmp(chDeviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			vulkanSwapchainExtensionFound = VK_TRUE;
			g_pchEnableDeviceExtensionNames_array[g_iEnabledDeviceExtensionCount] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			g_iEnabledDeviceExtensionCount++;
		}
	}

	// 6. free local arrays
	for (uint32_t i = 0; i < iDeviceExtensionCount; i++)
	{
		free(chDeviceExtensionNames_array[i]);
		chDeviceExtensionNames_array[i] = NULL;
	}

	// 7.Display Wether required Instance extension Names are supourted or not
	if (vulkanSwapchainExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; // Return hard coded failure.
		fprintf(g_pFile, "FillDeviceExtensionNames -> VK_KHR_SWAPCHAIN_EXTENSION_NAME Not found \n");
		return vkResult;
	}
	else
	{
		fprintf(g_pFile, "FillDeviceExtensionNames -> VK_KHR_SWAPCHAIN_EXTENSION_NAME found \n");
	}

	// Print only enabled extension names
	for (uint32_t i = 0; i < g_iEnabledDeviceExtensionCount; i++)
	{
		fprintf(g_pFile, "FillDeviceExtensionNames -> Enabled Vulkan Instance Extension Name= %s\n", g_pchEnableInstanceExtensionNames_array[i]);
	}

	return vkResult;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT /*ID of flags*/,
	VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT /*What triggered the Debug call*/,
	uint64_t iObject/*Object*/, size_t iLocation/*Warning/Error Location in Memory*/,
	int32_t iMessageCode/*Message ID*/,
	const char* pchLayerPrefix/*Layer Name*/,
	const char* pchMessage/*Actual Error Message*/,
	void* pUserData/**/)
{
	// Code
	FILE* pFile = (FILE*)pUserData;

	fprintf(pFile, "SAM_Validation : debugReportCallback() -> %s (%d) = %s\n", pchLayerPrefix, iMessageCode, pchMessage);

	return VK_FALSE;// compulsary False so that we can get more and more Errors and Warnings. Until we don't have any errors and warnings.
}
