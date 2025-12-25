/* Make Four char code in Intel byte order */
#ifndef LWC_H
#define LWC_H

#include <windows.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#define LWC_MAKE_ID(a,b,c,d)	\
	((unsigned long) (d)<<24 | (unsigned long) (c)<<16 | \
	 (unsigned long) (b)<<8 | (unsigned long) (a))

/* Universal IFF identifiers */

#define LWC_ID_FORM		LWC_MAKE_ID('F','O','R','M')
#define LWC_ID_LWCF		LWC_MAKE_ID('L','W','C','F')
#define LWC_ID_LWC2		LWC_MAKE_ID('L','W','C','2')//V 2.0 avec des calques

/* Vertices CHUNK identifiers */

#define LWC_ID_TRIA		LWC_MAKE_ID('T','R','I','A')
#define LWC_ID_VERT		LWC_MAKE_ID('V','E','R','T')
#define LWC_ID_INDX		LWC_MAKE_ID('I','N','D','X')
#define LWC_ID_BONE		LWC_MAKE_ID('B','O','N','E')
#define LWC_ID_WGHT		LWC_MAKE_ID('W','G','H','T')
#define LWC_ID_DATA		LWC_MAKE_ID('D','A','T','A')

/* Type de modele */

#define MODELTYPE_TRIANGLELIST	1
#define MODELTYPE_INDEXLIST		2  

/* Structures de stockage */

//Point

struct LWCHUMANFRAME
{
    glm::vec3 vRot[20];
    float fTime;
	LWCHUMANFRAME(){for(int i=0;i<20;i++)vRot[i]=glm::vec3(0.0f,0.0f,0.0f);}
};

struct LWCVERTEX
{
    glm::vec3 p;
    glm::vec3 n;
    float       tu, tv;
};

#define D3DFVF_LWCVERTEX  (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

//Bone
struct LWCBONE
{
    int			id;
	int			parent;

	glm::vec3 center;
    glm::vec3 direction;
	glm::vec3 up;
};

typedef struct MATERIAL {
  glm::vec4 Diffuse;
  glm::vec4 Ambient;
  glm::vec4 Specular;
  glm::vec4 Emissive;
  float         Power;
} MATERIAL, *LPDMATERIAL;

//Material
struct LWCMATERIAL
{
    unsigned long			dwStart;
	unsigned long			dwNumber;
	MATERIAL	    material;
	bool			specular;
	bool			transparent;
	bool			mirror;
};

/* Methodes de traitement*/

int		Lwc_ReadBONE		(HANDLE hFile,	LWCBONE* bone);
int		Lwc_WriteBONE		(HANDLE hFile,	LWCBONE* bone);
int		Lwc_ReadV12			(HANDLE hFile,	float* a,float* b,float* c);
int		Lwc_WriteV12		(HANDLE hFile,	float* a,float* b,float* c);
int		Lwc_WriteSIZECHUNK	(HANDLE hFile,	unsigned long* sizepos,	unsigned long* size);
int		Lwc_WriteCHUNK		(HANDLE hFile,	unsigned long* chunk,	unsigned long* sizepos);

int		Lwc_ReadREADVERTEX	(HANDLE hFile,LWCVERTEX* modelvertex);
int		Lwc_ReadTRIANGLE	(HANDLE hFile,unsigned long* pa,unsigned long* pb,unsigned long* pc);
int		Lwc_ReadCHUNK		(HANDLE hFile,unsigned long* chunk,unsigned long* size);

//-----------------------------------------------------------------------------
// Lire et ecrire un fichier special Lightwave converted
//-----------------------------------------------------------------------------

#pragma warning(disable : 4068)

#if !defined(__WIN32__) && !defined(_WIN32)
#define EXPORT16 __export 
#else
#define EXPORT16
#endif

//#define FLIP_X
//#define FLIP_Y
//#define FLIP_Z
//#define CHANGE_HAND

//**************ECRITURE*******************************************

//le container pour stocker le fichier .lwo lu

int		Lwc_WriteMATERIAL		(HANDLE hFile,	LWCMATERIAL* material)
{
	DWORD	num_octets_ok	=0;
	int		count			=0;
	
	
	WriteFile(hFile,material,sizeof(LWCMATERIAL),&num_octets_ok,NULL);
	count+= num_octets_ok;
	
	return count;

}

int		Lwc_WriteBONE		(HANDLE hFile,	LWCBONE* bone)
{
	DWORD	num_octets_ok	=0;
	int		count			=0;
	
	
	WriteFile(hFile,bone,sizeof(LWCBONE),&num_octets_ok,NULL);
	count+= num_octets_ok;
	
	return count;

}

int Lwc_WriteV12(HANDLE hFile,float* a,float* b,float* c)
{
	DWORD	num_octets_ok	=0;
	int		count			=0;
	
	
	WriteFile(hFile,a,sizeof(float),&num_octets_ok,NULL);
	count+= num_octets_ok;
	
	WriteFile(hFile,b,sizeof(float),&num_octets_ok,NULL);
	count+= num_octets_ok;

	WriteFile(hFile,c,sizeof(float),&num_octets_ok,NULL);
	count+= num_octets_ok;

	return count;
}

int Lwc_WriteCHUNK(HANDLE hFile,DWORD* chunk,DWORD* sizepos)
{
	DWORD	num_octets_ok	=0;
	DWORD	readed			=0;
	int		count			=0;
	
	if(chunk != NULL)
	{
	WriteFile(hFile,chunk,sizeof(DWORD),&num_octets_ok,NULL);
	count+= num_octets_ok;
	}
	if(sizepos != NULL)
	{
	*sizepos=SetFilePointer(hFile,0,NULL,FILE_CURRENT);
	WriteFile(hFile,chunk,sizeof(DWORD),&num_octets_ok,NULL);
	count+= num_octets_ok;
	}
	return count;
}


int Lwc_WriteSIZECHUNK(HANDLE hFile,DWORD* sizepos,DWORD* size)
{
	DWORD	dwMemoPos = SetFilePointer(hFile,0,NULL,FILE_CURRENT);
	DWORD	num_octets_ok	=0;
	DWORD	readed			=0;
	int		count			=0;
	
	if(sizepos != NULL && size!=NULL)
	{
	SetFilePointer(hFile,(*sizepos),NULL,FILE_BEGIN);
	WriteFile(hFile,size,sizeof(DWORD),&num_octets_ok,NULL);
	SetFilePointer(hFile,dwMemoPos,NULL,FILE_BEGIN);
	}
	return 0;
}
	
//**************LECTURE*******************************************

int		Lwc_ReadBONE		(HANDLE hFile,	LWCBONE* bone)
{
		DWORD	num_octets_ok;
		int		c=0;
		
		ReadFile(hFile,(LPVOID*)bone,sizeof(LWCBONE),&num_octets_ok,NULL);
		c+=num_octets_ok;
		
		return c;
}

int Lwc_ReadV12(HANDLE hFile,float* a,float* b,float* d)
{
		DWORD	num_octets_ok;
		int		c=0;
		
		ReadFile(hFile,(LPVOID*)a,sizeof(float),&num_octets_ok,NULL);
		c+=num_octets_ok;
		
		ReadFile(hFile,(LPVOID*)b,sizeof(float),&num_octets_ok,NULL);
		c+=num_octets_ok;
		
		ReadFile(hFile,(LPVOID*)d,sizeof(float),&num_octets_ok,NULL);
		c+=num_octets_ok;
		
		return c;
}

int Lwc_ReadREADVERTEX(HANDLE hFile,LWCVERTEX* modelvertex)
{
		DWORD	num_octets_ok;
		int		c=0;
		
		ReadFile(hFile,(LPVOID*)modelvertex,sizeof(LWCVERTEX),&num_octets_ok,NULL);
		c+=num_octets_ok;
		
		return c;
}

int Lwc_ReadTRIANGLE(HANDLE hFile,DWORD* pa,DWORD* pb,DWORD* pc)
{
		DWORD	num_octets_ok;
		int		c=0;
		
		ReadFile(hFile,(LPVOID*)pa,sizeof(DWORD),&num_octets_ok,NULL);
		c+=num_octets_ok;
		ReadFile(hFile,(LPVOID*)pb,sizeof(DWORD),&num_octets_ok,NULL);
		c+=num_octets_ok;
		ReadFile(hFile,(LPVOID*)pc,sizeof(DWORD),&num_octets_ok,NULL);
		c+=num_octets_ok;
		return c;
}

int Lwc_ReadCHUNK(HANDLE hFile,DWORD* chunk,DWORD* size)
{
	DWORD	num_octets_ok	=0;
	DWORD	readed			=0;
	int   count			=0;
	
	if(chunk != NULL)
	{
	ReadFile(hFile,(LPVOID*)&readed,4,&num_octets_ok,NULL);
	*chunk=readed;
	count=num_octets_ok;
		if(size	!= NULL)
		{
		ReadFile(hFile,(LPVOID*)&readed,4,&num_octets_ok,NULL);
		*size=readed;
		count+=num_octets_ok;
		}
	}
	return count;
}

int Lwc_ReadDWORD(HANDLE hFile,DWORD* dword)
{
	DWORD	num_octets_ok	=0;
	DWORD	readed			=0;
	int		count			=0;
	
	if(dword != NULL)
	{
	ReadFile(hFile,(LPVOID*)&readed,4,&num_octets_ok,NULL);
	*dword=readed;
	count=num_octets_ok;
	}
	return count;
}

int Lwc_ReadMATERIAL(HANDLE hFile,LWCMATERIAL* material)
{
	DWORD			num_octets_ok	=0;
	LWCMATERIAL		readed			;
	int				count			=0;
	
	if(material != NULL)
	{
	ReadFile(hFile,(LPVOID*)&readed,sizeof(LWCMATERIAL),&num_octets_ok,NULL);
	*material=readed;
	count=num_octets_ok;
	}
	return count;
}

#endif LWC_H

#ifndef LWCMODELE_CLASS_H
#define LWCMODELE_CLASS_H

#define IS_CORRUPTED	(0)
#define IS_OK			(1)

typedef struct SKELETON_WEIGHT
{
	//Container des monstres et objets
	DWORD			dwIndex;
	float			fValue;
}SKELETON_WEIGHT;

typedef struct LAYER_DATA
{
	//Container des monstres et objets
	DWORD			dwIndex;
	DWORD			dwNumTriangle;
}LAYER_DATA;
typedef struct LAYER_DATA *LPLAYER_DATA;

typedef struct SKELETON_BONE
{
	//Pointeur sur l' os parent
	
	SKELETON_BONE*		pParent;
	
	//La weight map de cet os
	
	SKELETON_WEIGHT*	pWeightMap;
	DWORD				dwNbrPoints;
	
	//Donn�es lu depuis le fichier
	
	int			parent;
	
	glm::vec3 center;
    glm::vec3 direction;
	glm::vec3 up;

    
	 
	//Donn�es pour calcul
	
	glm::mat4		mMatrice;
	glm::mat4		mSelfMatrice;
	glm::mat4		mRotation;
	glm::mat4		mSelfRotation;
	//Constructeur
	
	SKELETON_BONE():pParent(NULL),pWeightMap(NULL)	{}
	~SKELETON_BONE(){if(pWeightMap)delete[]pWeightMap;}

void RotationYawPitchRoll(float yaw,float pitch,float roll);
}SKELETON_BONE;
typedef struct SKELETON_BONE *LPSKELETON_BONE;

typedef struct SKELETON_SQUELETTE
{
	CHAR						cEtat;
	
	//Tableau des os
	
	DWORD						dwNbrOs;
	SKELETON_BONE *				pOs;
	
	//Tableau de pointeurs sur les os pour calcul et index
	//pour copie
	
	LPSKELETON_BONE *			pRenderOs;
	WORD			*			pwIndex;

	SKELETON_SQUELETTE():pOs(NULL),pRenderOs(NULL)	{}
	~SKELETON_SQUELETTE()							{}
	
	void Release(){	if(pOs)delete[]pOs;
					if(pRenderOs)delete[]pRenderOs;}

}SKELETON_SQUELETTE;
typedef struct SKELETON_SQUELETTE *LPSKELETON_SQUELETTE;

typedef struct LPVULKANDEVICE9
{
	void CreateVertexBuffer( uint32_t sizetotal, int mem_type, int flags, int mem_model, void** ptrptr ){}
	void CreateIndexBuffer( uint32_t sizetotal, int mem_type, int flags, int mem_model, void** ptrptr ){}
}VULKANDEVICE9, *LPVULKANDEVICE9;

typedef struct VULKANVERTEXBUFFER9
{
	void Lock( uint32_t sizetotal, uint32_t sizeelem, void* *ptrptr, uint32_t startindex){}
	void Unlock(){}
}VULKANVERTEXBUFFER9, *LPVULKANVERTEXBUFFER9;

typedef struct LPVULKANINDEXBUFFER9
{
	void Lock( uint32_t sizetotal, uint32_t sizeelem, void* *ptrptr, uint32_t startindex){}
	void Unlock(){}
}LPVULKANINDEXBUFFER9, *LPLPVULKANINDEXBUFFER9;


typedef struct LWC_SCULPT
{
INT		iModeltype;
//Render this destination
LPVULKANVERTEXBUFFER9	l_pVERTEXBUFFER;
//Cree et present seulement dans le modele
LPVULKANVERTEXBUFFER9	l_pSOURCEVERTEXBUFFER;
LPVULKANINDEXBUFFER9	l_pSOURCEINDEXBUFFER;
LWCMATERIAL*			l_pLAYER;
//With this deformer
LPSKELETON_SQUELETTE	l_pSQUELETTE;

INT		iNumLayer;
DWORD	dwNumTriangles;
DWORD	dwNumVertex;

void	MoveUV(float u,float v,int layer);
void	RenderSquelette(void);
void	Render(LPVULKANDEVICE9 l_pd3dDevice);
void	Release(void);

	LWC_SCULPT():	l_pVERTEXBUFFER(NULL),
					l_pSOURCEVERTEXBUFFER(NULL),
					l_pSOURCEINDEXBUFFER(NULL),
					l_pSQUELETTE(NULL),
					l_pLAYER(NULL){}
	~LWC_SCULPT()							{Release();}
}LWC_SCULPT;
typedef struct LWC_SCULPT *LPLWC_SCULPT;

typedef struct LWC_MODELE
{
float	fLowerPoint;
INT		iModeltype;
INT		iNumLayer;

LWCMATERIAL*			l_pLAYER;

LPVULKANVERTEXBUFFER9	l_pVERTEXBUFFER;
LPVULKANINDEXBUFFER9	l_pINDEXBUFFER;

LPSKELETON_SQUELETTE	l_pSQUELETTE;

DWORD	dwNumTriangles;
DWORD	dwNumVertex;

void	PrepareSquelette(void);
void	ChangeMirrorForPortrait(LPVULKANDEVICE9 l_pd3dDevice,int Portrait);
void	FillSculpt(LPVULKANDEVICE9 l_pd3dDevice,LPLWC_SCULPT pSculpt);
void	CreateSculpt(LPVULKANDEVICE9 l_pd3dDevice,LPLWC_SCULPT* lp_pSculpt);
void	ReadFromFile(LPVULKANDEVICE9 l_pd3dDevice,TCHAR* szLocation);
void	Render(LPVULKANDEVICE9 l_pd3dDevice,LPVOID pData=NULL,BOOL bUseMaterial=TRUE);
void	Release(void);

	LWC_MODELE():	l_pVERTEXBUFFER(NULL),
					l_pINDEXBUFFER(NULL),
					l_pSQUELETTE(NULL),
					l_pLAYER(NULL){}
	~LWC_MODELE()	{
					if(l_pLAYER)delete[]l_pLAYER;
					}

}LWC_MODELE;
typedef struct LWC_MODELE *LPLWC_MODELE;

#endif LWCMODELE_CLASS_H