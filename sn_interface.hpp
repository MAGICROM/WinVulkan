#define MOUSE_INPUT(x, maskpressed, maskreleased) \
	if(raw->data.mouse.usButtonFlags & maskpressed)MouseBt[x].input_pressed(timestamp); \
	if(raw->data.mouse.usButtonFlags & maskreleased)MouseBt[x].input_released(timestamp); 	

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
bool sn_Interface::Button_pressed(RAWINPUT* raw,int* pnum){
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