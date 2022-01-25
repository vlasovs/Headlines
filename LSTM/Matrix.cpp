//---------------------------------------------------------------------------

#pragma hdrstop

#include "Matrix.h"
#include "Math.h"
#include <thread>
#include <vector>

using namespace std;
//---------------------------------------------------------------------------
void CreateMatrix(double*& a, int n, int m) {
	a = new double[n*m];
}

void DestroyMatrix(double* a, int n, int m) {
	delete [] a;
}

void Copy(double* a, double* b, int n, int m){
	for (int i = 0; i < n; i++)
		for (int j = 0; j < m; j++)
			b[i*m+j] = a[i*m+j];
}

void Transpose(double* a, double* b, int n, int m){
	for (int i = 0; i < n; i++)
		for (int j = 0; j < m; j++)
			b[j*n+i] = a[i*m+j];
}

double NormL2_(double* a, int n){
	double sq;
	double sum = 0;
	for (int i = 0; i < n; i++) {
		sq = a[i] * a[i];
		sum += sq;
    }
	sum /= n;
	sum = sqrt(sum);
	return sum;
}

double NormC_(double* a, int n){

	double max = 0;
	for (int i = 0; i < n; i++) {
		if (max < fabs(a[i])) {
			max = fabs(a[i]);
		}
    }
	return max;

}

void Mul(double* a, double* b, double* c, int k, int m, int n){
	for (int i = 0; i < k; i++)	{
		for (int j = 0; j < m; j++){
			double sum = 0;
			for (int l = 0; l < n; l++){
				sum += a[i*n+l] * b[l*m+j];
			}
			c[i*m+j] = sum;
		}
	}
}

struct tparam{
	int id;
	int start;
	int end;
	double* a;
	double* b;
	double* c;
	int k;
	int m;
	int n;
};

void thrFunc(void * param){

	tparam* par=((tparam*)param);

	for (int i = par->start; i < par->end; i++)	{
		for (int j = 0; j < par->m; j++){
			double sum = 0;
			for (int l = 0; l < par->n; l++){
				sum += par->a[i*par->n+l] * par->b[l*par->m+j];
			}
			par->c[i*par->m+j] = sum;
		}
	}

	delete par;	
}

void Mul_Threading(double* a, double* b, double* c, int k, int m, int n, int np){

	vector<thread*> hands(np);
	for (int i = 0; i < np; i++) {
		tparam* par=new tparam();
		par->id=i;
		par->start=k*i/np;
		par->end=k*(i+1)/np;
		par->a=a;
		par->b=b;
		par->c=c;
		par->k=k;
		par->m=m;
		par->n=n;		
		hands[i] = new std::thread(thrFunc, par);
	}
	//WaitForMultipleObjects(np, hands, true, INFINITE);
	for (int i = 0; i < np; i++) {
		hands[i]->join();		
		delete hands[i];
	}
	//delete [] hands;
}

void SumM(double* a, double* b, double* c, int n){
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			c[i*n+j] = a[i*n+j] + b[i*n+j];
}

void Diagonal(double* a, int n){
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			if (i!=j) a[i*n+j] = 0;
}

bool GetLU(double* lu, int* p, int n, int m){
	int bi, r, c, i, w;
	double* temp;
	double el, e, a1, b1;
	double z; z = 0;
	for (i = 0; i < n; i++) p[i] = i;
	for (i = 0; i < n - 1; i++) {
		bi = i;
		for (r = i; r < n; r++)	{
			if (fabs(lu[p[r]*n+i]) > fabs(lu[p[bi]*n+i])) bi = r;
		}
		w = p[bi]; p[bi] = p[i]; p[i] = w;
		e = lu[p[i]*n+i];
		if (e == z)
			return false;
		for (r = i + 1; r < n; r++)	{
			if (lu[p[r]*n+i] == z) lu[p[r]*n+i] = z;
			else
			{
				el = lu[p[r]*n+i];
				el /= e;

				lu[p[r]*n+i] = el;
				for (c = i + 1; c < m; c++)	{
					if (lu[p[r]*n+c] == z)
						a1 = z;
					else
						a1 = lu[p[r]*n+c];
					if (lu[p[i]*n+c] == z)
						b1 = z;
					else
						b1 = lu[p[i]*n+c] * el;
					lu[p[r]*n+c] = a1 - b1;
				}
			}
		}
	}
	return true;
}

struct tparam2{
	int id;
	int start;
	int end;
	double* lu;
	int* p;
	int n;
	int np;
	int i;
	bool stop;
};

void thrFunc2(void * param){
	int bi, r, c, i, w;
	double* temp;
	double el, e, a1, b1;
	double z; z = 0;
	int count,start,end;
	tparam2* par=((tparam2*)param);
	for (i = 0; i < par->n - 1; i++) {
		while (par->stop);
		count=par->n-(i+1);
		start=(i+1)+(count*par->id)/par->np;
		end=(i+1)+(count*(par->id+1))/par->np;
		e = par->lu[par->p[i]*par->n+i];
		for (r = start; r < end; r++) {
			if (par->lu[par->p[r]*par->n+i] == z) par->lu[par->p[r]*par->n+i] = z;
			else
			{
				el = par->lu[par->p[r]*par->n+i];
				el /= e;
				par->lu[par->p[r]*par->n+i] = el;
				for (c = i + 1; c < par->n; c++)	{
					if (par->lu[par->p[r]*par->n+c] == z)
						a1 = z;
					else
						a1 = par->lu[par->p[r]*par->n+c];
					if (par->lu[par->p[i]*par->n+c] == z)
						b1 = z;
					else
						b1 = par->lu[par->p[i]*par->n+c] * el;
					par->lu[par->p[r]*par->n+c] = a1 - b1;
				}
			}
		}
		par->stop=true;
	} 
}

bool GetLU_Threading(double* lu, int* p, int n, int np){

	for (int i = 0; i < n; i++) p[i] = i;

	bool result=true;

	vector<thread*> hands(np);	
	tparam2** par=new tparam2*[np];
	for (int i = 0; i < np; i++) {
		par[i]=new tparam2();
		par[i]->id=i;
		par[i]->lu=lu;
		par[i]->p=p;
		par[i]->n=n;
		par[i]->np=np;
		par[i]->stop=true;
		hands[i] = new thread(thrFunc2, par[i]);
	}

	for (int k = 0; k < n - 1; k++) {

		int i = k;
		int bi = i;
		for (int r = i; r < n; r++)	{
			if (fabs(lu[p[r]*n+i]) > fabs(lu[p[bi]*n+i])) bi = r;
		}
		int w = p[bi]; p[bi] = p[i]; p[i] = w;
		//double* temp = lu[bi]; lu[bi] = lu[i]; lu[i] = temp;
		double e = lu[p[i]*n+i];
		/*
		if (e == 0) {
			result = false;
			break;
		}*/

		for (int i = 0; i < np; i++) {
			par[i]->stop=false;
		}

		for (int i = 0; i < np; i++) {
			while (!par[i]->stop);
		}

	}

	//WaitForMultipleObjects(np, hands, true, INFINITE);
	for (int i = 0; i < np; i++) {
		hands[i]->join();
		delete hands[i];
		/*
		bool next=false;
		while (!next){
			unsigned long r=WaitForSingleObject(hands[i],10);
			if (r==WAIT_OBJECT_0){
				next=true;
			}
		}*/
		//CloseHandle(hands[i]);
	}

	for (int i = 0; i < np; i++) {
		delete [] par[i];
	}
	delete [] par;
	//delete [] hands;

	return result;
}

void GetAnswer(double* lu, int* p, double* b, double* ans, int n){
	double* y = new double[n];
	double a1;
	double z; z = 0;
	for (int i = 0; i < n; i++){
		a1 = z;
		for (int j = 0; j < i; j++){
			a1 += (lu[p[i]*n+j]) * (y[j]);
		}
		if (a1 == z) a1 = z;
		if (b[p[i]] == z)
			y[i] = -a1;
		else
			y[i] = b[p[i]] - (double)a1;
	}
	for (int i = n - 1; i > -1; i--){
		a1 = z;
		for (int j = i + 1; j < n; j++)
			a1 += (lu[p[i]*n+j] * ans[j]);
		if (a1 == z) a1 = z;
		if (y[i] == z)
			ans[i] = -a1;
		else
			ans[i] = y[i] - a1;
		if (lu[p[i]*n+i] == z)
			ans[i] = 0;
		else
			ans[i] /= lu[p[i]*n+i];
	}
	delete [] y;
}

void GetError(double* m, double* b, double* ans, double* err, int n){
	double a1, a2, a;
	a = 0;
	for (int i = 0; i < n; i++){
		a1 = a;
		for (int j = 0; j < n; j++){
			a2 = m[i*n+j];
			a2 *= ans[j];
			a1 += a2;
		}
		err[i] = b[i];
		err[i] -= a1;
	}
	return;
}
