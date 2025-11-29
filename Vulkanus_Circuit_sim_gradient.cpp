//20 Mars 2023

float Matrice[] = {       0.001f,    -0.001f,        0.f,    0.f,   1.f,                       
                         -0.001f,     0.002f,    -0.001f,    0.f,   0.f,           
                             0.f,    -0.001f,     0.002f,-0.001f,   0.f,           
                             0.f,        0.f,    -0.001f, 0.001f,  -1.f,      
                             1.f,        0.f,        0.f,   -1.f,   0.f};      
float VX1[] = {0.f,0.f,0.f,0.f,0.f};
float VX2[] = {0.f,0.f,0.f,0.f,5.f};

void Matrix_by_vector(float* vr,float* M, float*v, int row_len)
{
    for(int i=0;i<row_len;i++)
    {
        vr[i] = 0.0f;
        for(int j=0;j<row_len;j++)
        {
            vr[i]+=M[row_len*i+j]*v[j];
        }
    }
}

void Vector_Sub(float* vr,float*v1,float* v2, int row_len)
{
    for(int i=0;i<row_len;i++)
    {
        vr[i]=v1[i]-v2[i];
    }
}

float Vector_Dot(float*v1,float* v2, int row_len)
{
    float v = 0.0f;
    for(int i=0;i<row_len;i++)
    {
        v+=v1[i]*v2[i];
    }
    return v;
}

void v1_plus_abyv2(float* vr,float*v1,float a,float* v2, int row_len)
{
    float v = 0.0f;
    for(int i=0;i<row_len;i++)
    {
        vr[i]=v1[i]+a*v2[i];
    }
}

void v1_minus_abyv2(float* vr,float*v1,float a,float* v2, int row_len)
{
    float v = 0.0f;
    for(int i=0;i<row_len;i++)
    {
        vr[i]=v1[i]-a*v2[i];
    }
}

void v1_equal_v2(float* vr,float*v1,int row_len)
{
    float v = 0.0f;
    for(int i=0;i<row_len;i++)
    {
        vr[i]=v1[i];
    }
}

void vector_reverse(float* vr,float*v1,int row_len)
{
    float v = 0.0f;
    for(int i=0;i<row_len;i++)
    {
        vr[i]=v1[row_len-1-i];
    }
}


void ConjGradient(float* Xo,float* Matrice,float* Vecteur,int len)
{
    float Ro[len]={};
    float R1[len]={};
    float Po[len]={};
    float V[len]={};
    float V2[len]={};

    float alpha = 0.0f;
    float beta = 0.0f;
      

    vector_reverse(Xo,Vecteur,len);

    Matrix_by_vector(V,Matrice,Xo,len);
    Vector_Sub(Ro,Vecteur,V,len);
    v1_equal_v2(Po,Ro,len);
   
    for(int kk=0;kk<1000;kk){
    for(int k=0;k<1000;k++)
    {
        Matrix_by_vector(V,Matrice,Po,len);
        alpha = Vector_Dot(Ro,Ro,len)/Vector_Dot(V,Ro,len);
        v1_plus_abyv2(V,Xo,alpha,Po,len);
        v1_equal_v2(Xo,V,len);
        Matrix_by_vector(V2,Matrice,Po,len);
        v1_minus_abyv2(R1,Ro,alpha,V2,len);
        float f1 = Vector_Dot(R1,R1,len);
        if(f1 < 1.98559822e-37)break;
        beta = f1 / Vector_Dot(Ro,Ro,len);
        v1_plus_abyv2(V,R1,beta,Po,len);
        v1_equal_v2(Po,V,len);
        v1_equal_v2(Ro,R1,len);
    }
    kk++;
    }
} 

void Testconj()
{
    ConjGradient(VX1,Matrice,VX2,5);
}

//Copy circuit right side from original right side
//Copy circuit matrix from original circuit
//Chaque element dostep
//Nan ou Infinite STOP
//lu_factor(circuitMatrix, circuitMatrixSize, circuitPermute)
//lu_solve(circuitMatrix, circuitMatrixSize, circuitPermute, circuitRightSide);

/*#if !defined(AFX_CPUUSAGE_H__60CF4F03_9F01_41E8_A9FB_51F065D5F3C2__INCLUDED_)
#define AFX_CPUUSAGE_H__60CF4F03_9F01_41E8_A9FB_51F065D5F3C2__INCLUDED_
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <pdh.h>
#include <pdhmsg.h>

PPDHCOUNTERSTRUCT			m_pCounterStruct;
HQUERY						m_hQuery;

PdhOpenQuery(NULL, 1, &m_hQuery);

m_pCounterStruct = (PPDHCOUNTERSTRUCT) new PDHCOUNTERSTRUCT;
PdhAddCounter(m_hQuery, szCounterName, (DWORD) m_pCounterStruct, &(m_pCounterStruct->hCounter));
PdhCollectQueryData(m_hQuery);  */

//a[i][j] a[j*n+i]

// factors a matrix into upper and lower triangular matrices by
    // gaussian elimination.  On entry, a[0..n-1][0..n-1] is the
    // matrix to be factored.  ipvt[] returns an integer vector of pivot
    // indices, used in the lu_solve() routine.
boolean lu_factor(double *a, int n, int *ipvt) {
	int i,j,k;


	double scaleFactors[n] = {};
	
        // divide each row by its largest element, keeping track of the
	// scaling factors
	for (i = 0; i != n; i++) { 
	    double largest = 0;
	    for (j = 0; j != n; j++) {
		double x = abs(a[i*n+j]);
		if (x > largest)
		    largest = x;
	    }
	    // if all zeros, it's a singular matrix
	    if (largest == 0)
		return false;
	    scaleFactors[i] = 1.0/largest;
	}
	
        // use Crout's method; loop through the columns
	for (j = 0; j != n; j++) {
	    
	    // calculate upper triangular elements for this column
	    for (i = 0; i != j; i++) {
		double q = a[i*n+j];
		for (k = 0; k != i; k++)
		    q -= a[i*n+k]*a[k*n+j];
		a[i*n+j] = q;
	    }

	    // calculate lower triangular elements for this column
	    double largest = 0;
	    int largestRow = -1;
	    for (i = j; i != n; i++) {
		double q = a[i*n+j];
		for (k = 0; k != j; k++)
		    q -= a[i*n+k]*a[k*n+j];
		a[i*n+j] = q;
		double x = abs(q);
		if (x >= largest) {
		    largest = x;
		    largestRow = i;
		}
	    }
	    
	    // pivoting
	    if (j != largestRow) {
		double x;
		for (k = 0; k != n; k++) {
		    x = a[largestRow*n+k];
		    a[largestRow*n+k] = a[j*n+k];
		    a[j*n+k] = x;
		}
		scaleFactors[largestRow] = scaleFactors[j];
	    }

	    // keep track of row interchanges
	    ipvt[j] = largestRow;

	    // avoid zeros
	    if (a[j*n+j] == 0.0) {
		//System.out.println("avoided zero");
		a[j*n+j]=1e-18;
	    }

	    if (j != n-1) {
		double mult = 1.0/a[j*n+j];
		for (i = j+1; i != n; i++)
		    a[i*n+j] *= mult;
	    }
	}
	return true;
}
	
const int u = 5;
# define stamp(x,y) test[x*u+y]
double test[25] ={}; 

void lu_solve(double *a, int n, int *ipvt, double *b);

double solved[u] = {};

void stampRightSide(int a, double x)
{
	solved[a] += x;
}

void stampMatrix(int a,int b, double x)
{
	stamp(a,b) += x;
}

void stamp_resistor(int a, int b, double r)
{
	double r0 = 1/r;
	
	stampMatrix(a,a,r0);
	stampMatrix(b,b,r0);
	stampMatrix(a,b,-r0);
	stampMatrix(b,a,-r0);
}

void stampVoltageSource(int n1, int n2, int vs, double v) {
	int vn = (4)+vs;
	stampMatrix(vn, n1, -1);
	stampMatrix(vn, n2, 1);
	stampRightSide(vn, v);
	stampMatrix(n1, vn, 1);
	stampMatrix(n2, vn, -1);
    }

void test_lu()
{
	//return;
	int pivots[u] = {};
	
	stamp_resistor(0, 1, 1000.);
	stamp_resistor(1, 2, 1000.);
	stamp_resistor(2, 3, 1000.);
	stampVoltageSource(0, 3, 0, 5.);
	
	for(int i =0; i<5000; i++)
		{
		if(lu_factor(test, u, pivots))
		lu_solve(test, u, pivots, solved);
		}

	
}

void lu_solve(double *a, int n, int *ipvt, double *b) {
	int i;
	// find first nonzero b element
	for (i = 0; i != n; i++) {
	    int row = ipvt[i];

	    double swap = b[row];
	    b[row] = b[i];
	    b[i] = swap;
	    if (swap != 0)
		break;
	}
	
	int bi = i++;
	for (; i < n; i++) {
	    int row = ipvt[i];
	    int j;
	    double tot = b[row];
	    
	    b[row] = b[i];
	    // forward substitution using the lower triangular matrix
	    for (j = bi; j < i; j++)
		tot -= a[i*n+j]*b[j];
	    b[i] = tot;
	}
	for (i = n-1; i >= 0; i--) {
	    double tot = b[i];
	    
	    // back-substitution using the upper triangular matrix
	    int j;
	    for (j = i+1; j != n; j++)
		tot -= a[i*n+j]*b[j];
	    b[i] = tot/a[i*n+i];
	}
}

