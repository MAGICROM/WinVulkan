#define LWC_MAKE_ID(a,b,c,d)	\
	((uint32_t) (d)<<24 | (uint32_t) (c)<<16 | \
	 (uint32_t) (b)<<8 | (uint32_t) (a))

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

typedef struct _COLORVALUE {
  float r;
  float g;
  float b;
  float a;
} COLORVALUE;

typedef struct _MATERIAL {
  COLORVALUE Diffuse;
  COLORVALUE Ambient;
  COLORVALUE Specular;
  COLORVALUE Emissive;
  float Power;
} MATERIAL;

struct LWCMATERIAL
{
    DWORD			dwStart;
	DWORD			dwNumber;
	MATERIAL     	material;
	BOOL			specular;
	BOOL			transparent;
	BOOL			mirror;
};

typedef struct SKELETON_WEIGHT
{
	//Container des monstres et objets
	DWORD			dwIndex;
	float			fValue;
}SKELETON_WEIGHT;

//Bones
struct LWCBONE
{
    int			id;
	int			parent;

	glm::vec3 center;
    glm::vec3 direction;
	glm::vec3 up;
};

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

void SKELETON_BONE::RotationYawPitchRoll(float yaw,float pitch,float roll)
{

	glm::mat4 identity(1.0f);
    
    glm::mat4 mTransportRelatif = glm::translate(identity,
                                    glm::vec3(direction.x,
					                direction.y,
					                direction.z)); 
		
    
    mSelfRotation = glm::rotate(identity, yaw, glm::vec3(1, 0, 0));
	mSelfRotation = glm::rotate(mSelfRotation, pitch, glm::vec3(0, 1, 0));
	mSelfRotation = glm::rotate(mSelfRotation, roll, glm::vec3(0, 0, 1));
	
    mSelfMatrice = mSelfRotation * mTransportRelatif;
}

typedef struct SKELETON_SQUELETTE
{
	uint8_t						cEtat;
	
	//Tableau des os
	
	uint32_t					dwNbrOs;
	SKELETON_BONE *				pOs;
	
	//Tableau de pointeurs sur les os pour calcul et index
	//pour copie
	
	LPSKELETON_BONE *			pRenderOs;
	uint16_t		*		    pwIndex;

	SKELETON_SQUELETTE():pOs(NULL),pRenderOs(NULL)	{}
	~SKELETON_SQUELETTE()							{}
	
	void Release(){	if(pOs)delete[]pOs;
					if(pRenderOs)delete[]pRenderOs;}

}SKELETON_SQUELETTE;
typedef struct SKELETON_SQUELETTE *LPSKELETON_SQUELETTE;

#define IS_CORRUPTED	(0)
#define IS_OK			(1)

struct LWCMODEL
{
    float	    fLowerPoint = 0.0f;
    int32_t		iModeltype = 0;
    int32_t		iNumLayer = 0;

    LWCMATERIAL* pLayers;
    snBuffer Vertices;
    snBuffer Indexes;

    int32_t	dwNumTriangles = 0;
    int32_t	dwNumVertex = 0;

    LPSKELETON_SQUELETTE	l_pSQUELETTE = nullptr;

   void PrepareSquelette(void);
};

void LWCMODEL::PrepareSquelette(void)
{
	l_pSQUELETTE->pRenderOs=new LPSKELETON_BONE[l_pSQUELETTE->dwNbrOs];
	
	if(l_pSQUELETTE->pRenderOs == NULL)
		{l_pSQUELETTE->cEtat=IS_CORRUPTED;return;}
		
	l_pSQUELETTE->pwIndex=new uint16_t[l_pSQUELETTE->dwNbrOs];

	if(l_pSQUELETTE->pwIndex == NULL)
		{l_pSQUELETTE->cEtat=IS_CORRUPTED;return;}
	
	uint16_t dwOsComputed=0L;

	//D'abord tous les Root bone 
	for(uint16_t dwN=0;dwN<l_pSQUELETTE->dwNbrOs;dwN++)
			{
			if(l_pSQUELETTE->pOs[dwN].pParent==NULL)
				{
				l_pSQUELETTE->pRenderOs[dwOsComputed]=&l_pSQUELETTE->pOs[dwN];
				l_pSQUELETTE->pwIndex[dwOsComputed++]=dwN;
				}
			l_pSQUELETTE->pOs[dwN].RotationYawPitchRoll(0.0f,0.0f,0.0f);
			}
	
	while(dwOsComputed < l_pSQUELETTE->dwNbrOs)
		{
		for(uint16_t dwN=0;dwN<l_pSQUELETTE->dwNbrOs;dwN++)
			{
			for(uint16_t dwRecur=0;dwRecur<dwOsComputed;dwRecur++)
				{
				if(l_pSQUELETTE->pOs[dwN].pParent==l_pSQUELETTE->pRenderOs[dwRecur])
					{
					l_pSQUELETTE->pRenderOs[dwOsComputed]=&l_pSQUELETTE->pOs[dwN];
					l_pSQUELETTE->pwIndex[dwOsComputed++]=dwN;
					}
				}
			}
		}
	
	for(uint32_t dwNBone=0;dwNBone<l_pSQUELETTE->dwNbrOs;dwNBone++)
			{
			LPSKELETON_BONE pBone=l_pSQUELETTE->pRenderOs[dwNBone];
					
				if(pBone->pParent)
					pBone->direction=pBone->center-pBone->pParent->center;
				else
					pBone->direction=pBone->center;
						
				pBone->RotationYawPitchRoll(0.0f,0.0f,0.0f);	
			}

	l_pSQUELETTE->cEtat=IS_OK;
}

#define READ_ON(_type_) *((_type_*)(data+pif));pif+=sizeof(_type_)
uint32_t readLwcFile(const char* filename,LWCMODEL** ppModel){
    std::vector<char> file = readFile(filename);
    if(!file.size())return 0; //PAS DE fiCHIER
    uint32_t pif = 0;
    char* data = file.data();
    uint32_t chunk=0;
    uint32_t chunk_size=0;
    uint32_t layers=0;
    uint32_t vert_size=0;
    //FORM//////////////////////////////////////////////////////////////////
    chunk = READ_ON(uint32_t);
    chunk_size = READ_ON(uint32_t);
    if(chunk!=LWC_ID_FORM)return 0;
    //LWC2//////////////////////////////////////////////////////////////////
    chunk = READ_ON(uint32_t);
    if(chunk!=LWC_ID_LWCF && chunk!=LWC_ID_LWC2)return 0;
    //Layers////////////////////////////////////////////////////////////////
    if(chunk==LWC_ID_LWC2)
    {
        (*ppModel) = new LWCMODEL;
        layers = READ_ON(uint32_t);
        (*ppModel)->pLayers = new LWCMATERIAL[layers];
        for(uint32_t i=0;i<layers;i++)
        {
           (*ppModel)->pLayers[i] = READ_ON(LWCMATERIAL);
        }
    }
    //Geometry
    do
    {
    chunk = READ_ON(uint32_t);
    chunk_size = READ_ON(uint32_t);
    if(chunk==LWC_ID_VERT)
	{
        uint32_t vertices = READ_ON(uint32_t);
        Vertex* pVertices = nullptr;
        
        (*ppModel)->Vertices.CreateTransferBuffer(nullptr,vertices*sizeof(Vertex));
        (*ppModel)->Vertices.Lock( 0, vertices*sizeof(Vertex), (void**)&pVertices, 0 );
        for(uint32_t dwN=0;dwN<vertices;dwN++)
        {
            pVertices[dwN] = READ_ON(Vertex);
            (*ppModel)->dwNumVertex++;
        } 
        (*ppModel)->Vertices.UnLock();
    }
    if(chunk==LWC_ID_INDX)
	{
        uint32_t indexes = READ_ON(uint32_t);
        uint16_t* pIndexes = nullptr;
       
        (*ppModel)->Indexes.CreateTransferBuffer( nullptr , indexes*sizeof(uint16_t) );
        (*ppModel)->Indexes.Lock( 0, indexes*sizeof(uint16_t), (void**)&pIndexes, 0 );
        for(uint32_t dwN=0;dwN<indexes;dwN++)
        {
            pIndexes[dwN] = READ_ON(uint16_t);
        } 
        (*ppModel)->Indexes.UnLock();
    }
    if(chunk==LWC_ID_BONE)
	{
        uint32_t dwNumBone = READ_ON(uint32_t);
		
		if(dwNumBone > 0)
			{
			(*ppModel)->l_pSQUELETTE=new SKELETON_SQUELETTE;	
			
			if((*ppModel)->l_pSQUELETTE == NULL )
				{return 0;}
		
			(*ppModel)->l_pSQUELETTE->pOs=new SKELETON_BONE[dwNumBone];
			(*ppModel)->l_pSQUELETTE->dwNbrOs=dwNumBone;
			if((*ppModel)->l_pSQUELETTE->pOs == NULL )
				{return 0;}
			}
		
		uint32_t localchunksize = READ_ON(uint32_t);
        glm::mat4 identity(1.0f);
		
		for(DWORD dwN=0;dwN<dwNumBone;dwN++)
			{
			LWCBONE TheBoneReaded = READ_ON(LWCBONE);
			
			if(TheBoneReaded.parent >= 0)
				(*ppModel)->l_pSQUELETTE->pOs[dwN].pParent=&(*ppModel)->l_pSQUELETTE->pOs[TheBoneReaded.parent];
			else
				(*ppModel)->l_pSQUELETTE->pOs[dwN].pParent=NULL;
			
			(*ppModel)->l_pSQUELETTE->pOs[dwN].parent		=TheBoneReaded.parent;
			(*ppModel)->l_pSQUELETTE->pOs[dwN].center		=TheBoneReaded.center;
			(*ppModel)->l_pSQUELETTE->pOs[dwN].direction	=TheBoneReaded.direction;
			(*ppModel)->l_pSQUELETTE->pOs[dwN].up			=TheBoneReaded.up;

			(*ppModel)->l_pSQUELETTE->pOs[dwN].mMatrice = glm::translate(identity,glm::vec3(
					(*ppModel)->l_pSQUELETTE->pOs[dwN].direction.x,
					(*ppModel)->l_pSQUELETTE->pOs[dwN].direction.y,
					(*ppModel)->l_pSQUELETTE->pOs[dwN].direction.z)); 
			
			uint32_t dwLocalChunk = READ_ON(uint32_t);
			if(dwLocalChunk != LWC_ID_WGHT )continue;
		    DWORD dwNumWeight = READ_ON(uint32_t);
		
			if(dwNumWeight > 0L)
				{
				(*ppModel)->l_pSQUELETTE->pOs[dwN].pWeightMap=new SKELETON_WEIGHT[dwNumWeight];
		
				if((*ppModel)->l_pSQUELETTE->pOs[dwN].pWeightMap == NULL )
					{return 0;}
				}
		
			for(DWORD dwNWeight=0;dwNWeight<dwNumWeight;dwNWeight++)
				{
				DWORD dwIndex = READ_ON(uint32_t);
				float fValue = READ_ON(float);
				
				if(dwIndex > ((*ppModel)->dwNumVertex-1))
							dwIndex=((*ppModel)->dwNumVertex-1);

				(*ppModel)->l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].dwIndex=dwIndex;
				(*ppModel)->l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].fValue=fValue;
				}
			
			(*ppModel)->l_pSQUELETTE->pOs[dwN].dwNbrPoints=dwNumWeight;
			
			//Next DATA
			localchunksize = READ_ON(uint32_t);
			}
		    (*ppModel)->PrepareSquelette();
        }
    } while (pif < file.size());
    return vert_size; 
}