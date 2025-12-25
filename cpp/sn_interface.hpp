#define MOUSE_INPUT(x, maskpressed, maskreleased) \
	if(raw->data.mouse.usButtonFlags & maskpressed)sn_interface.MouseBt[x].input_pressed(timestamp); \
	if(raw->data.mouse.usButtonFlags & maskreleased)sn_interface.MouseBt[x].input_released(timestamp); 	

void sn_Interface::Wmousemove(void)
{
	wfX = float(wX) / float(destWidth) * 2.f;
	wfY = float(wY) / float(destHeight) * 2.f;
	
}
void sn_Interface::ChangeSize(unsigned short Width,unsigned short Height)
{
	
	destWidth = Width;
	destHeight = Height;
	aspectratio = (float)destWidth/(float)destHeight;
	aspectH = base / destWidth; 		
	aspectV = base / Height; 

	resizing = false;
}
long long sn_Interface::Tick()
	{
	static LARGE_INTEGER Frequency;
	static LARGE_INTEGER time_start;
	static LARGE_INTEGER time_now;
	static int pos = 0;
	static double fpss[128];

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

	//Converti les clock processeur en millisecond et stocke les 128 derniers
	fpss[pos] = (double)Tick;
	fpss[pos] *= 0.000001;
	pos++;
	if(pos>127)pos=0;
	
	//Fais une integrale et calcule un delta moyen et les fps
	fps = 0.0f;
	for(volatile int i = 0; i < 128; i++)fps += fpss[i];
	fps /= 128.0;
	delta_time = fps;
	fps = 1.0 / delta_time; 

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
void Mouse(RAWINPUT* raw, long timestamp)
	{
		if(raw->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
		{
			sn_interface.mdY = raw->data.mouse.lLastY - sn_interface.mY;
			sn_interface.mdX = raw->data.mouse.lLastX - sn_interface.mX;
		}
		else
		{
			sn_interface.mdY = raw->data.mouse.lLastY;
			sn_interface.mdX = raw->data.mouse.lLastX;
		}
		sn_interface.mX = raw->data.mouse.lLastX;
		sn_interface.mY = raw->data.mouse.lLastY;
		sn_interface.radX += (float)sn_interface.mdX * 0.00152f;
		sn_interface.radY += (float)sn_interface.mdY * 0.00152f;
		if(sn_interface.radX > 1.f)sn_interface.radX = 1.f;
		if(sn_interface.radX < -1.f)sn_interface.radX = -1.f;
		if(sn_interface.radY > 1.f)sn_interface.radY = 1.f;
		if(sn_interface.radY < -1.f)sn_interface.radY = -1.f;

		MOUSE_INPUT(0, RI_MOUSE_LEFT_BUTTON_DOWN, RI_MOUSE_LEFT_BUTTON_UP)
		MOUSE_INPUT(1, RI_MOUSE_RIGHT_BUTTON_DOWN, RI_MOUSE_RIGHT_BUTTON_UP)
		MOUSE_INPUT(2, RI_MOUSE_MIDDLE_BUTTON_DOWN, RI_MOUSE_MIDDLE_BUTTON_UP)
		MOUSE_INPUT(3, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP)
		MOUSE_INPUT(4, RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP)

		if(raw->data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
		{
			sn_interface.wheelDelta += (short)raw->data.mouse.usButtonData;
			sn_interface.baseexp += ((float)(short)raw->data.mouse.usButtonData)*.001f;
			sn_interface.aspectH = sn_interface.base / sn_interface.destWidth; 		
			sn_interface.aspectV = sn_interface.base / sn_interface.destHeight; 
			sn_interface.base = 400.f * exp(sn_interface.baseexp);
		}
	}
bool Keyboard(RAWINPUT* raw, long timestamp)
	{
		if(raw->data.keyboard.Flags == 0)
			sn_interface.Keys[raw->data.keyboard.MakeCode].input_pressed(timestamp);
		else
			return sn_interface.Keys[raw->data.keyboard.MakeCode].input_released(timestamp);
		return false;
	}