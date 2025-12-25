#include <lwc_vulkan.h>

void SKELETON_BONE::RotationYawPitchRoll(float yaw,float pitch,float roll)
{
	glm::mat4 mTransportRelatif = glm::mat4(1.0f);
		
	mTransportRelatif = glm::translate(glm::mat4(1.0f),glm::vec3(direction.x, direction.y, direction.z));
	mSelfRotation = glm::rotate(glm::mat4(1.0f),yaw,glm::vec3(0, 0, 1.f)) *
					glm::rotate(glm::mat4(1.0f),pitch,glm::vec3(0, 1.f, 0)) *
					glm::rotate(glm::mat4(1.0f),roll,glm::vec3(1.f, 0, 0));
	
	mSelfMatrice = mSelfRotation * mTransportRelatif;
}

void LWC_MODELE::PrepareSquelette(void)
{
	
	l_pSQUELETTE->pRenderOs=new LPSKELETON_BONE[l_pSQUELETTE->dwNbrOs];
	
	if(l_pSQUELETTE->pRenderOs == NULL)
		{l_pSQUELETTE->cEtat=IS_CORRUPTED;return;}
		
	l_pSQUELETTE->pwIndex=new WORD[l_pSQUELETTE->dwNbrOs];

	if(l_pSQUELETTE->pwIndex == NULL)
		{l_pSQUELETTE->cEtat=IS_CORRUPTED;return;}
	
	WORD dwOsComputed=0L;

	//D'abord tous les Root bone 
	for(WORD dwN=0;dwN<l_pSQUELETTE->dwNbrOs;dwN++)
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
		for(WORD dwN=0;dwN<l_pSQUELETTE->dwNbrOs;dwN++)
			{
			for(WORD dwRecur=0;dwRecur<dwOsComputed;dwRecur++)
				{
				if(l_pSQUELETTE->pOs[dwN].pParent==l_pSQUELETTE->pRenderOs[dwRecur])
					{
					l_pSQUELETTE->pRenderOs[dwOsComputed]=&l_pSQUELETTE->pOs[dwN];
					l_pSQUELETTE->pwIndex[dwOsComputed++]=dwN;
					}
				}
			}
		}
	
	for(DWORD dwNBone=0;dwNBone<l_pSQUELETTE->dwNbrOs;dwNBone++)
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

void LWC_MODELE::ReadFromFile(LPVULKANDEVICE9 l_pd3dDevice,TCHAR* szLocation)
{
	if(!l_pd3dDevice)return;
	
	DWORD Position_fichier;

	DWORD Taille_fichier;
	DWORD Taille_fichier_Haute;
	
	DWORD Taille_totale;
	DWORD Taille_chunk;
	
	DWORD Taille_alire;
	DWORD Taille_lu;
	DWORD Fourcc_chunk;
	DWORD Number_Layer;

	fLowerPoint=0.0f;
	//OUVERTURE***********************************************************************
	HANDLE  hBinFile = CreateFile(	szLocation,
								GENERIC_READ,0,NULL,OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,NULL);

	if(hBinFile == INVALID_HANDLE_VALUE)return;

	Taille_fichier = GetFileSize(hBinFile,&Taille_fichier_Haute);
	SetFilePointer(hBinFile,0,NULL,FILE_BEGIN);
//********************************************************************************

	Lwc_ReadCHUNK(hBinFile,&Fourcc_chunk,&Taille_totale);
	
	Taille_alire=Taille_totale;
	
	if(Fourcc_chunk!=LWC_ID_FORM)return;

	Taille_alire	-=	Lwc_ReadCHUNK(hBinFile,&Fourcc_chunk,NULL);
		
	if(Fourcc_chunk!=LWC_ID_LWCF && Fourcc_chunk!=LWC_ID_LWC2)return;

	iNumLayer=0;
	
	if(Fourcc_chunk==LWC_ID_LWC2)
	{
	Taille_alire	-=	Lwc_ReadDWORD(hBinFile,&Number_Layer);
	l_pLAYER=new LWCMATERIAL[Number_Layer];
	iNumLayer=Number_Layer;
	
	if(l_pLAYER)
		{
		for(DWORD dwN=0;dwN<Number_Layer;dwN++)
			{
			Taille_alire	-=	Lwc_ReadMATERIAL(hBinFile,&l_pLAYER[dwN]);
			Taille_alire=Taille_alire;
			}
		}
	}
	
	while(Taille_alire > 0)
	{
	//Read the next chunk
	Taille_alire	-=	Lwc_ReadCHUNK(hBinFile,&Fourcc_chunk,&Taille_chunk);
	//Get the current position in the files
	Position_fichier=SetFilePointer(hBinFile,0,NULL,FILE_CURRENT);
	//And change to next valid CHUNK position	
	Position_fichier+=Taille_chunk;
	//******************************************************
	//DEBUT DE LIRE LES CHUNK
	//******************************************************
	
	//Vertices chunk
	if(Fourcc_chunk==LWC_ID_VERT)
		{
		DWORD dwLocalChunkSize=Taille_chunk;
		DWORD dwNumVertices = 0L;
		
		ReadFile(hBinFile,(LPVOID*)&dwNumVertices,sizeof(DWORD),&Taille_lu,NULL);
		dwLocalChunkSize-=Taille_lu;
	
		if( FAILED( l_pd3dDevice->CreateVertexBuffer( (dwNumVertices+1)*sizeof(LWCVERTEX),
                                       D3DUSAGE_WRITEONLY, D3DFVF_LWCVERTEX,
                                       D3DPOOL_MANAGED, &l_pVERTEXBUFFER ) ) ){CloseHandle(hBinFile);return;}
		
		for(DWORD dwN=0;dwN<dwNumVertices;dwN++)
		{
		VOID* pVertices;

		LWCVERTEX Vertice;
		
	
			
		ReadFile(hBinFile,(LPVOID*)&Vertice,sizeof(LWCVERTEX),&Taille_lu,NULL);
		dwLocalChunkSize-=Taille_lu;
		
		l_pVERTEXBUFFER->Lock( dwN*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pVertices, 0 );
		memcpy( pVertices, &Vertice, sizeof(LWCVERTEX) );
		l_pVERTEXBUFFER->Unlock();  

			if(Vertice.p.y<fLowerPoint)
				fLowerPoint=Vertice.p.y;
		
		}
		dwNumVertex=dwNumVertices;
		dwNumTriangles=dwNumVertices/3;
		iModeltype=MODELTYPE_TRIANGLELIST;
		}
	
	//Index chunk
	if(Fourcc_chunk==LWC_ID_INDX)
		{
		DWORD dwLocalChunkSize=Taille_chunk;
		DWORD dwNumIndices = Taille_chunk / sizeof(DWORD);
		
		if( FAILED( l_pd3dDevice->CreateIndexBuffer( (dwNumIndices+1)*sizeof(WORD),
                                       D3DUSAGE_WRITEONLY  
										, D3DFMT_INDEX16,
                                       D3DPOOL_MANAGED, &l_pINDEXBUFFER ) ) ){CloseHandle(hBinFile);return;}
	
		for(DWORD dwN=0;dwN<dwNumIndices;dwN++)
		{
		VOID* pIndices;
		DWORD dwIndice;
		WORD Indice;
		
		ReadFile(hBinFile,(LPVOID*)&dwIndice,sizeof(DWORD),&Taille_lu,NULL);
		dwLocalChunkSize-=Taille_lu;

		Indice=(WORD)dwIndice;
		l_pINDEXBUFFER->Lock( dwN*sizeof(WORD), sizeof(WORD), (BYTE**)&pIndices, 0 );
		memcpy( pIndices, &Indice, sizeof(WORD) );
		l_pINDEXBUFFER->Unlock();  
		
		}
		dwNumTriangles=dwNumIndices/3;
		iModeltype=MODELTYPE_INDEXLIST;
		DWORD dwNjjmBone=LWC_ID_BONE;
		dwNjjmBone=LWC_ID_BONE;
		}
	
	//Index chunk
	if(Fourcc_chunk==LWC_ID_BONE)
		{
		DWORD dwLocalChunkSize=Taille_chunk;
		DWORD dwLocalChunk=0L;
		DWORD dwNumBone = 0L;
			
		ReadFile(hBinFile,(LPVOID*)&dwNumBone,sizeof(DWORD),&Taille_lu,NULL);
		dwLocalChunkSize-=Taille_lu;
		
		if(dwNumBone > 0)
			{
			l_pSQUELETTE=new SKELETON_SQUELETTE;	
			
			if(l_pSQUELETTE == NULL )
				{CloseHandle(hBinFile);return;}
		
			l_pSQUELETTE->pOs=new SKELETON_BONE[dwNumBone];
			l_pSQUELETTE->dwNbrOs=dwNumBone;
			if(l_pSQUELETTE->pOs == NULL )
				{CloseHandle(hBinFile);return;}
			}
		
		dwLocalChunkSize -=	Lwc_ReadCHUNK(hBinFile,&dwLocalChunk,NULL);
		
		for(DWORD dwN=0;dwN<dwNumBone;dwN++)
			{
			LWCBONE TheBoneReaded;
		
			dwLocalChunkSize -=	Lwc_ReadBONE(hBinFile,&TheBoneReaded);
			
			if(TheBoneReaded.parent >= 0)
				l_pSQUELETTE->pOs[dwN].pParent=&l_pSQUELETTE->pOs[TheBoneReaded.parent];
			else
				l_pSQUELETTE->pOs[dwN].pParent=NULL;
			
			l_pSQUELETTE->pOs[dwN].parent		=TheBoneReaded.parent;
			l_pSQUELETTE->pOs[dwN].center		=TheBoneReaded.center;
			l_pSQUELETTE->pOs[dwN].direction	=TheBoneReaded.direction;
			l_pSQUELETTE->pOs[dwN].up			=TheBoneReaded.up;
		
			D3DXMatrixTranslation(
			&l_pSQUELETTE->pOs[dwN].mMatrice,
					l_pSQUELETTE->pOs[dwN].direction.x,
					l_pSQUELETTE->pOs[dwN].direction.y,
					l_pSQUELETTE->pOs[dwN].direction.z); 
			
			dwLocalChunkSize -=	Lwc_ReadCHUNK(hBinFile,&dwLocalChunk,NULL);
			if(dwLocalChunk != LWC_ID_WGHT )continue;
		
			DWORD dwNumWeight = 0L;
		
			ReadFile(hBinFile,(LPVOID*)&dwNumWeight,sizeof(DWORD),&Taille_lu,NULL);
			dwLocalChunkSize-=Taille_lu;

			if(dwNumWeight > 0L)
				{
				l_pSQUELETTE->pOs[dwN].pWeightMap=new SKELETON_WEIGHT[dwNumWeight];
		
				if(l_pSQUELETTE->pOs[dwN].pWeightMap == NULL )
					{CloseHandle(hBinFile);return;}
				}
		
			for(DWORD dwNWeight=0;dwNWeight<dwNumWeight;dwNWeight++)
				{
				DWORD dwIndex;
				float fValue;
			
				ReadFile(hBinFile,(LPVOID*)&dwIndex,sizeof(DWORD),&Taille_lu,NULL);
					dwLocalChunkSize-=Taille_lu;
				ReadFile(hBinFile,(LPVOID*)&fValue,sizeof(float),&Taille_lu,NULL);
					dwLocalChunkSize-=Taille_lu;
				
				if(dwIndex > (dwNumVertex-1))
							dwIndex=(dwNumVertex-1);

				l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].dwIndex=dwIndex;
				l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].fValue=fValue;
				}
			
			l_pSQUELETTE->pOs[dwN].dwNbrPoints=dwNumWeight;
			
			//Next DATA
			dwLocalChunkSize -=	Lwc_ReadCHUNK(hBinFile,&dwLocalChunk,NULL);
			}
		PrepareSquelette();
		}
	//******************************************************
	//FIN DE LIRE LES CHUNK
	//******************************************************
	//Realignement
	SetFilePointer(hBinFile,Position_fichier,NULL,FILE_BEGIN);
	Taille_alire-=Taille_chunk;
	}
	

	//FERMETURE**********************************************************************	

	CloseHandle(hBinFile);
}

extern LPVULKANTEXTURE8*				g_pWindunTextures;
BOOL g_bDrawAPortrait =false;

void LWC_MODELE::ChangeMirrorForPortrait(LPVULKANDEVICE9 l_pd3dDevice,int Portrait)
{
				int ligne = (Portrait)/8;
				int colonne = (Portrait)-(8*ligne);

				float tu1=colonne*0.125f;
				float tu2=tu1+0.125f;
				float tv1=(1.0f-(3.0f*0.11328125f))+(ligne*0.11328125f);
				float tv2=tv1+0.11328125f;

				VOID* pOriginalVertices;
				
				
				l_pVERTEXBUFFER->Lock( 72*sizeof(LWCVERTEX), sizeof(LWCVERTEX)*6, (BYTE**)&pOriginalVertices, 0 );
			
				LWCVERTEX*	pOriginal=(LWCVERTEX*)pOriginalVertices;

				pOriginal[0].tu=tu1;
				pOriginal[0].tv=tv2;
				pOriginal[1].tu=tu1;
				pOriginal[1].tv=tv1;
				pOriginal[2].tu=tu2;
				pOriginal[2].tv=tv2;
				pOriginal[3].tu=tu2;
				pOriginal[3].tv=tv2;
				pOriginal[4].tu=tu1;
				pOriginal[4].tv=tv1;
				pOriginal[5].tu=tu2;
				pOriginal[5].tv=tv1;

				l_pVERTEXBUFFER->Unlock();  
				g_bDrawAPortrait=true;
}
		
void LWC_MODELE::Render(LPVULKANDEVICE9 l_pd3dDevice,LPVOID pData,BOOL bUseMaterial)
{
	l_pd3dDevice->SetVertexShader( D3DFVF_LWCVERTEX );
	l_pd3dDevice->SetStreamSource( 0, l_pVERTEXBUFFER, sizeof(LWCVERTEX) );

	D3DXMATRIX worldmat;
	D3DXMATRIX identity;
	
	if(iNumLayer)
		{
		if(iModeltype==MODELTYPE_TRIANGLELIST)
			{
			for(int n=0;n<iNumLayer;n++)
				{
				if(pData)
				{
					if(n==1)
					l_pd3dDevice->SetTransform( D3DTS_WORLD, &((WD_COMPAS*)pData)->mNord );	
					else
					l_pd3dDevice->SetTransform( D3DTS_WORLD, &((WD_COMPAS*)pData)->mNormal );	
				}
				
				D3DXMATRIX mirrormat;
					mirrormat._11 = 0.5f; mirrormat._12 = 0.0f; mirrormat._13 = 0.0f; mirrormat._14 = 0.0f; 
					mirrormat._21 = 0.0f; mirrormat._22 =-0.5f; mirrormat._23 = 0.0f; mirrormat._24 = 0.0f; 
					mirrormat._31 = 0.0f; mirrormat._32 = 0.0f; mirrormat._33 = 1.0f; mirrormat._34 = 0.0f; 
					mirrormat._41 = 0.5f; mirrormat._42 = 0.5f; mirrormat._43 = 0.0f; mirrormat._44 = 1.0f; 
				
				/*if(bUseMaterial)
					l_pd3dDevice->SetMaterial( &l_pLAYER[n].material );*/
					
				if(l_pLAYER[n].transparent && bUseMaterial)
				{
				l_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCCOLOR );
				l_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_DESTCOLOR );
				l_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	TRUE );
				//l_pd3dDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );
				//l_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,		FALSE );
				}
				if(l_pLAYER[n].specular && bUseMaterial)
				{
				l_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE ); 
				}
				
				if(l_pLAYER[n].mirror && bUseMaterial)
				{
				l_pd3dDevice->SetTexture( 0, *(g_pWindunTextures+TEXTURE_MIRROR) );
				l_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mirrormat );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
		
				//l_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
				//l_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
				//l_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
				//l_pd3dDevice->SetRenderState	( D3DRS_LIGHTING, FALSE );
				}

				l_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST , l_pLAYER[n].dwStart, l_pLAYER[n].dwNumber);	
				
				if(bUseMaterial)
				{
				l_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE,		FALSE ); 
				//l_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
				l_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	FALSE );
				l_pd3dDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );
				l_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,		TRUE );
				if(g_bDrawAPortrait && n==0)l_pd3dDevice->SetTexture( 0, *(g_pWindunTextures+TEXTURE_INVENTAIRE) );
				if(g_bDrawAPortrait && n==1){l_pd3dDevice->SetTexture( 0, *(g_pWindunTextures+TEXTURE_SWITCH1) );g_bDrawAPortrait=false;}
				//l_pd3dDevice->SetTexture( 0, *(g_pWindunTextures+TEXTURE_OBJET0) );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
				}
				}
			}
		}
	else
		{
		if(iModeltype==MODELTYPE_TRIANGLELIST)
			{
			l_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST , 0L, dwNumTriangles);	
			}
			else
			{
			l_pd3dDevice->SetIndices( l_pINDEXBUFFER, 0 );
			l_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dwNumVertex, 0, dwNumTriangles);
			}
		}
}

void LWC_MODELE::Release(void)
{
SAFE_RELEASE(l_pVERTEXBUFFER);  
SAFE_RELEASE(l_pINDEXBUFFER);  
SAFE_RELEASE(l_pSQUELETTE);  
}

void LWC_MODELE::CreateSculpt(LPVULKANDEVICE9 l_pd3dDevice,LPLWC_SCULPT* lp_pSculpt)
{

	*lp_pSculpt=new LWC_SCULPT;

	(*lp_pSculpt)->l_pSOURCEVERTEXBUFFER	=l_pVERTEXBUFFER;
	(*lp_pSculpt)->l_pSOURCEINDEXBUFFER		=l_pINDEXBUFFER;
	(*lp_pSculpt)->iModeltype				=iModeltype;
	(*lp_pSculpt)->dwNumTriangles			=dwNumTriangles;
	(*lp_pSculpt)->dwNumVertex				=dwNumVertex;
	(*lp_pSculpt)->iNumLayer				=iNumLayer;
	(*lp_pSculpt)->l_pLAYER					=l_pLAYER;

	if(l_pSQUELETTE)
	{
		if( FAILED( l_pd3dDevice->CreateVertexBuffer( (dwNumVertex+1)*sizeof(LWCVERTEX),
                                       D3DUSAGE_WRITEONLY, D3DFVF_LWCVERTEX,
                                       D3DPOOL_MANAGED, &(*lp_pSculpt)->l_pVERTEXBUFFER ) ) ){return;}
	
		//On copie tout les points
		for(DWORD dwNPoint=0;dwNPoint<dwNumVertex;dwNPoint++)
				{
				VOID* pOriginalVertices;
				VOID* pComputedVertices;
				
				l_pVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pOriginalVertices, 0 );
				(*lp_pSculpt)->l_pVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pComputedVertices, 0 );
				
				LWCVERTEX*	pComputed=(LWCVERTEX*)pComputedVertices;
				LWCVERTEX*	pOriginal=(LWCVERTEX*)pOriginalVertices;

				*pComputed=*pOriginal;
				
				(*lp_pSculpt)->l_pVERTEXBUFFER->Unlock();  
				l_pVERTEXBUFFER->Unlock();  
				}
	
	//On copie le squelette si il y en a un
		if(l_pSQUELETTE->dwNbrOs > 0)
			{
			(*lp_pSculpt)->l_pSQUELETTE=new SKELETON_SQUELETTE;	
			
			if((*lp_pSculpt)->l_pSQUELETTE == NULL )
				{return;}
		
			(*lp_pSculpt)->l_pSQUELETTE->pOs=new SKELETON_BONE[l_pSQUELETTE->dwNbrOs];
			(*lp_pSculpt)->l_pSQUELETTE->dwNbrOs=l_pSQUELETTE->dwNbrOs;
			if((*lp_pSculpt)->l_pSQUELETTE->pOs == NULL )
				{return;}
			}
		
		for(DWORD dwN=0;dwN<l_pSQUELETTE->dwNbrOs;dwN++)
			{
			(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN]=l_pSQUELETTE->pOs[dwN];
		
				if(l_pSQUELETTE->pOs[dwN].dwNbrPoints > 0)
				{
				(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap=new SKELETON_WEIGHT[l_pSQUELETTE->pOs[dwN].dwNbrPoints];
		
				if((*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap == NULL )
					{return;}
				}
		
			for(DWORD dwNWeight=0;dwNWeight<l_pSQUELETTE->pOs[dwN].dwNbrPoints;dwNWeight++)
				{
				(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].dwIndex=
					l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].dwIndex;
				(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].fValue=
					l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].fValue;
				}
			
			(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].dwNbrPoints=l_pSQUELETTE->pOs[dwN].dwNbrPoints;
			
			}
		
		(*lp_pSculpt)->l_pSQUELETTE->pRenderOs=new LPSKELETON_BONE[l_pSQUELETTE->dwNbrOs];
	
		if((*lp_pSculpt)->l_pSQUELETTE->pRenderOs == NULL)
			{(*lp_pSculpt)->l_pSQUELETTE->cEtat=IS_CORRUPTED;return;}
		
		(*lp_pSculpt)->l_pSQUELETTE->pwIndex=NULL;
			
		for(dwN=0;dwN<(*lp_pSculpt)->l_pSQUELETTE->dwNbrOs;dwN++)
			{
			if((*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].parent >= 0)
				(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].pParent=&(*lp_pSculpt)->l_pSQUELETTE->pOs[(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].parent];
			else
				(*lp_pSculpt)->l_pSQUELETTE->pOs[dwN].pParent=NULL;

			(*lp_pSculpt)->l_pSQUELETTE->pRenderOs[dwN]=&(*lp_pSculpt)->l_pSQUELETTE->pOs[l_pSQUELETTE->pwIndex[dwN]];
			}
	}
	

}

void LWC_MODELE::FillSculpt(LPVULKANDEVICE9 l_pd3dDevice,LPLWC_SCULPT pSculpt)
{

	(pSculpt)->l_pSOURCEVERTEXBUFFER	=l_pVERTEXBUFFER;
	(pSculpt)->l_pSOURCEINDEXBUFFER		=l_pINDEXBUFFER;
	(pSculpt)->iModeltype				=iModeltype;
	(pSculpt)->dwNumTriangles			=dwNumTriangles;
	(pSculpt)->dwNumVertex				=dwNumVertex;
	(pSculpt)->iNumLayer				=iNumLayer;
	(pSculpt)->l_pLAYER					=l_pLAYER;

	
		if( FAILED( l_pd3dDevice->CreateVertexBuffer( (dwNumVertex+1)*sizeof(LWCVERTEX),
                                       D3DUSAGE_WRITEONLY, D3DFVF_LWCVERTEX,
                                       D3DPOOL_MANAGED, &(pSculpt)->l_pVERTEXBUFFER ) ) ){return;}
	
		//On copie tout les points
		for(DWORD dwNPoint=0;dwNPoint<dwNumVertex;dwNPoint++)
				{
				VOID* pOriginalVertices;
				VOID* pComputedVertices;
				
				l_pVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pOriginalVertices, 0 );
				(pSculpt)->l_pVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pComputedVertices, 0 );
				
				LWCVERTEX*	pComputed=(LWCVERTEX*)pComputedVertices;
				LWCVERTEX*	pOriginal=(LWCVERTEX*)pOriginalVertices;

				*pComputed=*pOriginal;
				
				(pSculpt)->l_pVERTEXBUFFER->Unlock();  
				l_pVERTEXBUFFER->Unlock();  
				}
	
	if(l_pSQUELETTE)
	{//On copie le squelette si il y en a un
		if(l_pSQUELETTE->dwNbrOs > 0)
			{
			(pSculpt)->l_pSQUELETTE=new SKELETON_SQUELETTE;	
			
			if((pSculpt)->l_pSQUELETTE == NULL )
				{return;}
		
			(pSculpt)->l_pSQUELETTE->pOs=new SKELETON_BONE[l_pSQUELETTE->dwNbrOs];
			(pSculpt)->l_pSQUELETTE->dwNbrOs=l_pSQUELETTE->dwNbrOs;
			if((pSculpt)->l_pSQUELETTE->pOs == NULL )
				{return;}
			}
		
		for(DWORD dwN=0;dwN<l_pSQUELETTE->dwNbrOs;dwN++)
			{
			(pSculpt)->l_pSQUELETTE->pOs[dwN]=l_pSQUELETTE->pOs[dwN];
		
				if(l_pSQUELETTE->pOs[dwN].dwNbrPoints > 0)
				{
				(pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap=new SKELETON_WEIGHT[l_pSQUELETTE->pOs[dwN].dwNbrPoints];
		
				if((pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap == NULL )
					{return;}
				}
		
			for(DWORD dwNWeight=0;dwNWeight<l_pSQUELETTE->pOs[dwN].dwNbrPoints;dwNWeight++)
				{
				(pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].dwIndex=
					l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].dwIndex;
				(pSculpt)->l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].fValue=
					l_pSQUELETTE->pOs[dwN].pWeightMap[dwNWeight].fValue;
				}
			
			(pSculpt)->l_pSQUELETTE->pOs[dwN].dwNbrPoints=l_pSQUELETTE->pOs[dwN].dwNbrPoints;
			
			}
		
		(pSculpt)->l_pSQUELETTE->pRenderOs=new LPSKELETON_BONE[l_pSQUELETTE->dwNbrOs];
	
		if((pSculpt)->l_pSQUELETTE->pRenderOs == NULL)
			{(pSculpt)->l_pSQUELETTE->cEtat=IS_CORRUPTED;return;}
		
		(pSculpt)->l_pSQUELETTE->pwIndex=NULL;
			
		for(dwN=0;dwN<(pSculpt)->l_pSQUELETTE->dwNbrOs;dwN++)
			{
			if((pSculpt)->l_pSQUELETTE->pOs[dwN].parent >= 0)
				(pSculpt)->l_pSQUELETTE->pOs[dwN].pParent=&(pSculpt)->l_pSQUELETTE->pOs[(pSculpt)->l_pSQUELETTE->pOs[dwN].parent];
			else
				(pSculpt)->l_pSQUELETTE->pOs[dwN].pParent=NULL;

			(pSculpt)->l_pSQUELETTE->pRenderOs[dwN]=&(pSculpt)->l_pSQUELETTE->pOs[l_pSQUELETTE->pwIndex[dwN]];
			}
	}
	

}

//0 = Colonne
//1 = DEpaule
//2 = DBras
//3 = DABras
//4 = DMain
//5 = GEpaule
//6 = GBras
//7 = GABras
//8 = GMain
//9 = Cou
//10 = Tete
//11 = Base
//12 = DHanche
//13 = DCuisse
//14 = DMollet
//15 = DPied
//16 = GHanche
//17 = GCuisse
//18 = GMollet
//19 = GPied

void LWC_SCULPT::MoveUV(float u,float v,int layer)
{
	if(iNumLayer)
		{
		if(iModeltype==MODELTYPE_TRIANGLELIST)
			{
			if(layer<iNumLayer)
				{
				//On remet tout les points et les normales a zero
				for(DWORD dwNPoint=l_pLAYER[layer].dwStart;dwNPoint<l_pLAYER[layer].dwStart+l_pLAYER[layer].dwNumber*3;dwNPoint++)
				{
				VOID* pOriginalVertices;
				VOID* pComputedVertices;
				
				l_pSOURCEVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pOriginalVertices, 0 );
				l_pVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pComputedVertices, 0 );
				
				LWCVERTEX*	pComputed=(LWCVERTEX*)pComputedVertices;
				LWCVERTEX*	pOriginal=(LWCVERTEX*)pOriginalVertices;

				pComputed->tu=pOriginal->tu+u;
				pComputed->tv=pOriginal->tv+v;
								
				l_pVERTEXBUFFER->Unlock();  
				l_pSOURCEVERTEXBUFFER->Unlock();  
				}
				}
			}
		}
}

void LWC_SCULPT::RenderSquelette(void)
{
	if(!l_pSQUELETTE)return;
	
	//On remet tout les points et les normales a zero
	for(DWORD dwNPoint=0;dwNPoint<dwNumVertex;dwNPoint++)
				{
				VOID* pOriginalVertices;
				VOID* pComputedVertices;
				
				l_pSOURCEVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pOriginalVertices, 0 );
				l_pVERTEXBUFFER->Lock( dwNPoint*sizeof(LWCVERTEX), sizeof(LWCVERTEX), (BYTE**)&pComputedVertices, 0 );
				
				LWCVERTEX*	pComputed=(LWCVERTEX*)pComputedVertices;
				LWCVERTEX*	pOriginal=(LWCVERTEX*)pOriginalVertices;

				*pComputed=*pOriginal;
					
				pComputed->p=D3DXVECTOR3(0.0f,0.0f,0.0f);
				pComputed->n=D3DXVECTOR3(0.0f,0.0f,0.0f);
				
				l_pVERTEXBUFFER->Unlock();  
				l_pSOURCEVERTEXBUFFER->Unlock();  
				}
	
	//On calcule la cinematique de chaque os
	for(DWORD dwNBone=0;dwNBone<l_pSQUELETTE->dwNbrOs;dwNBone++)
			{
			LPSKELETON_BONE pBone=l_pSQUELETTE->pRenderOs[dwNBone];
			
			if(pBone->pParent)
					{
					D3DXMatrixMultiply(
					&pBone->mMatrice,
					&pBone->mSelfMatrice,
					&pBone->pParent->mMatrice);
					D3DXMatrixMultiply(
					&pBone->mRotation,
					&pBone->mSelfRotation,
					&pBone->pParent->mRotation);
					}
				else
					{
					pBone->mMatrice=pBone->mSelfMatrice;
					pBone->mRotation=pBone->mSelfRotation;
					}
			
			VOID* pComputedVertices;
			VOID* pOriginalVertices;
				
			l_pSOURCEVERTEXBUFFER->Lock( 0, sizeof(LWCVERTEX)*dwNumVertex, (BYTE**)&pOriginalVertices, D3DLOCK_READONLY );
			l_pVERTEXBUFFER->Lock( 0, sizeof(LWCVERTEX)*dwNumVertex, (BYTE**)&pComputedVertices, 0 );
			
			for(DWORD dwNWeight=0;dwNWeight<pBone->dwNbrPoints;dwNWeight++)
				{
				
				LWCVERTEX*	pComputed=((LWCVERTEX*)pComputedVertices)+pBone->pWeightMap[dwNWeight].dwIndex;
				LWCVERTEX*	pOriginal=((LWCVERTEX*)pOriginalVertices)+pBone->pWeightMap[dwNWeight].dwIndex;

				D3DXVECTOR3 post(0.0f,0.0f,0.0f);
				D3DXVECTOR3 pre=pOriginal->p-pBone->center;
									
				//Les points sont orientees et transposees
				//D3DXVec3TransformCoord(	&post,&pre,&pBone->mMatrice);
				D3DXVec3TransformCoord(	&post,&pre,&pBone->mMatrice);
				pComputed->p+=post*pBone->pWeightMap[dwNWeight].fValue;
				
				//Les normales sont correctement orientees
				D3DXVec3TransformCoord(	&post,&pOriginal->n,&pBone->mRotation);
				pComputed->n+=post*pBone->pWeightMap[dwNWeight].fValue;
				
				//memcpy( ((LWCVERTEX*)pComputedVertices)+pBone->pWeightMap[dwNWeight].dwIndex, pComputed, sizeof(LWCVERTEX) );
				
				
				}

			l_pVERTEXBUFFER->Unlock();  
			l_pSOURCEVERTEXBUFFER->Unlock();  
			}
}

void LWC_SCULPT::Render(LPVULKANDEVICE9 l_pd3dDevice)
{
	//RenderSquelette();
		
	l_pd3dDevice->SetVertexShader( D3DFVF_LWCVERTEX );
	if(l_pVERTEXBUFFER)
		l_pd3dDevice->SetStreamSource( 0, l_pVERTEXBUFFER, sizeof(LWCVERTEX) );
	else
		l_pd3dDevice->SetStreamSource( 0, l_pSOURCEVERTEXBUFFER, sizeof(LWCVERTEX) );

	if(iNumLayer)
		{
		if(iModeltype==MODELTYPE_TRIANGLELIST)
			{
			for(int n=0;n<iNumLayer;n++)
				{
				D3DXMATRIX mirrormat;
					mirrormat._11 = 0.5f; mirrormat._12 = 0.0f; mirrormat._13 = 0.0f; mirrormat._14 = 0.0f; 
					mirrormat._21 = 0.0f; mirrormat._22 =-0.5f; mirrormat._23 = 0.0f; mirrormat._24 = 0.0f; 
					mirrormat._31 = 0.0f; mirrormat._32 = 0.0f; mirrormat._33 = 1.0f; mirrormat._34 = 0.0f; 
					mirrormat._41 = 0.5f; mirrormat._42 = 0.5f; mirrormat._43 = 0.0f; mirrormat._44 = 1.0f; 
				
				//l_pd3dDevice->SetMaterial( &l_pLAYER[n].material );
					
				if(l_pLAYER[n].transparent)
				{
				l_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,			D3DBLEND_SRCCOLOR );
				l_pd3dDevice->SetRenderState( D3DRS_DESTBLEND,			D3DBLEND_DESTCOLOR );
				l_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	TRUE );
				//l_pd3dDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_NONE );
				//l_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,		FALSE );
				}
				if(l_pLAYER[n].specular)
				{
				l_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE ); 
				}
				
				if(l_pLAYER[n].mirror)
				{
				l_pd3dDevice->SetTexture( 0, *(g_pWindunTextures+TEXTURE_MIRROR) );
				l_pd3dDevice->SetTransform( D3DTS_TEXTURE0, &mirrormat );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACENORMAL );
		
				//l_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
				//l_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
				//l_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
				l_pd3dDevice->SetRenderState	( D3DRS_LIGHTING, FALSE );
				}

				l_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST , l_pLAYER[n].dwStart, l_pLAYER[n].dwNumber);	
				
				l_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE,		FALSE ); 
				//l_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
				l_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,	FALSE );
				l_pd3dDevice->SetRenderState( D3DRS_CULLMODE,			D3DCULL_CCW );
				l_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,		TRUE );
				//l_pd3dDevice->SetTexture( 0, *(g_pWindunTextures+TEXTURE_OBJET0) );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
				l_pd3dDevice->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU  );
				}
			}
		}
	else
		{
		if(iModeltype==MODELTYPE_TRIANGLELIST)
			{
			l_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST , 0L, dwNumTriangles);	
			}
			else
			{
			l_pd3dDevice->SetIndices( l_pSOURCEINDEXBUFFER, 0 );
			l_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, dwNumVertex, 0, dwNumTriangles);
			}
		}
}

void LWC_SCULPT::Release(void)
{
	//Present uniquement dans le modele
		l_pSOURCEINDEXBUFFER=NULL;
		l_pSOURCEVERTEXBUFFER=NULL;
		l_pLAYER=NULL;
	//Present ici
		SAFE_RELEASE(l_pVERTEXBUFFER);  
		SAFE_RELEASE(l_pSQUELETTE);  
}

void LoadMotion(TCHAR* szLocation,LWCHUMANFRAME** frames)
{
	DWORD Taille_fichier;
	DWORD Taille_fichier_Haute;
	DWORD Taille_lu;
	DWORD NombreDeFrame;	
//OUVERTURE***********************************************************************
	HANDLE  hBinFile = CreateFile(	szLocation,
								GENERIC_READ,0,NULL,OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,NULL);

	if(hBinFile == INVALID_HANDLE_VALUE)return;

	Taille_fichier = GetFileSize(hBinFile,&Taille_fichier_Haute);
	SetFilePointer(hBinFile,0,NULL,FILE_BEGIN);
//********************************************************************************

	ReadFile(hBinFile,(LPVOID*)&NombreDeFrame,sizeof(DWORD),&Taille_lu,NULL);
	if(NombreDeFrame > 0)
	{
	(*frames)=new LWCHUMANFRAME[NombreDeFrame];
	}
	
	if((*frames))
	{
		LWCHUMANFRAME forread;
		for(DWORD dwI=0L;dwI<NombreDeFrame;dwI++)
		{
		ReadFile(hBinFile,&forread,sizeof(LWCHUMANFRAME),&Taille_lu,NULL);
		(*frames)[dwI]=forread;
		}
	
	
	}
	CloseHandle(hBinFile);
}
