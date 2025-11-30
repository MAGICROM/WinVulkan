//#define ONDECORE_CODE in one and only *.c or *.cpp for using the library

#ifndef ONDECORE_H
#define ONDECORE_H 2025 //18 Novembre

#pragma once 

#define _PARAMEMETER_

#define _IN_					_PARAMEMETER_
#define _OUT_					_PARAMEMETER_
#define _IN_OUT_				_PARAMEMETER_

#define REGISTER_NUMBER			32

typedef struct strRegisterSet
{
	float FLOAT[REGISTER_NUMBER];					//Pour les valeurs moins precise
	double DOUBLE[REGISTER_NUMBER];					//Pour les valeurs en interne
	int INT[REGISTER_NUMBER];						//Pour les boucles
	int LONG[REGISTER_NUMBER];						//Pour les offset dans les tableau
	unsigned char* UCHAR_POINTER[REGISTER_NUMBER];  //Pour les indian
	double* DOUBLE_POINTER[2];						//Pour les grain
}REGISTER_SET;

static REGISTER_SET ASIO_RegisterSet;
static int ASIO_DefaultCanal = 0;

enum dsp_opcode
{
	opcode_return = 0,
	opcode_osc,
	opcode_sinus,
	opcode_triangle,
	opcode_square,
	opcode_noise_white,
	opcode_noise_pink,
	opcode_noise_brown,
	opcode_lorenz,
	opcode_adsr,
	opcode_add,
	opcode_mul,
	opcode_sub,
	opcode_div,
	opcode_value,
}; 

typedef enum ocmodel
{
	ocmclass_null, //Singleton or something not to destroy
	ocmclass_visible,
	ocmclass_sample,
	ocmclass_openAL
}ocmodel;

template<class T>
struct Base_Node
{
	T* Ptr;
	Base_Node() :Ptr(nullptr) {}
};

template<class T>
class MiniBase
{
	unsigned long number;
	unsigned long page_size;
	unsigned long base_size;
	Base_Node<T>* pBase;
public:
	MiniBase() :pBase(nullptr),
		page_size(128),
		number(0), 
		base_size(0){}
	Base_Node<T>* FirstFree(unsigned long* i);
	void Remove(T * p);
	unsigned long Add(T * p);
};

class OndeComponent;

struct OndeMsg
{
	int port;
	int msg;
	void* ptr;
};

class OndeCore
{
	MiniBase<OndeComponent> BaseComponent;
	unsigned long count_gfx;
	//messages 
	OndeMsg message[16];
	unsigned long count_msg;
	OndeComponent* first_gfx;
public:
	unsigned long StoreGfx(OndeComponent* p);
	void UnStoreGfx(OndeComponent* p);
	OndeCore();
	void PostMsg(int port, int msg, void* ptr);
	bool GetMsg(unsigned long *pcount_msg, OndeMsg *pmessage); //while(GetMsg(mypos,&OndeMsg)){}
	OndeComponent* List() { return first_gfx; }
	static OndeCore& Instance();
};

class OndeComponent
{
	unsigned long base_id;
	unsigned long msg_id;
public:
	unsigned long flags;
	ocmodel class_type;
	OndeComponent* pNext;
	bool GetMsg(OndeMsg *pmessage)
	{
		return OndeCore::Instance().GetMsg(&msg_id,pmessage);
	}
	OndeComponent(const ocmodel type = ocmclass_null):class_type(type)
	{
		flags = 0;
		
		base_id = OndeCore::Instance().StoreGfx(this);
		msg_id = 0;
	}
	~OndeComponent()
	{
		
		OndeCore::Instance().UnStoreGfx(this);
	}
	static OndeComponent* First() { return OndeCore::Instance().List();}
};

class Visible : public OndeComponent
{
public:
	bool visible; 
	bool nope;

	float x;
	float y;
	float sx;
	float sy;
	
	float v;
	float nv;
	float vv;

	Visible(const ocmodel type = ocmclass_visible) :OndeComponent(type)
	{
		visible = true;
		vv = nv = v = x = y = sx = sy = 0.0f;
		nope = true;
	}
	~Visible()
	{
		
	}

	bool In(float tx, float ty)
	{
		if (tx >= x)
			if (tx <= x + sx)
				if (ty >= y)
					if (ty <= y + sy)
					{
						nope = false;
						return true;
					}
		//nope iam tested but not under the mouse
		nope = true;
		return false;
	}
	void GrabDeltaY(float delta_y)
	{
		nv = v - (delta_y / 5.f);
		int n = (int)nv;
		if (n < 0)
		{
			nv = 0.0f;
			n = 0;
		}
		else if (n > 90)
		{
			nv = 90.0f;
			n = 90;
		}
		vv = (float)n;
	}
		
	static void DeselectAll()
	{
		Visible* x = (Visible*)First();
		while (x)
		{
			x->flags = x->flags & ~(1);
			x = (Visible*)x->pNext;
		}
	}
	static void MoveAllSelected(float dx, float dy)
	{
		Visible* x = (Visible*)First();
		while (x)
		{
			if (x->flags & 1)
			{
				x->x += dx;
				x->y += dy;
			}
			x = (Visible*)x->pNext;
		}
	}
};

#ifdef ONDECORE_CODE
OndeCore& OndeCore::Instance() {
	static OndeCore TheCloud;
	return TheCloud;
}

OndeCore::OndeCore() :first_gfx(nullptr), count_gfx(0), count_msg(0)
{
}; 

template<class T>
Base_Node<T>* MiniBase<T>::FirstFree(unsigned long* i)
{
	//No base Create one 
	if (!pBase)
	{
		(*i) = 0;
		base_size = page_size;
		pBase = new Base_Node<T>[base_size];
		return &pBase[0];
	}

	//Search a free bloc  
	for ((*i) = 0; (*i) < base_size; (*i)++)
	{
		if (!pBase[(*i)].Ptr)return &pBase[(*i)];
	}

	//No free bloc create a bigger base
	Base_Node<T>* pNewBiggerBase = new Base_Node<T>[base_size + page_size];
	//Copy Base
	for ((*i) = 0; (*i) < base_size; (*i)++)
	{
		pNewBiggerBase[(*i)].Ptr = pBase[(*i)].Ptr;
	}
	delete[] pBase;
	base_size += page_size;
	pBase = pNewBiggerBase;
	return &pNewBiggerBase[(*i)];
}

template<class T>
void MiniBase<T>::Remove(T * p)
{
	for (unsigned long i = 0; i < base_size; i++)
	{
		if (pBase[i].Ptr == (T*)p)
		{
			pBase[i].Ptr = nullptr;
			number--;
			return;
		}
	}
}

template<class T>
unsigned long MiniBase<T>::Add(T * p)
{
	unsigned long i;
	Base_Node<T>* px = FirstFree(&i);
	px->Ptr = p;
	number++;
	return i;
}

void OndeCore::PostMsg(int port, int msg, void* ptr)
{
	message[count_msg].port = port;
	message[count_msg].msg = msg;
	message[count_msg++].ptr = ptr;
	if(count_msg > 15)count_msg = 0;
}

bool OndeCore::GetMsg(unsigned long *pcount_msg, OndeMsg *pmessage)
{
	if(*pcount_msg == count_msg)return false;
	*pmessage = message[(*pcount_msg)++];
	if((*pcount_msg)> 15)(*pcount_msg) = 0;
	return true;
}

unsigned long OndeCore::StoreGfx(OndeComponent* p)
{
	p->pNext = first_gfx;
	first_gfx = p;
	count_gfx++;
	return BaseComponent.Add(p);
}

void OndeCore::UnStoreGfx(OndeComponent* p)
{
	BaseComponent.Remove(p);
	
	if (p == first_gfx)
	{
		first_gfx = first_gfx->pNext;
		count_gfx--;
		return;
	}

	OndeComponent* ps = first_gfx;

	while (ps)
	{
		if (p == ps->pNext)
		{
			ps->pNext = ps->pNext->pNext;
			count_gfx--;
			return;
		}
		ps = ps->pNext;
	}
}
#endif //ONDECORE_CODE
#endif //ONDECORE_H