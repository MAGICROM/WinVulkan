#define DELAY_FAST 0.15f

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
	double delta_time;	//temps lissé par intégration
	double fps;			
	
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
}sn_interface;

