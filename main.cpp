//WINDOWS
#include "windows.h" // Windows main header
#include "math.h"
#include <hidusage.h> // Human Interface Device (Hi du Sage)

#include <iostream>
#include <chrono>

//Initialize pour win32
void sn_Wulkaninit(HINSTANCE hinstance,HWND hwnd);
//Cleanup
void sn_Vulkandestroy();
//Acquire and present
void sn_Vulkandraw();
      
#define DELAY_FAST 0.15f
//#define USE_THREAD

//Chaque touche et bouton
struct sn_Key
{
	bool pressed = false;
	bool fastclick = false;
	long long timestamp = 0;
	float duration = 0;

void input_pressed(long time)
	{																				
	if(duration < DELAY_FAST)
		fastclick = true;
	if(!pressed)
		{
		duration = 0.f;
		pressed = true;
		}
	timestamp = time;
	}

bool input_released(long time)
	{																				
	if(duration < DELAY_FAST && fastclick)
		{
		fastclick = false;	//double click
		pressed = false;
		return true;
		}
	fastclick = false;
	pressed = false;
	return false;
	}
};

struct sn_Interface
{	
	bool _no_initialized = true; //For initialisation
	long long c_start;	//Actual high precision time
	float delta_time;	//temps lissé par intégration
	float fps;			
	
	sn_Key Keys[256];
	sn_Key MouseBt[5];
	short wheelDelta = 0;
	long wX = 0; 		//Window
	long wY = 0;
	
	long mdX = 0; 		//Delta de HID
	long mdY = 0;

	long mX = 0; 		//Delta de HID ou Position de HID
	long mY = 0;
	
	float radX = 0.f; 	//Integration de HID borné sur -1 a 1
	float radY = 0.f;

	float wfX = 0.f; 		//Window -1 1 et aspect ration
	float wfY = 0.f;

	float aspectH = 0.f; 		//Window -1 1 et aspect ration
	float aspectV = 0.f;
	float base = 400.f;
	float baseexp = 0;
	
	float aspectratio;
	unsigned short destWidth = 0;
	unsigned short destHeight = 0;
	
	bool resizing = true;

void 		Wmousemove(void);
void 		ChangeSize(unsigned short Width,unsigned short Height);
long long 	Tick();
bool 		Button_pressed(RAWINPUT* raw,int* pnum);
void 		Mouse(RAWINPUT* raw, long timestamp);
bool 		Keyboard(RAWINPUT* raw, long timestamp);

}sn_interface;

bool run_vulkan = true;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)						
{																									\

	#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	ImGui_ImplWin32_WndProcHandler(hWnd,uMsg,wParam,lParam);   
	#endif

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
			sn_interface.Keyboard(raw, sn_interface.c_start);
	
			
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
			sn_interface.Mouse(raw, sn_interface.c_start);		

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
		
		#ifdef USE_IMGUI_PLEASE_IFYOUCAN
		ImGuiIO& io = ImGui::GetIO();
		io.AddMousePosEvent((float)LOWORD(lParam), HIWORD(lParam));
		#endif

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
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));												\
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

#define USE_IMGUI_PLEASE_IFYOUCAN
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
#include "imgui.h"
#include "imgui_impl_win32.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include "imgui_impl_vulkan.h"
#endif

#include <glm/glm.hpp>
#include <glm/ext.hpp>

void sn_Interface::Wmousemove(void)
{
	wfX = float(wX) / float(destWidth) * 2.f;
	wfY = float(wY) / float(destHeight) * 2.f;
	
}
void sn_Interface::ChangeSize(unsigned short Width,unsigned short Height)
{
	
	destWidth = Width;
	destHeight = Height;
	aspectratio = (float)Height/(float)Width;
	aspectH = base / destWidth; 		
	aspectV = base / Height; 

	#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)destWidth, (float)destHeight);
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
	#endif
	resizing = false;
}

long long sn_Interface::Tick()
	{
	static LARGE_INTEGER Frequency;
	static LARGE_INTEGER time_start;
	static LARGE_INTEGER time_now;
	static int pos = 0;
	static float fpss[128];

	if(_no_initialized)
		{
			QueryPerformanceFrequency(&Frequency); 
			QueryPerformanceCounter(&time_start);
			_no_initialized = false; 
		}
	
	//Update time
	
	QueryPerformanceCounter(&time_now); 
	time_now.QuadPart -= time_start.QuadPart;
	time_now.QuadPart *= 1000000;
	time_now.QuadPart /= Frequency.QuadPart;

	long long Tick = time_now.QuadPart - c_start;
	c_start = time_now.QuadPart;

	fpss[pos] = (float)Tick;
	fpss[pos] *= 0.000001f;
	pos++;
	if(pos>127)pos=0;
	
	fps = 0.0f;
	for(volatile int i = 0; i < 128; i++)fps += fpss[i];
	fps /= 128.0f;
	delta_time = fps;
	fps = 1.0f / delta_time; 
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	//ImGuiIO& io = ImGui::GetIO();
	//io.DeltaTime = delta_time;
#endif
	if(MouseBt[0].pressed)MouseBt[0].duration = c_start - MouseBt[0].timestamp;
	if(MouseBt[1].pressed)MouseBt[1].duration = c_start - MouseBt[1].timestamp;
	if(MouseBt[2].pressed)MouseBt[2].duration = c_start - MouseBt[2].timestamp;
	if(MouseBt[3].pressed)MouseBt[3].duration = c_start - MouseBt[3].timestamp;
	if(MouseBt[4].pressed)MouseBt[4].duration = c_start - MouseBt[4].timestamp;
	
	for(volatile int i = 0; i < 256; i++)
	{
		if(Keys[i].pressed)
		{
			Keys[i].duration += delta_time;
		}
	}

	return Tick;
	}
	bool sn_Interface::Button_pressed(RAWINPUT* raw,int* pnum)
	{
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN){*pnum=0;return true;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP){*pnum=0;return false;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN){*pnum=1;return true;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP){*pnum=1;return false;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN){*pnum=2;return true;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP){*pnum=2;return false;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN){*pnum=3;return true;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP){*pnum=3;return false;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN){*pnum=4;return true;}
		if(raw->data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP){*pnum=4;return false;}
		return false;
	}
	
	#define MOUSE_INPUT(x, maskpressed, maskreleased) \
	if(raw->data.mouse.usButtonFlags & maskpressed)MouseBt[x].input_pressed(timestamp); \
	if(raw->data.mouse.usButtonFlags & maskreleased)MouseBt[x].input_released(timestamp); 	

	void sn_Interface::Mouse(RAWINPUT* raw, long timestamp)
	{
		if(raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			mdY = raw->data.mouse.lLastY - mY;
			mdX = raw->data.mouse.lLastX - mX;
		}
		else
		{
			mdY = raw->data.mouse.lLastY;
			mdX = raw->data.mouse.lLastX;
		}
		mX = raw->data.mouse.lLastX;
		mY = raw->data.mouse.lLastY;
		radX += (float)mdX * 0.00152f;
		radY += (float)mdY * 0.00152f;
		if(radX > 1.f)radX = 1.f;
		if(radX < -1.f)radX = -1.f;
		if(radY > 1.f)radY = 1.f;
		if(radY < -1.f)radY = -1.f;

		MOUSE_INPUT(0, RI_MOUSE_LEFT_BUTTON_DOWN, RI_MOUSE_LEFT_BUTTON_UP)
		MOUSE_INPUT(1, RI_MOUSE_RIGHT_BUTTON_DOWN, RI_MOUSE_RIGHT_BUTTON_UP)
		MOUSE_INPUT(2, RI_MOUSE_MIDDLE_BUTTON_DOWN, RI_MOUSE_MIDDLE_BUTTON_UP)
		MOUSE_INPUT(3, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP)
		MOUSE_INPUT(4, RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP)
		
		#ifdef USE_IMGUI_PLEASE_IFYOUCAN
		int button = -1;
		if(Button_pressed(raw,&button))
		{
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddMouseButtonEvent(button,true);
		}
		else if(button != -1)
		{
		//ImGuiIO& io = ImGui::GetIO();
		//io.AddMouseButtonEvent(button,false);
		}
		#endif

		if(raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			wheelDelta += (short)raw->data.mouse.usButtonData;
			baseexp += ((float)(short)raw->data.mouse.usButtonData)*.001f;
			aspectH = base / destWidth; 		
			aspectV = base / destHeight; 
			base = 400.f * exp(baseexp);
		}
	}
bool sn_Interface::Keyboard(RAWINPUT* raw, long timestamp)
	{
		if(raw->data.keyboard.Flags == 0)
			Keys[raw->data.keyboard.MakeCode].input_pressed(timestamp);
		else
			return Keys[raw->data.keyboard.MakeCode].input_released(timestamp);
		return false;
	}
	
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normale;
	glm::vec2 uv;
};

struct Camera
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 up = glm::vec3(0.0f,0.1f,0.0f);
	glm::vec3 forward;
	glm::vec3 right;
	
	glm::mat4 projection = glm::mat4(0.0);
	glm::quat orientation = glm::quat(glm::vec3(0.0f));
	
	float focale = 50.f; //mm
	float fov = 2.f * glm::atan(50.f/2*focale);
	
	float z_near = 0.0f;
	float z_far = 10.0f;
	
	glm::mat4 LookAtLH(const glm::vec3& to)
	{
		orientation = glm::lookAtLH(position,to,up);
		projection = glm::perspectiveLH(fov,
										sn_interface.aspectratio,
										z_near,
										z_far);

		glm::mat4 Translation = glm::translate(glm::mat4(1.0), -position);
		
		glm::mat4 Rotation = glm::mat4_cast(orientation);
		right = glm::vec3(Rotation[0][0],Rotation[1][0],Rotation[2][0]);
		forward = glm::vec3(Rotation[0][2],Rotation[1][2],Rotation[2][2]);
		
		glm::mat4 World = Rotation * Translation;
		
		return World;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <vector>
#include <set>

struct snVulkan
{
	VkPipeline Pipe;
	VkDevice mdevice;
	operator VkDevice&(){return mdevice;};
	operator VkPipeline&(){return Pipe;};
}device_test;

	uint32_t swap_imageCount;
	VkResult err;
	uint32_t graphicsFamily{0};
	uint32_t presentFamily{0};
	
    //VULKAN API
    VkInstance instance;

	//SCREEN
    VkSurfaceKHR surface;

	//GPU
    VkPhysicalDevice carte_graphique = VK_NULL_HANDLE;
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
	
	//DRIVER VULKAN
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkRenderPass renderPass;
	//VkRenderPass renderPass_imgui;

	//MEMORY
	VkDescriptorPool descriptorPool;
	#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	VkDescriptorPool imgui_pDescriptorPool;
	VkCommandPool imgui_CommandPool;
	VkCommandBuffer imgui_CommandBuffer;
	VkRenderPass imgui_renderPass;
	#endif
	

	void rebuild(bool);
	void Buffers(uint32_t swap_imageCount);
	void DestroyBuffers();

struct PipeModel{

	VkPipelineLayout 							PipeLayout;
	VkPipeline 									Pipe;

	VkShaderModuleCreateInfo					*Shaders;		  //[]		
	VkPipelineShaderStageCreateInfo             *ShaderStageInfo; //[]		
	VkDescriptorSetLayoutBinding   				*bindings;		  //[]
	VkDynamicState                 				*dynamicStates;   //[]
	
	VkPipelineVertexInputStateCreateInfo        vertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo      inputAssembly{};
	VkPipelineViewportStateCreateInfo           viewportState{};
	VkPipelineRasterizationStateCreateInfo      rasterizer{};
	VkPipelineMultisampleStateCreateInfo        multisampling{};
	VkPipelineColorBlendAttachmentState         colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo         colorBlending{};
	VkPipelineDynamicStateCreateInfo            dynamicState{};
	
	VkDescriptorSetLayoutCreateInfo             set_layout{};
	VkPipelineLayoutCreateInfo                  pipelineLayoutInfo{};
	VkGraphicsPipelineCreateInfo                pipelineInfo{};
	
	void PipeModel_Clear(){
			memset(this,0,sizeof(PipeModel));
		}
	void PipeModel_CreateBindings(size_t numbers)
		{
        	set_layout.bindingCount = numbers;
			pipelineLayoutInfo.setLayoutCount = 1;
			bindings = new VkDescriptorSetLayoutBinding[set_layout.bindingCount];
			for(volatile int i = 0; i < set_layout.bindingCount; i++)
				{
				//Segmentation fault prevention 1
				memset(&bindings[i],0,sizeof(VkDescriptorSetLayoutBinding));
				bindings[i].binding = 0;
				bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[i].descriptorCount = 1;
				bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				bindings[i].pImmutableSamplers = NULL;
			}
			set_layout.pBindings = bindings;
		}
	void PipeModel_CreateShaders(size_t numbers)
		{
			pipelineInfo.stageCount = numbers;
			ShaderStageInfo = new VkPipelineShaderStageCreateInfo[pipelineInfo.stageCount];
			Shaders = new VkShaderModuleCreateInfo[pipelineInfo.stageCount];
			pipelineInfo.pStages = ShaderStageInfo;

			for(volatile int i = 0; i < pipelineInfo.stageCount; i++)
			{
				//Segmentation fault prevention 1
				memset(&ShaderStageInfo[i],0,sizeof(VkPipelineShaderStageCreateInfo));
				//Segmentation fault prevention 2
				memset(&Shaders[i],0,sizeof(VkShaderModuleCreateInfo));

				Shaders[i].sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				Shaders[i].pCode = nullptr;  
				Shaders[i].codeSize = 0;
				
				ShaderStageInfo[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
       			ShaderStageInfo[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
        		ShaderStageInfo[i].module = nullptr;
			}
		}
	void PipeModel_PreFill()
	{
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        dynamicStates = new VkDynamicState[2];
		dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
       	dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

    	set_layout.bindingCount = 0;
        set_layout.pBindings = nullptr;

		pipelineLayoutInfo ={
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pSetLayouts = 0,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = 0
		};

        pipelineInfo ={
			.stageCount = 0,
			.pStages = 0,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = 0,
			.renderPass = 0,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};
	}
	void PipeModel_Proceed(VkRenderPass rp)
	{
		for(volatile int i = 0; i < pipelineInfo.stageCount; i++)
		{
			VkShaderModuleCreateInfo createInfoSHD{};
			createInfoSHD.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfoSHD.codeSize = Shaders[i].codeSize;
			createInfoSHD.pCode = Shaders[i].pCode;
			createInfoSHD.flags = Shaders[i].flags;
			
			if (vkCreateShaderModule(device, &createInfoSHD, nullptr, &ShaderStageInfo[i].module) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader!");
			}
		}
		
		if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &PipeLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
		}

		pipelineInfo.layout = PipeLayout;
		pipelineInfo.renderPass = rp;
		
		if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Pipe) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
		}


		for(volatile int i = 0; i < pipelineInfo.stageCount; i++)
		{
		vkDestroyShaderModule(device, ShaderStageInfo[i].module, nullptr);
		}
		
		if(ShaderStageInfo)delete [] ShaderStageInfo;
		if(dynamicStates)delete [] dynamicStates;
		if(bindings)delete [] bindings; 
	};
};
void EcranOn();
void EcranOff();

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

void sn_Wulkaninit(HINSTANCE hinstance,HWND hwnd)
{
    std::vector<const char*> extensions;
    std::vector<const char*> layers;
    std::vector<const char*> extension_names;

    VkLayerProperties *instance_layers{NULL};
    VkExtensionProperties *instance_extensions{};
	const char **required_extensions{NULL};
	
	uint32_t queueFamilyCount{0};
    uint32_t deviceCount{};
    uint32_t instance_layer_count{0};
    uint32_t required_extension_count{0};
    uint32_t instance_extension_count{0};
        
    auto t_start = std::chrono::high_resolution_clock::now();
    auto t_now = std::chrono::high_resolution_clock::now();
        
    const char *instance_validation_layers_alt1[] = {"VK_LAYER_LUNARG_standard_validation"};
    vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
    
	const char *instance_needed_layers[] = {"VK_LAYER_LUNARG_standard_validation"};

	char **instance_validation_layers = (char**) instance_validation_layers_alt1;
        if (instance_layer_count > 0) {
            instance_layers = (VkLayerProperties*)malloc(sizeof (VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count,instance_layers);
            for(uint32_t i=0;i<instance_layer_count;i++)
            {
                layers.push_back(instance_layers[i].layerName);
            }
        }
		
	required_extension_count = 2; //VK_KHR_surface, VK_KHR_win32_surface, VK_EXT_debug_utils
	
    for(uint32_t i=0;i<instance_layer_count;i++)
        {
			if (required_extension_count > 0) {
            instance_extensions = (VkExtensionProperties*)malloc(sizeof (VkExtensionProperties) * instance_extension_count);
            err = vkEnumerateInstanceExtensionProperties(layers[i], &instance_extension_count, instance_extensions);
            for(uint32_t i=0;i<instance_extension_count;i++)
            {
                extensions.push_back(instance_extensions[i].extensionName);
				std::cout << "Extension: " << instance_extensions[i].extensionName << std::endl;
			}
			free(instance_extensions);
        	}
		}
	
	extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "WinVulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Spacenet";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instancecreateInfo = {};
	instancecreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instancecreateInfo.pApplicationInfo = &appInfo;
	instancecreateInfo.enabledExtensionCount = static_cast<uint32_t>(extension_names.size());
    instancecreateInfo.ppEnabledExtensionNames = extension_names.data();
	//fossilize
	//instancecreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	//no fossilize
	instancecreateInfo.enabledLayerCount = 0;
	instancecreateInfo.ppEnabledLayerNames = layers.data();
	
	err = vkCreateInstance(&instancecreateInfo, nullptr, &instance);
	if (err != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan instance!" << std::endl;
		return;
	}

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hinstance;
	surfaceCreateInfo.hwnd = hwnd;
	err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
	
	//Combien de Gpus ?      
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	
	//Teste tous les gpus
	for (const auto& tested_device : devices) 
	{
		vkGetPhysicalDeviceQueueFamilyProperties(tested_device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(tested_device, &queueFamilyCount, queueFamilies.data());
	
		uint32_t n_queue = 0;
		
		for (const auto& queueFamily : queueFamilies) {
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				VkBool32 presentSupport = false;
				graphicsFamily = n_queue;
				vkGetPhysicalDeviceSurfaceSupportKHR(tested_device, n_queue, surface, &presentSupport);
				if (presentSupport) {
					//Ce GPU est Bon pour cette surface
					carte_graphique = tested_device;
					presentFamily = n_queue;
					n_queue++;
					}
				}
			}
	}
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {graphicsFamily, presentFamily};

	float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}
            
        // Available extensions and layers names
        const char* const* _ppExtensionNames = NULL;    
        // get extension names
        uint32_t _extensionCount = 0;

        vkEnumerateDeviceExtensionProperties( carte_graphique, NULL, &_extensionCount, NULL);
        std::vector<const char *> extNames;
        std::vector<VkExtensionProperties> extProps(_extensionCount);
        vkEnumerateDeviceExtensionProperties( carte_graphique, NULL, &_extensionCount, extProps.data());
        for (uint32_t i = 0; i < _extensionCount; i++) {
            extNames.push_back(extProps[i].extensionName);
			std::cout << "Extension: " << extProps[i].extensionName << std::endl;
        }
        _ppExtensionNames = extNames.data();
    
        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.pEnabledFeatures = &deviceFeatures;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
            createInfo.ppEnabledExtensionNames = extNames.data();
            //createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
            //createInfo.ppEnabledLayerNames = layers.data();
            
            err = vkCreateDevice(carte_graphique, &createInfo, nullptr, &device);
        if(err != VK_SUCCESS){
            std::runtime_error("failed to create logical device!");
        }
        
		VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
		
		vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
        vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
        
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(carte_graphique, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(carte_graphique, surface, &formatCount, nullptr);
        if (formatCount != 0) {
                formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(carte_graphique, surface, &formatCount, formats.data());
            }
        uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(carte_graphique, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
                presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(carte_graphique, surface, &presentModeCount, presentModes.data());
            }

		VkDescriptorPoolSize descriptorPoolSizes[] =
		{	{ VK_DESCRIPTOR_TYPE_SAMPLER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 }
		};

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
			descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCI.maxSets = 10 * (sizeof(descriptorPoolSizes)/sizeof(*descriptorPoolSizes));
			descriptorPoolCI.poolSizeCount = (uint32_t)(sizeof(descriptorPoolSizes)/sizeof(*descriptorPoolSizes));
			descriptorPoolCI.pPoolSizes = descriptorPoolSizes;
	
		err = vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &descriptorPool);
		swap_imageCount = capabilities.minImageCount + 1;

		#ifdef USE_IMGUI_PLEASE_IFYOUCAN
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        ImGui::StyleColorsDark();
        //ImGui_ImplGlfw_InitForVulkan(window, true);
		// Setup scaling
    	// Make process DPI aware and obtain main monitor scale
   		ImGui_ImplWin32_EnableDpiAwareness();
    	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
	
		ImGuiStyle& style = ImGui::GetStyle();
    	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    	style.FontScaleDpi = main_scale;    

        // Create Descriptor Pool
    	// If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
		{
			static VkAllocationCallbacks*   g_Allocator = nullptr;
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 0;
			for (VkDescriptorPoolSize& pool_size : pool_sizes)
				pool_info.maxSets += pool_size.descriptorCount;
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			err = vkCreateDescriptorPool(device, &pool_info, g_Allocator, &imgui_pDescriptorPool);
			check_vk_result(err);
		}
		
		
		

		if (capabilities.maxImageCount > 0 && swap_imageCount > capabilities.maxImageCount) {
				swap_imageCount = capabilities.maxImageCount;
			}

							
			VkAttachmentDescription Attachments{};

            Attachments.format = VK_FORMAT_B8G8R8A8_UNORM;
            Attachments.samples = VK_SAMPLE_COUNT_1_BIT;
            Attachments.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            Attachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            Attachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            Attachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            Attachments.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            Attachments.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference color_Ref{};
            color_Ref.attachment = 0;
            color_Ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			
			VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_Ref;
			
			VkRenderPassCreateInfo renderPassInfo_imgui{};
            renderPassInfo_imgui.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo_imgui.attachmentCount = 1;
            renderPassInfo_imgui.pAttachments = &Attachments;
            renderPassInfo_imgui.subpassCount = 1;
            renderPassInfo_imgui.pSubpasses = &subpass;
            renderPassInfo_imgui.dependencyCount = 0;

            err = vkCreateRenderPass(device, &renderPassInfo_imgui, nullptr, &imgui_renderPass);
            if( err!= VK_SUCCESS){
                    throw std::runtime_error("failed to create render pass!!");
                }
				
			ImGui_ImplVulkan_InitInfo dearimgui{};
            dearimgui.Instance = instance;
            dearimgui.Device = device;
            dearimgui.PhysicalDevice = carte_graphique;  
            dearimgui.ImageCount = swap_imageCount;
            dearimgui.MinImageCount = swap_imageCount;
            dearimgui.Queue = graphicsQueue;
            dearimgui.QueueFamily = graphicsFamily;
            dearimgui.DescriptorPool = imgui_pDescriptorPool;
			dearimgui.PipelineInfoMain.RenderPass = imgui_renderPass;
			dearimgui.PipelineInfoMain.Subpass = 0;
			dearimgui.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			dearimgui.CheckVkResultFn = check_vk_result;
        
        
        ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplVulkan_Init(&dearimgui);
		#endif
		
		
		EcranOn();
		rebuild(false);
}
uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
{
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(carte_graphique, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

        throw std::runtime_error("failed to find suitable memory type!");
}
struct Uniform
{
    bool transfert;
    void * mem_ptr;
    alignas(16)
	uint32_t mem_size;

    VkBuffer buffer;
    VkDeviceMemory mem;

void CreateCopyBuffer(void *Src,uint32_t size){
    
    VkBufferCreateInfo ubo{};
            ubo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            ubo.size = size;
            if(transfert)
                ubo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            else
                ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            ubo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    //Buffer creation
    err = vkCreateBuffer(device,&ubo,nullptr,&buffer);
    
    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device,buffer,&mem_reqs);
    VkMemoryAllocateInfo mem_info{};
            mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_info.memoryTypeIndex = findMemoryType(mem_reqs.memoryTypeBits, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            mem_info.allocationSize = mem_reqs.size;
    err = vkAllocateMemory(device,&mem_info,nullptr,&mem);
    err = vkBindBufferMemory(device, buffer, mem, 0);
    //Buffer mapping and copy
    err = vkMapMemory(device,mem,0,VK_WHOLE_SIZE,0,&mem_ptr);
    if(Src)memcpy(mem_ptr, Src, size);
    if(transfert)
        {
        VkMappedMemoryRange range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = mem,
        .size = size
		};
        err = vkFlushMappedMemoryRanges(device, 1, &range);
        if(Src)vkUnmapMemory(device,mem);
        }

    mem_size = size;
	}
};
void sn_Vulkandestroy()
{
	vkDeviceWaitIdle(device);

	#ifdef USE_IMGUI_PLEASE_IFYOUCAN           
	   	ImGui_ImplVulkan_Shutdown();
		vkFreeCommandBuffers(device,imgui_CommandPool,1,&imgui_CommandBuffer);
		vkDestroyCommandPool(device, imgui_CommandPool, nullptr);
		vkDestroyRenderPass(device, imgui_renderPass, nullptr);
	#endif

    EcranOff();
	vkDestroyDescriptorPool(device,descriptorPool,NULL);  
	vkDestroyDevice(device,NULL);
    vkDestroySurfaceKHR(instance,surface,NULL);
    vkDestroyInstance(instance, NULL);
}
VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features,VkPhysicalDevice physicalDevice) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }
VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
        return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
            physicalDevice
        );
    }

#include <fstream>
static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
        }

void Read_Obj()
{
	std::vector<char> object = readFile("suzan.obj");
	if(!object.size())return; //PAS DE fiCHIER
	std::vector<char> line;
	bool newline = true;
	
	std::vector<float> uvs;
	std::vector<float> vertex;
	std::vector<float> normales;
	std::vector<std::vector<char>> faces;
	
	int uvs_vec_size = 0;
	int vertex_vec_size = 0;
	int normales_vec_size = 0;
	int faces_vec_size = 0;
	int vec_size = 0;
	int uvs_size = 0;
	int vertex_size = 0;
	int normale_size = 0;

	int j = 0; //index dans la ligne
	std::vector<std::vector<char>> tokens;
	std::vector<char> token;
	for(volatile int i = 0;i < object.size();i++)//Jusqu'a la fin du fichier
	{
		if(object[i] != '\n'){
		line.push_back(object[i]); //Pousse chaque caracteres de la ligne
		//char* pligne = reinterpret_cast<char*>(line.data());
		
		if(line[j] != ' ')
			{
				token.push_back(line[j]);//Stocke chaque token
				j++;
				continue;
			}
		tokens.push_back(token);//Dans les tokens de la ligne
		j++;
		if(object[i] != '\n'){token.clear();
							  continue;}//Encore un token
		
		}
		//Fin de ligne
		tokens.push_back(token);
		token.clear();
    	int store = -1;
		vec_size = 0;
		for(int k=0;k<tokens.size();k++)//Procede sur tous les tokens
		{
			if(k==0)//C'est le premier token c'est quoi?
			{
				store = -1;
				if(tokens[k].size() == 1)
				{
					if(tokens[0][0]=='v'){store = 0;vertex_size++;} //vertex
					if(tokens[0][0]=='f'){store = 3;} //face
				}
				if(tokens[k].size() == 2)
				{
					if(tokens[0][0]=='v' && tokens[0][1]=='n'){store = 1;normale_size++;} //normales
					if(tokens[0][0]=='v' && tokens[0][1]=='t'){store = 2;uvs_size++;} //uvs
				}
			}
			else //Pousse les suivants dans leur stockage respectif
			{
			vec_size++;
			if(store == 0){vertex.push_back(std::atof(reinterpret_cast<char*>(tokens[k].data())));
				if(vec_size > vertex_vec_size)vertex_vec_size=vec_size;}
			if(store == 1){normales.push_back(std::atof(reinterpret_cast<char*>(tokens[k].data())));
				if(vec_size > normales_vec_size)normales_vec_size=vec_size;}
			if(store == 2){uvs.push_back(std::atof(reinterpret_cast<char*>(tokens[k].data())));
				if(vec_size > uvs_vec_size)uvs_vec_size=vec_size;}
			if(store == 3){
				faces.push_back(tokens[k]);
				if(vec_size > faces_vec_size)faces_vec_size=vec_size;}
			}
		}
		if(store == 3)//Si c'est les indices de faces faut décoder
		{
			if(faces.size() == 4)std::cout << "QUAD";
			if(faces.size() == 3)std::cout << "TRI" << std::endl;
			std::vector<std::vector<char>> indices;
			std::vector<int> indices_int;
			for(int l=0;l<faces.size();l++)
			{
				std::vector<char> n;
				for(int m=0;m<faces[l].size();m++)
				{
				char v=faces[l][m];
				if(v == '/')
					{
						indices.push_back(n);
						n.clear();
						continue;
					}
				n.push_back(v);
				}
				indices.push_back(n);
				for(int m=0;m<indices.size();m++)
				{
					indices_int.push_back(std::atoi(reinterpret_cast<char*>(indices[m].data())));
					
				}
				for(int m=0;m<indices_int.size();m++)
				{
					std::cout << "/" << indices_int[m];
				}
				std::cout << ",";
				indices_int.clear();
				indices.clear();
			}
			faces.clear();
		}
		line.clear();
		tokens.clear();
		newline = true;
		j = 0;
		
	}
	if(vertex_size == vertex.size()/vertex_vec_size)
		if(uvs_size == uvs.size()/uvs_vec_size)
			if(normale_size == normales.size()/normales_vec_size)
				return;
	
	newline = true;
	//Tout a été lu

}

//is a [] BY FRAMES
#define SN_ARRAY_BY_FRAME
struct SnSwapChain
{
	VkExtent2D window_size;
	
	VkSwapchainKHR swapChain{NULL};     
    
	std::vector<VkImage>		swapChainImages;
    std::vector<VkImageView> 	swapChainImageViews;
    std::vector<VkFramebuffer> 	swapChainFramebuffers;
	
	VkCommandPool commandPool;
	VkCommandBuffer SN_ARRAY_BY_FRAME 		*commandBuffers = nullptr; 		
	VkCommandBuffer SN_ARRAY_BY_FRAME 		*imgui_commandBuffers = nullptr; 				
	
	VkDescriptorSetLayout 					descriptorSetLayout; 
	VkDescriptorSet SN_ARRAY_BY_FRAME 		*descriptorsets = nullptr; 			
	
	Uniform SN_ARRAY_BY_FRAME 				*uniforms = nullptr; 							//[] BY FRAMES

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	//SYNCHRO
	
	std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
	

	//Memory
	
	
	bool pipe = true;
	void CreatePipeline()
	{
		PipeModel pm;
		
		pm.PipeModel_Clear();
		pm.PipeModel_PreFill();
		
		pm.PipeModel_CreateShaders(2);

		auto vertShaderCode = readFile("shaders/vert.spv");
		pm.Shaders[0].pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
		pm.Shaders[0].codeSize = vertShaderCode.size();
		pm.ShaderStageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		pm.ShaderStageInfo[0].pName = "main";
		
		auto fragShaderCode = readFile("shaders/frag.spv");
		pm.Shaders[1].pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
		pm.Shaders[1].codeSize = fragShaderCode.size();
		pm.ShaderStageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		pm.ShaderStageInfo[1].pName = "main";

		pm.PipeModel_CreateBindings(1);
		
		pm.bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pm.bindings[0].binding = 0;
		pm.bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pm.bindings[0].descriptorCount = 1;

		
		
		pm.pipelineLayoutInfo.setLayoutCount = 1; //un par frame
		pm.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; //<---- Ils servent ici un
		
		pm.PipeModel_Proceed(renderPass);

		pipelineLayout = pm.PipeLayout;
		graphicsPipeline = pm.Pipe;
	}
	void DestroyPipeline()
    {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }
	void CreateBuffers()
	{
		if(!uniforms)
		{
			uniforms = new Uniform[swap_imageCount];
			for (size_t i = 0; i < swap_imageCount; i++) 
			{
				glm::mat4 matrice = glm::mat4(1.f);
				uniforms[i].transfert = false;
				uniforms[i].CreateCopyBuffer(glm::value_ptr(matrice),sizeof(glm::mat4));
			}	
		}
	}
	void DestroyBuffers()
	{
		for (size_t i = 0; i < swap_imageCount; i++) 
		{
				vkDestroyBuffer(device,uniforms[i].buffer,nullptr);
		}	
	}
	void CreateDescriptors()
    {
		Read_Obj();
		//Une liste de bindings pour le shader, un modele d'entrées
		
		VkDescriptorSetLayoutBinding setLayoutBindings{};
		setLayoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		// Shader binding point
		setLayoutBindings.binding = 0;
		// Accessible from the vertex shader only (flags can be combined to make it accessible to multiple shader stages)
		setLayoutBindings.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		// Binding contains one element (can be used for array bindings)
		setLayoutBindings.descriptorCount = 1;
		
		// Create the descriptor set layout donc le modele d'entrées en un seul
		VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
		descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutCI.bindingCount = 1; //Le nombre d'entrées de la liste
		descriptorLayoutCI.pBindings = &setLayoutBindings;

		//Le modèle des modèles 
		
		
		err = vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &descriptorSetLayout);
		if( err!= VK_SUCCESS){throw std::runtime_error("failed to create descriptor set layout!");}
		
		descriptorsets = new VkDescriptorSet[swap_imageCount];
		//Les sets de ce modéle un par frame
		
		for (size_t i = 0; i < swap_imageCount; i++) 
		{	
			VkDescriptorSetAllocateInfo allocateInfo{};
			allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocateInfo.descriptorPool = descriptorPool;
			allocateInfo.descriptorSetCount = 1;
			allocateInfo.pSetLayouts = &descriptorSetLayout;
			
			err = vkAllocateDescriptorSets(device, &allocateInfo, &descriptorsets[i]);
			if( err!= VK_SUCCESS){throw std::runtime_error("failed to allocate descriptor sets!");}
			
			VkWriteDescriptorSet writeDescriptorSets;
			VkDescriptorBufferInfo Buffer_info_UBO;
			VkCopyDescriptorSet readDescriptorSets;
		
			Buffer_info_UBO.buffer = uniforms[i].buffer;
			Buffer_info_UBO.offset = 0;
			Buffer_info_UBO.range = uniforms[i].mem_size;

			writeDescriptorSets.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets.dstSet = descriptorsets[i];
			writeDescriptorSets.dstBinding = 0;
			writeDescriptorSets.dstArrayElement = 0;
			writeDescriptorSets.descriptorCount = 1;
			writeDescriptorSets.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets.pBufferInfo = &Buffer_info_UBO;
			
			vkUpdateDescriptorSets(device, 1, &writeDescriptorSets, 0, nullptr);
		}
		
    }

	void DestroyDescriptors()
    {
		vkDeviceWaitIdle(device);
		vkFreeDescriptorSets(device,descriptorPool,swap_imageCount,descriptorsets);
		delete [] descriptorsets;
		
		vkDestroyDescriptorSetLayout(device,descriptorSetLayout,NULL);
    }
	
	
	void SWAPCHAIN_MemoryBarrier(VkCommandBuffer buf,VkImage img,VkImageLayout de,VkImageLayout a)
	{
			VkImageMemoryBarrier memory_barrier = {};
			memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			memory_barrier.srcAccessMask = 0;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = 0;//VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			memory_barrier.oldLayout = de;  //>>>>>>>>>>
			memory_barrier.newLayout = a;//>>>>>>>>>>
			memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			memory_barrier.image = img;//>>>>>>>>>>
			memory_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;//>>>>>>>>>>
			memory_barrier.subresourceRange.levelCount = 1;
			memory_barrier.subresourceRange.layerCount = 1;

			if(de == VK_IMAGE_LAYOUT_UNDEFINED && a == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
			{
			memory_barrier.srcAccessMask = 0;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			vkCmdPipelineBarrier(buf,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0,0,NULL,0,NULL,1,&memory_barrier);
			}
			
			if(de == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && a == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
			{
			memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = 0;
			vkCmdPipelineBarrier(buf,
				VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,0,NULL,0,NULL,1,&memory_barrier);
			}
	}
	void SWAPCHAIN_Create(){
		
		VkSurfaceFormatKHR surfaceFormat = {
			VK_FORMAT_R8G8B8A8_UNORM,//VK_FORMAT_B8G8R8A8_SRGB,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		window_size ={
					static_cast<uint32_t>(sn_interface.destWidth),
					static_cast<uint32_t>(sn_interface.destHeight)
				};

		VkSwapchainCreateInfoKHR createInfoSC{};

			createInfoSC.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfoSC.surface = surface;
			createInfoSC.minImageCount = swap_imageCount;
			createInfoSC.imageFormat = surfaceFormat.format;
			createInfoSC.imageColorSpace = surfaceFormat.colorSpace;
			createInfoSC.imageExtent = window_size;
			createInfoSC.imageArrayLayers = 1;
			createInfoSC.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

		if (graphicsFamily != presentFamily) {
				createInfoSC.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfoSC.queueFamilyIndexCount = 2;
				createInfoSC.pQueueFamilyIndices = queueFamilyIndices;
			} else {
				createInfoSC.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}
			createInfoSC.preTransform = capabilities.currentTransform;
			createInfoSC.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfoSC.presentMode = presentMode;
			createInfoSC.clipped = VK_TRUE;
			createInfoSC.oldSwapchain = VK_NULL_HANDLE;

			err = vkCreateSwapchainKHR(device, &createInfoSC, nullptr, &swapChain);
			if(err != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain!");
			}

		// STEP 7 IMAGES VIEWS
			
			vkGetSwapchainImagesKHR(device, swapChain, &swap_imageCount, nullptr);
			swapChainImages.resize(swap_imageCount);
			vkGetSwapchainImagesKHR(device, swapChain, &swap_imageCount, swapChainImages.data());
			
			VkFormat swapChainImageFormat = surfaceFormat.format;
			
			swapChainImageViews.resize(swapChainImages.size());

			for (size_t i = 0; i < swapChainImages.size(); i++) {
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapChainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapChainImageFormat;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				err = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);  
				if( err!= VK_SUCCESS){
					throw std::runtime_error("failed to create image views!");
				}
			}

		// STEP 8 RENDER PASS
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(carte_graphique, &memProperties);

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = swapChainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; //VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			//Mode clear activé
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = findDepthFormat(carte_graphique);
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			err = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
			if( err!= VK_SUCCESS){
					throw std::runtime_error("failed to create render pass!!");
				}

			// STEP 9 PIPELINE
			
			CreatePipeline();
			
			// STEP 10 FRAME BUFFERS
			
			swapChainFramebuffers.resize(swapChainImageViews.size());

			for (size_t i = 0; i < swapChainImageViews.size(); i++) {
				VkImageView attachments[] = {
					swapChainImageViews[i]
				};

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = window_size.width;
				framebufferInfo.height = window_size.height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create framebuffer!");
				}
			}

			// STEP 11 POOL BUFFER SYNC

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = graphicsFamily;

			if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create command pool!");
			}

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = swap_imageCount;

			commandBuffers = new VkCommandBuffer[swap_imageCount];
			
			
			if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}

			imgui_commandBuffers = new VkCommandBuffer[swap_imageCount];
			
			if (vkAllocateCommandBuffers(device, &allocInfo, imgui_commandBuffers) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
		
	//------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
		
	for (size_t i = 0; i < swap_imageCount; i++) 
		{	
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapChainFramebuffers[i];
		renderPassBeginInfo.renderArea.offset = {0, 0};
		renderPassBeginInfo.renderArea.extent = window_size;

		VkClearValue clearColor = {{0.3f, 0.3f, 0.3f, 1.0f}};
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;
		
		VkClearValue* pClearColor = &clearColor;
		VkClearValue* pDepthValue = nullptr;

		VkRenderingAttachmentInfoKHR Color = {};
		VkRenderingAttachmentInfo Depth = {};

		Color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		Color.imageView = swapChainImageViews[i];
		Color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Color.resolveMode = VK_RESOLVE_MODE_NONE;
		Color.imageView = VK_NULL_HANDLE;
		Color.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Color.loadOp = pClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pClearColor)
		{
			Color.clearValue = *pClearColor;
		}
		
		Depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		Depth.imageView = VK_NULL_HANDLE;
		Depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		Depth.resolveMode = VK_RESOLVE_MODE_NONE;
		Depth.resolveImageView = VK_NULL_HANDLE;
		Depth.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Depth.loadOp = pDepthValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pDepthValue)
		{
			Depth.clearValue = *pDepthValue;
		}

		VkRenderingInfoKHR RenderingInfo = {};
		RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		RenderingInfo.renderArea = {{0,0},{sn_interface.destWidth,sn_interface.destHeight}};
		RenderingInfo.layerCount = 0;
		RenderingInfo.viewMask = 0;
		RenderingInfo.colorAttachmentCount = 1;
		RenderingInfo.pColorAttachments = &Color;
		RenderingInfo.pDepthAttachment = &Depth;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) 
			throw std::runtime_error("failed to begin recording command buffer!");

		//SWAPCHAIN_MemoryBarrier(commandBuffers[i],swapChainImages[i],VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);	
		vkCmdBeginRenderPass(commandBuffers[i],&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
			
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		

			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float) window_size.width;
			viewport.height = (float) window_size.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = {0, 0};
			scissor.extent = window_size;
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(commandBuffers[i], 
									VK_PIPELINE_BIND_POINT_GRAPHICS, 
									pipelineLayout, 0, 1, &descriptorsets[i], 0, nullptr);

			//vkCmdBeginRendering(commandBuffers[i],&RenderingInfo);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			//vkCmdEndRendering(commandBuffers[i]);
	
		//SWAPCHAIN_MemoryBarrier(commandBuffers[i],swapChainImages[i],VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
		}
	}
//------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------	
void SWAPCHAIN_Destroy(){
		
        
		vkFreeCommandBuffers(device,commandPool,swap_imageCount,commandBuffers);
		delete [] commandBuffers;
		vkFreeCommandBuffers(device,commandPool,swap_imageCount,imgui_commandBuffers);
		delete [] imgui_commandBuffers;
		        
		vkDestroyCommandPool(device, commandPool, nullptr);
	
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
            }

        DestroyPipeline();
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (auto imageView : swapChainImageViews) {
                vkDestroyImageView(device, imageView, nullptr);
            }
            
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
	
	void SyncObjectsCreate() {
        imageAvailableSemaphores.resize(swap_imageCount);
        renderFinishedSemaphores.resize(swap_imageCount);
        inFlightFences.resize(swap_imageCount);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < swap_imageCount; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
	
	void SyncObjectsDestroy() {
        for (size_t i = 0; i < swap_imageCount; i++)
		{
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
    }
	
	void CreateImgui()
	{


	}	
	void LightUp()
	{
		CreateBuffers();
		CreateDescriptors();
		SyncObjectsCreate();
	}
}ECRAN;
void EcranOn()
{
	ECRAN.LightUp();
}
void EcranOff()
{
	ECRAN.SWAPCHAIN_Destroy();
	ECRAN.DestroyDescriptors();
	ECRAN.DestroyBuffers();
	
}
void rebuild(bool rebuild)
	{  	
		vkDeviceWaitIdle(device);
		if(rebuild)ECRAN.SWAPCHAIN_Destroy();
		ECRAN.SWAPCHAIN_Create();
	}


static float f = 0.0f;

void Update_uniforms(uint32_t imageIndex){     

		static float horloge = 0.f;
		horloge += sn_interface.delta_time * f;// * glm::two_pi<float>();
		
		glm::mat4 Model = glm::mat4(1.0f);//glm::rotate(glm::mat4(1.0f), horloge, glm::vec3(0, 0, 1.f));
		glm::mat4 project = glm::scale(glm::mat4(1.0f), glm::vec3(sn_interface.aspectH, sn_interface.aspectV, 1.f));
		glm::mat4 trans = glm::translate(glm::mat4(1.0f),glm::vec3(sn_interface.wfX, sn_interface.wfY, 0.f));
		
		Model = trans * project * glm::rotate(glm::mat4(1.0f), horloge, glm::vec3(0, 0, 1.f));
		memcpy(ECRAN.uniforms[imageIndex].mem_ptr,glm::value_ptr(Model),sizeof(glm::mat4));
	}

bool show_demo_window = true;
bool show_another_window = false;

void sn_Vulkandraw(){        

	static uint32_t imageIndex = 0;
	err = vkAcquireNextImageKHR(device, 
								ECRAN.swapChain, 
								UINT64_MAX, 
								ECRAN.imageAvailableSemaphores[imageIndex], 
								VK_NULL_HANDLE, 
								&imageIndex);
		if(err == VK_ERROR_OUT_OF_DATE_KHR)
			{
				std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
				sn_interface.resizing = false;
				rebuild(true);
				//return;
			}else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }
	if(err == VK_ERROR_DEVICE_LOST)std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
	if(err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)std::cout << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" << std::endl;
	if(err == VK_ERROR_OUT_OF_DATE_KHR)std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
	if(err == VK_ERROR_OUT_OF_DEVICE_MEMORY)std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	if(err == VK_ERROR_OUT_OF_HOST_MEMORY)std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	if(err == VK_ERROR_SURFACE_LOST_KHR)std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
	if(err == VK_ERROR_UNKNOWN)std::cout << "VK_ERROR_UNKNOWN" << std::endl;
	
	vkWaitForFences(device, 1, &ECRAN.inFlightFences[imageIndex], VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &ECRAN.inFlightFences[imageIndex]);
	
	//if(err == VK_ERROR_VALIDATION_FAILED)std::cout << "VK_ERROR_VALIDATION_FAILED" << std::endl;
	// OFFLINE (1340)
		//vkBeginCommandBuffer
		//vkCmdPipelineBarrier
		
		//vkCmdBindPipeline
		//vkCmdBindDescriptorSets
		//vkCmdDraw
		
		//vkCmdPipelineBarrier
		//vkEndCommandBuffer
			
	// ONLINE (1429)
		//vkAcquireNextImageKHR
		//Update uniforms
		//vkQueueSubmit
		//vkQueuePresentKHR

	
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
		ImGuiIO& io = ImGui::GetIO();
		
		// Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		
		
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
           static int counter = 0;

			io.MouseDrawCursor = true;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

			static float clr_color[3] = {0.1f,0.6f,0.8f};
            ImGui::SliderFloat("float", &f, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clr_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
			ImGui::SameLine();
            ImGui::Text("ECRAN.currentFrame = %d", ECRAN.currentFrame);
			ImGui::SameLine();
            ImGui::Text("imageIndex = %d", imageIndex);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
		
        ImDrawData* draw_data = ImGui::GetDrawData();
		
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
      
     	vkResetCommandBuffer(ECRAN.imgui_commandBuffers[imageIndex],VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		//err = vkResetCommandPool(device, ECRAN.commandPool, 0);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(ECRAN.imgui_commandBuffers[imageIndex], &info);
      
    	VkClearValue clear_color = {};

        VkRenderPassBeginInfo RPinfo = {};
        
		RPinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        RPinfo.renderPass = imgui_renderPass;
        RPinfo.framebuffer = ECRAN.swapChainFramebuffers[imageIndex];
        RPinfo.renderArea.extent.width = sn_interface.destWidth;
        RPinfo.renderArea.extent.height = sn_interface.destHeight;
        RPinfo.clearValueCount = 1;
        RPinfo.pClearValues = &clear_color;
		
        //vkCmdBeginRenderPass(ECRAN.imgui_commandBuffers[imageIndex], &RPinfo, VK_SUBPASS_CONTENTS_INLINE);
		
		VkClearValue* pClearColor = nullptr;
		VkClearValue* pDepthValue = nullptr;

		VkRenderingAttachmentInfoKHR Color = {};
		VkRenderingAttachmentInfo Depth = {};
		Color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		Depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		Color.imageView = ECRAN.swapChainImageViews[imageIndex];
		Color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Color.resolveMode = VK_RESOLVE_MODE_NONE;
		Color.imageView = VK_NULL_HANDLE;
		Color.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Color.loadOp = pClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pClearColor)
		{
			Color.clearValue = *pClearColor;
		}
		
		Depth.imageView = VK_NULL_HANDLE;
		Depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		Depth.resolveMode = VK_RESOLVE_MODE_NONE;
		Depth.resolveImageView = VK_NULL_HANDLE;
		Depth.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Depth.loadOp = pDepthValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pDepthValue)
		{
			Depth.clearValue = *pDepthValue;
		}
		
		
		
		VkRenderingInfoKHR RenderingInfo = {};
		RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		RenderingInfo.renderArea = {{0,0},{sn_interface.destWidth,sn_interface.destHeight}};
		RenderingInfo.layerCount = 0;
		RenderingInfo.viewMask = 0;
		RenderingInfo.colorAttachmentCount = 1;
		RenderingInfo.pColorAttachments = &Color;
		RenderingInfo.pDepthAttachment = &Depth;
		
		//vkCmdBeginRendering(ECRAN.imgui_commandBuffers[imageIndex],&RenderingInfo);
		ImGui_ImplVulkan_RenderDrawData(draw_data, ECRAN.imgui_commandBuffers[imageIndex],VK_NULL_HANDLE);
		//vkCmdEndRendering(ECRAN.imgui_commandBuffers[imageIndex]);
		
  		ECRAN.SWAPCHAIN_MemoryBarrier(ECRAN.imgui_commandBuffers[imageIndex],
									ECRAN.swapChainImages[imageIndex],
									VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
									VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		
		//Submit command buffer
		//vkCmdEndRenderPass(ECRAN.imgui_commandBuffers[imageIndex]);
		
		err = vkEndCommandBuffer(ECRAN.imgui_commandBuffers[imageIndex]);

		if(err != VK_SUCCESS)
		{
			
			if(err == VK_ERROR_OUT_OF_HOST_MEMORY)throw std::runtime_error("Marche pas le vkEndCommandBuffer de Imgui!");
		}
	VkCommandBuffer batch[2] = {ECRAN.commandBuffers[imageIndex],ECRAN.imgui_commandBuffers[imageIndex]};
	#else
	VkCommandBuffer batch[1] = {ECRAN.commandBuffers[imageIndex]};
    #endif


	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &ECRAN.imageAvailableSemaphores[imageIndex];
	submitInfo.pWaitDstStageMask = waitStages;
	
	#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	submitInfo.commandBufferCount = 2;
	#else
	submitInfo.commandBufferCount = 1;
	#endif

	submitInfo.pCommandBuffers = batch;//&ECRAN.commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &ECRAN.renderFinishedSemaphores[imageIndex];

	Update_uniforms(imageIndex);

	err = vkQueueSubmit(graphicsQueue, 1, &submitInfo, ECRAN.inFlightFences[imageIndex]);

	if (err != VK_SUCCESS) {
			if(err == VK_ERROR_OUT_OF_HOST_MEMORY)throw std::runtime_error("VK_ERROR_OUT_OF_HOST_MEMORY");
			if(err == VK_ERROR_OUT_OF_DEVICE_MEMORY)throw std::runtime_error("VK_ERROR_OUT_OF_DEVICE_MEMORY");
			if(err == VK_ERROR_DEVICE_LOST)throw std::runtime_error("VK_ERROR_DEVICE_LOST");
			throw std::runtime_error("failed to submit draw command buffer!");
        }

		//Tu vas attendre sur ce semaphore pour présenter l'image
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &ECRAN.imageAvailableSemaphores[imageIndex];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &ECRAN.swapChain;
		presentInfo.pImageIndices = &imageIndex;

		err = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
          
            sn_interface.resizing = false;
			rebuild(true);
        } else if (err != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
		vkQueueWaitIdle(presentQueue);
        ECRAN.currentFrame = (ECRAN.currentFrame + 1) % swap_imageCount;
			
}

