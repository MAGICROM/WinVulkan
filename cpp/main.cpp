//WINDOWS
#include "windows.h" // Windows main header
#include "math.h"
#include <hidusage.h> // Human Interface Device (Hi du Sage)

#include <iostream>
#include <chrono>

#include "sn_interface.h"

//Initialize pour win32
void sn_Wulkaninit(HINSTANCE hinstance,HWND hwnd);
//Cleanup
void sn_Vulkandestroy();
//Acquire and present
void sn_Vulkandraw();

void Mouse(RAWINPUT* raw, long timestamp);
bool Keyboard(RAWINPUT* raw, long timestamp);      

//uncomment USE_THREAD if you want wulkan on another thread
//#define USE_THREAD
//uncomment USE_IMGUI_PLEASE_IFYOUCAN if you want imgui
#define USE_IMGUI_PLEASE_IFYOUCAN

//***********************************************************************************************************************************************************************
//define USE_IMGUI_PLEASE_IFYOUCAN if you want imgui
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);   
#endif
//***********************************************************************************************************************************************************************

bool run_vulkan = true;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						
{																									

//***********************************************************************************************************************************************************************
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	ImGui_ImplWin32_WndProcHandler(hWnd,uMsg,wParam,lParam);   
#endif
//***********************************************************************************************************************************************************************

	unsigned char key = (unsigned char)wParam;
	
	switch (uMsg)
	{
	case WM_CLOSE:
		run_vulkan = false;
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		ValidateRect(hWnd, NULL);
		break;
	case WM_INPUT:
		{ 
		UINT dwSize;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
			OutputDebugString (TEXT("GetRawInputData does not return correct size !\n")); 

		RAWINPUT* raw = (RAWINPUT*)lpb;

		if (raw->header.dwType == RIM_TYPEKEYBOARD) 
		{
			Keyboard(raw, sn_interface.c_start);
	
			
			std::cout << "Keyboard: MakeCode=" << raw->data.keyboard.MakeCode
					<< " Flags=" << raw->data.keyboard.Flags
					<< " Reserved=" << raw->data.keyboard.Reserved
					<< " ExtraInformation=" << raw->data.keyboard.ExtraInformation	
					<< " Message=" << raw->data.keyboard.Message
					<< " VKey=" << raw->data.keyboard.VKey
					<< " Time=" << sn_interface.Keys[raw->data.keyboard.MakeCode].duration
					<< std::endl;
		}
		else if (raw->header.dwType == RIM_TYPEMOUSE) 
		{
			Mouse(raw, sn_interface.c_start);		
			/*std::cout << " lLastX=" << sn_interface.mdX
					  << " lLastY=" << sn_interface.mdY
					  << std::endl;*/
			/*std::cout << MouseBt[0].timestamp << std::endl;
			std::cout << MouseBt[1].timestamp << std::endl;
			std::cout << MouseBt[2].timestamp << std::endl;
			std::cout << MouseBt[3].timestamp << std::endl;
			std::cout << MouseBt[4].timestamp << std::endl;
			std::cout << "Mouse: usFlags=" << raw->data.mouse.usFlags
					<< " ulButtons=" << raw->data.mouse.ulButtons
					<< " usButtonFlags=" << raw->data.mouse.usButtonFlags
					<< " usButtonData=" << raw->data.mouse.usButtonData
					<< " ulRawButtons=" << raw->data.mouse.ulRawButtons
					<< " lLastX=" << raw->data.mouse.lLastX
					<< " lLastY=" << raw->data.mouse.lLastY
					<< " ulExtraInformation=" << raw->data.mouse.ulExtraInformation
					<< std::endl;*/
		} 
		
		//DefRawInputProc(&raw, 1 , sizeof(RAWINPUTHEADER));
		delete[] lpb; 
		
	}
		break;
	case WM_KEYDOWN:
		
		break;
	case WM_KEYUP:
		
		break;
	case WM_LBUTTONDOWN:
		
		break;
	case WM_RBUTTONDOWN:
		
		break;
	case WM_MBUTTONDOWN:
		
		break;
	case WM_LBUTTONUP:
		
		break;
	case WM_RBUTTONUP:
		
		break;
	case WM_MBUTTONUP:
		
		break;
	case WM_MOUSEWHEEL:
	{
		break;
	}
	case WM_MOUSEMOVE:
	{
		sn_interface.wX = LOWORD(lParam) - (sn_interface.destWidth / 2);
		sn_interface.wY = HIWORD(lParam) - (sn_interface.destHeight / 2);
		sn_interface.Wmousemove();
		
		//#ifdef USE_IMGUI_PLEASE_IFYOUCAN
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddMousePosEvent((float)LOWORD(lParam), HIWORD(lParam));
		//#endif

		/*std::cout << "Mouse :" 
				<< sn_interface.base
				<< " " 
				<< sn_interface.wfY
				<< " " 
				<< sn_interface.MouseBt[2].duration
				<< " " 
				<< sn_interface.MouseBt[3].duration
				<< " " 
				<< sn_interface.MouseBt[4].duration
			    << " W " 
				<< sn_interface.wheelDelta
				<< " fps "
				<< sn_interface.fps
				<< std::endl;*/


		break;
	}
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED))
			{
				sn_interface.ChangeSize(LOWORD(lParam),HIWORD(lParam));
				std::cout << "WM_SIZE CHANGE SIZE" << std::endl;
			}
			//sn_interface.ChangeSize(LOWORD(lParam),HIWORD(lParam));
		}
		else
		std::cout << "WM_SIZE SIZE_MINIMIZED"<< std::endl;
		break;
	case WM_GETMINMAXINFO:
	{
		LPMINMAXINFO minMaxInfo = (LPMINMAXINFO)lParam;
		minMaxInfo->ptMinTrackSize.x = 64;
		minMaxInfo->ptMinTrackSize.y = 64;
		break;
	}
	case WM_ENTERSIZEMOVE:
		std::cout << "WM_ENTERSIZEMOVE"<< std::endl;
		sn_interface.resizing = true;
		break;
	case WM_EXITSIZEMOVE:
		std::cout << "WM_EXITSIZEMOVE"<< std::endl;
		sn_interface.resizing = false;
		break;
	}
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												
}																			
typedef struct MyData {
    HWND hwnd;
    HINSTANCE hinstance;
} MYDATA, *PMYDATA;
DWORD WINAPI MyThreadFunction( LPVOID lpParam );
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wndClass;

	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "windows";
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		std::cout << "Could not register window class!\n";
		fflush(stdout);
		exit(1);
	}

	bool fullscreen = false;

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int wdWidth = 800;
	int wdHeight = 600;

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = fullscreen ? (long)screenWidth : (long)wdWidth;
	windowRect.bottom = fullscreen ? (long)screenHeight : (long)wdHeight;

	sn_interface.destWidth = windowRect.right;
	sn_interface.destHeight = windowRect.bottom;
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	LPCSTR name = "windows";
 	LPCSTR windowTitle = "Vulkan Example Window";

	HWND window = CreateWindowEx(0,
		name,
		windowTitle,
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hInstance,
		NULL);

	sn_Wulkaninit(hInstance,window);
	
	if (fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	ShowCursor(FALSE);
	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);

#ifdef USE_THREAD
	MYDATA pDataArray;
	pDataArray.hinstance = hInstance;
	pDataArray.hwnd = window;
	DWORD   dwThreadIdArray;
	HANDLE  hThreadArray = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            MyThreadFunction,       // thread function name
            &pDataArray,          	// argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray);   	// returns the thread identifier 
#endif





	RAWINPUTDEVICE Rid[2];
        
	Rid[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
	Rid[0].dwFlags = 0; //RIDEV_NOLEGACY;    // adds mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = window;

	Rid[1].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
	Rid[1].usUsage = 0x06;              // HID_USAGE_GENERIC_KEYBOARD
	Rid[1].dwFlags = 0; //RIDEV_NOLEGACY;    // adds keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = window;

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
	{
		std::cout << "Failed to register raw input device(s).\n";
	}

	
	MSG msg;
	bool quitMessageReceived = false;
	while (!quitMessageReceived) {
		
#ifndef USE_THREAD		
		sn_interface.Tick();
		sn_Vulkandraw();
#endif
		
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			
			if (msg.message == WM_QUIT) {
				run_vulkan = false;
				Sleep(100);
				quitMessageReceived = true;
				break;
			}
			
			
		}
	}
#ifndef USE_THREAD		
	sn_Vulkandestroy();
#endif

    return 0;
}
DWORD WINAPI MyThreadFunction( LPVOID lpParam ) 
{ 
   	HANDLE hStdout=GetStdHandle(STD_OUTPUT_HANDLE);
	PMYDATA pDataArray = (PMYDATA)lpParam;
	
	//sn_Wulkaninit(pDataArray->hinstance,pDataArray->hwnd);

	while (run_vulkan){
		sn_interface.Tick();
		sn_Vulkandraw();
		}
	sn_Vulkandestroy();
    return 0; 
} 

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//***********************************************************************************************************************************************************************
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imGuIZMOquat.h"
#include "imgui_impl_win32.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "imgui_impl_vulkan.h"
#endif
//***********************************************************************************************************************************************************************
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <array>
#include <vector>
#include <set>
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//     ### #####  ###  #####  ###   ###
//    #      #   #   #   #     #   # 
//    ####   #   #####   #     #   #
//       #   #   #   #   #     #   #
//    ###    #   #   #   #    ###   ###

	//RUNNING INFLIGHT
	
	uint32_t swap_imageCount;
	VkResult err;
	
	//PCIe
	
	uint32_t graphicsFamily{0};
	uint32_t presentFamily{0};
	uint32_t transferFamily{0};
	VkQueue graphicsQueue;
    VkQueue presentQueue;
	VkQueue transferQueue;
	
	//VULKAN API----------------------------------------------------------------
    
	VkInstance instance;
	
	//SCREEN--------------------------------------------------------------------
    
	VkSurfaceKHR surface;
	
	//GPU-----------------------------------------------------------------------
    
	VkPhysicalDevice carte_graphique = VK_NULL_HANDLE;
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
	
	//DRIVER VULKAN-------------------------------------------------------------
    
	VkDevice device;
   	
	//MEMORY--------------------------------------------------------------------

	
	//IMGUI---------------------------------------------------------------------
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	VkDescriptorPool imgui_pDescriptorPool;
	VkRenderPass imgui_renderPass;
#endif
	
#include"sn_interface.hpp"
#include"sn_utils.hpp"
#include"sn_wulkan.hpp"
#include"sn_buffer.hpp"
#include"sn_pipeline.hpp"
#include"sn_lwc.hpp"
#include"sn_swapchain.hpp"


