#include "ext.h"
#include "ext_obex.h"
#include "math.h"
#include <time.h>
#include <stdlib.h>

/**
	@file
	randist.c */

// randist.mxo MaxMsp External 
// M. Roddy 2013
// Compiled using max5 SDK



typedef struct {
	t_object b_ob;
	long b_num;			//Data Struct
	long b_id;
	double randVar;
	int distType;
	int possNum;
	float varFlt1;
	float varFlt2;
	
	void *rand_out;		//outlet and inlet pointers
	void *inDist;
	void *inPoss;
	void *inVar1;
	void *inVar2;
	
} t_randist;

//Function Declarations
void randist_bang(t_randist *x);
double equalDist(t_randist *x, long numPoss);
double highPass(long numPoss);
double lowPass(long numPoss);
double triangle(long numPoss);
double gaussian(long numPoss, float sigma, float mu);
double exponential(long numPoss, float lambda);
double cauchy(long numPoss);
double weibull(long numPoss);

//Outlets and Inlets
void randist_in1(t_randist *x, long n);
void randist_in2(t_randist *x, long n);
void randist_ft1(t_randist *x, double n);
void randist_ft2(t_randist *x, double n);



void randist_assist(t_randist *x, void *b, long m, long a, char *s);
void randist_free(t_randist *x);
void *randist_new(long num);

void *randist_class;
t_symbol *ps_list;

int main()
{
	static t_class *c;

	c = class_new("randist", (method)randist_new, (method)randist_free, sizeof(t_randist), 0L, A_DEFLONG, 0);

	class_addmethod(c, (method)randist_bang,		"bang",		0);
	class_addmethod(c, (method)randist_ft1, "ft2", A_FLOAT, 0);
	class_addmethod(c, (method)randist_ft2, "ft1", A_FLOAT, 0);
	class_addmethod(c, (method)randist_in1, "in3", A_LONG, 0);
	class_addmethod(c, (method)randist_in2, "in4", A_LONG, 0);



	class_addmethod(c, (method)randist_assist,	"assist",	A_CANT, 0);
 
	class_register(CLASS_BOX, c); 
	randist_class = c;
	
	ps_list = gensym("list");	

	return 0;
}

void randist_bang(t_randist *x)
{

	long possNum = x->possNum;
	float var1 = x->varFlt1;
	float var2 = x->varFlt2;
//	post("var1 = lf1 /n var2 = %f2", var1,var2);

	
	//Select which function to execute
	switch (x->distType) {
		case 0:
			x->randVar = equalDist(x,possNum);
			break;
		case 1:	
			x->randVar = highPass(possNum);
			break;
		case 2:
			x->randVar = lowPass(possNum);
			break;
		case 3:
			x->randVar = triangle(possNum);
			break;
		case 4:
			x->randVar = gaussian(possNum, var1, var2);
			break;
		case 5:
			x->randVar = exponential(possNum, var2); 
			break; 
		default:
			post("error with distType");
			break;
	}
	outlet_float(x->rand_out, x->randVar);
}	

double equalDist(t_randist *x, long numPoss)
{
	double r = rand(); //
	double l = RAND_MAX / numPoss;
	double y = ceil(r/l); // rounds up to next integer
	
	return y/numPoss;
}

double highPass(long numPoss)
{
	double output;
	double r1 = rand();
	double r2 = rand();

	if (r1>=r2) 
	output = r1;
	else output = r2;
	double l = RAND_MAX / numPoss;
	output = ceil(output/l); // rounds up to next integer
	
	return output/numPoss;
}

double lowPass(long numPoss)
{
	double output;
	double r1 = rand();
	double r2 = rand();
	
	if (r1<=r2) 
		output = r1;
	else output = r2;
	double l = RAND_MAX / numPoss;
	output = ceil(output/l); // rounds up to next integer
	
	return output/numPoss;
}

double triangle(long numPoss)
{
	double output;
	double r1 = rand();
	double r2 = rand();
	double l = RAND_MAX / numPoss;
	output = 0.5*(r1+r2);
	output = ceil(output/l); // rounds up to next integer
	return output/numPoss;
			  

	
}


double gaussian(long numPoss, float sigma, float mu) // sigma is standard deviation, mu is mean
{
	int k;
	int const N = 12;
	float const halfN = 6;
	float const scale = 1;
	double sum = 0;

	for(k = 1; k <= N; k++)
	{
		double r = rand();
		double l = RAND_MAX / numPoss;
		double y = ceil(r/l); // rounds up to next integer
		r = y/numPoss;
		sum += r;
	}
	return sigma * scale * (sum - halfN) + mu;	
}

double exponential(long numPoss, float lambda)
{
	double r = rand();
	double l = RAND_MAX / numPoss;
	double y = ceil(r/l); // rounds up to next integer
	r = y/numPoss;
	return (-log(r))/lambda;
}	
	


//Inlets and Outlets

void randist_in1(t_randist *x, long n)
{
	x->distType = n;
}

void randist_in2(t_randist *x, long n)
{
	if (n<1) n = 1;
	if (n>RAND_MAX) n = RAND_MAX;
	x->possNum = n;
}

void randist_ft1(t_randist *x, double n)
{
	x->varFlt1 = n;	
	
}

void randist_ft2(t_randist *x, double n)
{
	x->varFlt2 = n;	
	
}





//

void randist_assist(t_randist *x, void *b, long m, long a, char *s)
{
	if (m == ASSIST_INLET)
	{
		switch(a){
			case 0: sprintf(s,"Outputs random number when bang is received.");
				break;
			case 1: sprintf(s,"Number of output possibilites, specified in int.");
				break;
			case 2: sprintf(s,"Distribution selection.Distributions: \n0. Equal distribution. \n1. Highpass distribution. \n2. Lowpass distribution. \n3. Triangle distribution. \n4. Gaussian distribution. \n5. Exponential distribution");
				break;
			case 3: sprintf(s,"Floating point variable: Center/Mean for Gaussian and Horizontal spread (Lambda) for Exponential.");
				break;
			case 4: sprintf(s,"Floating point variable: Standard deviation for Gaussian.");
				break;
			default:
				post("error with randist_assist");
				break;				
		}
	}

			else 
		sprintf(s,"Output");
}


// unneeded memory free function.

void randist_free(t_randist *x)
{
//	short i;
	
	//for (i=1; i < x->b_num; i++)
//		object_free(x->b_mem[i].m_proxy);
//	sysmem_freeptr(x->b_mem);
}

void *randist_new(long num)
{
	t_randist *x;


	x = (t_randist *)object_alloc(randist_class);
	if (num < 2)
		num = 2;
	x->b_num = num;
	x->b_id = 0;
	x->distType = 0;
	x->possNum = 2;
	x->varFlt1 = 0.07; // default standard deviation for gaussian
	x->varFlt2 = 0.5; //default mean for gaussian
	x->rand_out = floatout((t_object *)x);
	x->inVar2 = floatin(x, 2);
	x->inVar1 = floatin(x, 1);
	x->inDist = intin(x, 3);
	x->inPoss = intin(x, 4);



	return x;
}

