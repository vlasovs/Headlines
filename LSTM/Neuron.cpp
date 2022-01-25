// ---------------------------------------------------------------------------
#pragma hdrstop

#include "Neuron.h"

#include <io.h>
#include <iostream>
#include <sstream>
#include <fstream>
// ---------------------------------------------------------------------------

double SigmaFunc(double Sigma) {
	if (Sigma > logl(1.0E300)) {
		return 1;
	}
	if (Sigma < -logl(1.0E300)) {
		return 0;
	}
	double res = 1.0 / (1.0 + exp(-Sigma));
	return res;
}

double TanhFunc(double Sigma) {
	if (Sigma > logl(1.0E300)) {
		return 1;
	}
	if (Sigma < -logl(1.0E300)) {
		return -1;
	}
	double e1 = exp(Sigma);
	double e2 = exp(-Sigma);
	double res = (e1 - e2) / (e1 + e2);
	return res;
}

string ReadWord(string& s) {
	int k;
	k = 0;
	s += " ";
	while (k + 1 < (int)s.length() && !((s[k] == ' ') && (s[k + 1] != ' ')))
		k++;
	string result = s.substr(0, k);
	s = s.substr(k + 1, s.length() - k - 1);
	return result;
}

void SwapComma(string& s) {
	for (unsigned int i = 0; i < s.length(); i++) {
		if (s[i] == ',')
			s[i] = '.';
	}
}

void SwapDot(string& s) {
	for (unsigned int i = 0; i < s.length(); i++) {
		if (s[i] == '.')
			s[i] = ',';
	}
}

void SwapComma(wstring& s) {
	for (unsigned int i = 0; i < s.length(); i++) {
		if (s[i] == L',')
			s[i] = L'.';
	}
}

void SwapDot(wstring& s) {
	for (unsigned int i = 0; i < s.length(); i++) {
		if (s[i] == L'.')
			s[i] = L',';
	}
}

Layer::Layer(void) : EnterCount(0), Amount(0), next(0), prev(0) {
}

string Layer::GetName(void){
    return name;
}

void Layer::SetCoeff(double val, int i) {
	w[i] = val;
}

double Layer::GetCoeff(int i) {
	return w[i];
}

void Layer::SetDelta(double val, int count){
	DeltaW[count] = val;
}

double Layer::GetDelta(int count){
	return DeltaW[count];
}

Layer::~Layer(void) {
	if (EnterCount) {
		delete [] w;
		delete [] DeltaW;
		delete [] DeltaSigma;
		delete [] Out;
		delete [] Enter;
	}
}

void Layer::SetNeighbors(Layer* next, Layer* prev){
	this->next=next;
	this->prev=prev;
}

void Layer::SetEnter(const double* enter) {
	for (int k = 0; k < EnterCount; k++) {
		Enter[k] = enter[k];
	}
}

void Layer::SetEnter(const double* enter, const int count){
	EnterCount=count;
	delete [] Enter;
    Enter=new double[count];
	for (int k = 0; k < count; k++) {
		Enter[k] = enter[k];
	}
}

int Layer::GetCoeffAmount() {
	return Amount;
}

int Layer::GetEnterCount(void){
	return EnterCount;
}

int Layer::GetNeuronCount(void){
	return NeuronCount;
}

int Layer::GetExitCount(void){
	return ExitCount;
}

void Layer::Fill(int flag) {
	for (int k = 0; k < Amount; k++) {
		double f=0;
		if (flag < 0)
			f = (double)rand() / RAND_MAX * 2 - 1.0;
		else if (flag == 0)
			f = 0;
		else if (flag == 1)
			f = 1;
		else if (flag == 2)
			f = 0.5;
		SetCoeff(f, k);
	}
}

double* Layer::GetAnswer(void) {
	return Out;
}

double* Layer::GetDeltaSigma(void){
	return DeltaSigma;
}

double* Layer::GetDelta(){
	return DeltaW;
}

void Layer::FreeDelta(void) {
	if (DeltaW) {
		for (int k = 0; k < Amount; k++) {
			DeltaW[k] = 0;
		}
	}
	if (DeltaSigma) {
		for (int k = 0; k < NeuronCount; k++) {
			DeltaSigma[k] = 0;
		}
	}
}

void Layer::SetDifference(const double* Delta) {
	for (int k = 0; k < Amount; k++) {
		SetDelta(Delta[k],k);
	}
}

void Layer::AplyError(double h) {
	for (int k = 0; k < Amount; k++) {
		double kof=GetCoeff(k);
		double d=GetDelta(k);
		kof = kof + h *  d;
		SetCoeff(kof,k);
	}
}

Dense::Dense(void) : Layer() {
	name="Dense";
}

void Dense::SetEnterCount(int EnterCount, int NeuronCount, int ExitCount) {
	this->EnterCount = EnterCount;
	this->NeuronCount = NeuronCount;
	this->ExitCount = ExitCount;

	Amount = NeuronCount * (EnterCount + 1);
	int a = Amount;
	Enter = new double[EnterCount];
	w = new double[a];
	DeltaW = new double[a];
	Out = new double[ExitCount];
	DeltaSigma = new double[ExitCount];
}

void Dense::Execute(double* Enter, int count) {
	SetEnter(Enter);
	int count1 = 0;
	for (int i = 0; i < NeuronCount; i++) {
		double sum = 0;
		for (int e = 0; e < EnterCount; e++) {
			sum += this->Enter[e] * w[count1++];
		}
		sum += w[count1++];
		Out[i] = SigmaFunc(sum);
	}
	if (next) {
		next->Execute(Out,NeuronCount);
	}
}

void Dense::CalcDeltaSigma(double* ds, int stage) {
	for (int e = 0; e < NeuronCount; e++) {
		double sum = 0;
		double dfo = Out[e] * (1 - Out[e]);
		if (ds) {
			dfo *= ds[e];
		}
		sum = dfo;
		DeltaSigma[e] += sum;
	}
	if (prev) {
		double* ds2=new double[EnterCount];
		for (int k = 0; k < EnterCount; k++) {
			ds2[k]=0;
			for (int n = 0; n < NeuronCount; n++) {
				double fs=DeltaSigma[n];
				double s=fs*w[n*(EnterCount+1)+k];
				ds2[k]+=s;
			}
		}
		prev->CalcDeltaSigma(ds2, 0);
        delete [] ds2;
	}
}

void Dense::GetGradient(double* Gradient, int offset) {
	for (int i = 0; i < Amount; i++) {
		Gradient[offset + i] = 0;
	}
	for (int i = 0; i < NeuronCount; i++) {
		double dfo = DeltaSigma[i];
		for (int l = 0; l < EnterCount; l++) {
			Gradient[offset + i * (EnterCount + 1) + l] = Enter[l] * dfo;
		}
		Gradient[offset + i * (EnterCount + 1) + EnterCount] = dfo;
	}
}

void Dense::CalcDelta(double Alpha) {
	int count = 0;
	for (int i = 0; i < NeuronCount; i++) {
		double dfo = DeltaSigma[i];
		for (int l = 0; l < EnterCount; l++) {
			DeltaW[count] = Alpha*(DeltaW[count]) + Enter[l] * dfo;
			count++;
		}
		DeltaW[count] = Alpha*(DeltaW[count]) + dfo;
		count++;
	}
}

void Dense::Clear(){
	//Nothing
}

void Dense::ClearDeltaSigma(void){
	for (int i=0;i<NeuronCount;i++){
		DeltaSigma[i]=0;
	}
}

/********************************************************************/

LSTM::LSTM(void):Layer(),Exit(0),C(0){
	name="LSTM";
}

LSTM::~LSTM(void){
	if (EnterCount) {
		Clear();
	}
}

void LSTM::Clear(){
	for (int i = 0; i < NeuronCount; i++) {
		while (!Stack[i].empty()){
			delete [] Stack[i].back();
			delete [] Sigma[i].back();
			delete [] DeltaSigmas[i].back();
			Stack[i].pop_back();
			Sigma[i].pop_back();
			DeltaSigmas[i].pop_back();
			Pipe[i].pop_back();
		}
		Exit[i]=0;
		C[i]=0;
	}
}

void LSTM::SetEnterCount(int EnterC, int NeuronCount, int ExitCount){
	this->ExitCount=ExitCount;
	EnterCount = EnterC;
	CoeffCount = (EnterC + 2);
	this->NeuronCount = NeuronCount;
	Amount = 4 * CoeffCount * NeuronCount;
	int a=Amount;
	w = new double[a];
	DeltaW = new double[a];
	DeltaSigma = new double[NeuronCount];

	Sigma.resize(NeuronCount);
	DeltaSigmas.resize(NeuronCount);
	Stack.resize(NeuronCount);
	Pipe.resize(NeuronCount);
	Exit.resize(NeuronCount);
	C.resize(NeuronCount);
	Enter = new double[EnterCount];
	Out = new double[ExitCount];
}

void LSTM::NewStage(void){
	for (int i = 0; i < NeuronCount; i++) {
		Sigma[i].push_back(new double[4]);
		DeltaSigmas[i].push_back(new double[4]);
		Stack[i].push_back(new double[EnterCount+1]);
		Stack[i].back()[0]=Exit[i];
		Pipe[i].push_back(C[i]);
	}
	int c=Sigma[0].size();
	c=c;
}

void LSTM::Execute(double* Enter, int count){
	NewStage();
	int count1=0;
	for (int i = 0; i < NeuronCount; i++) {
		int cur=Stack[i].size()-1;
		//Stack[i][cur][0]=Exit[i];
		for (int k = 0; k < EnterCount; k++) {
			Stack[i][cur][k+1] = Enter[k];
		}
		int p = CoeffCount;
		for (int k = 0; k < 4; k++) {
			double sum = 0;
			for (int j = 0; j < EnterCount + 1; j++){
				double e=this->Stack[i][cur][j];
				double ww=w[count1++];
				sum+= ww * e;
			}
			sum+=w[count1++];
			Sigma[i][cur][k]=sum;
		}
		double* ss=Sigma[i][cur];
		double s;
		s=ss[0];
		double s1 = SigmaFunc(s);
		s=ss[1];
		double s2 = SigmaFunc(s);
		s=ss[2];
		double t1 = TanhFunc(s);
		s=ss[3];
		double s3 = SigmaFunc(s);
		double c1 = s1 * C[i] + s2 * t1;
		double t2 = TanhFunc(c1);
		double e1 = s3 * t2;
		Exit[i] = e1;
		C[i] = c1;
	}
	for (int i = 0; i < NeuronCount; i++) {
		Out[i]=Exit[i];
	}
	if (next) {
		next->Execute(Out,NeuronCount);
	}
}

void LSTM::CalcDeltaSigma(double* ds, int stage){
	for (int l = 0; l < NeuronCount; l++) {
		double c1=C[l];
		double de;
		if (ds) {
			de=ds[l];
		} else {
			de=1;
		}
		double dc=0;
		double prevf=0;
		for (int i = Stack[l].size()-1 ; i >= 0; i--) {
			double tc=TanhFunc(c1);
			double _tc=1-tc*tc;
			double* ss=Sigma[l][i];
			double* dss=DeltaSigmas[l][i];
			double f=SigmaFunc(ss[0]); //f
			double _f=f*(1-f);
			double q=SigmaFunc(ss[1]); //i
			double _q=q*(1-q);
			double g=TanhFunc(ss[2]);  //g
			double _g=1-g*g;
			double o=SigmaFunc(ss[3]); //o
			double _o=o*(1-o);
			dc=de*o*_tc+dc*prevf;
			c1=Pipe[l][i];
			double res1=dc*_f*c1;
			dss[0]+=res1;
			double res2=dc*_q*g;
			dss[1]+=res2;
			double res3=dc*_g*q;
			dss[2]+=res3;
			double res4=de*_o*tc;
			dss[3]+=res4;
			prevf=f;
			de=0;
			for (int k = 0; k < 4; k++) {
				de+=dss[k]*w[(4*l+k)*CoeffCount];
			}
		}
		//DeltaSigma[l]+=de;
	}
	if (prev) {
		double* ds2=new double[EnterCount];
        int c=Stack[0].size()-1;
		for (int i = c ; i >= 0; i--) {
			for (int e = 0; e < EnterCount; e++) {
				ds2[e]=0;
				for (int l = 0; l < NeuronCount; l++) {
					for (int k = 0; k < 4; k++) {
						double dss=DeltaSigmas[l][i][k];
						ds2[e]+=dss*w[(4*l+k)*CoeffCount+e+1];
					}
				}
			}
			prev->CalcDeltaSigma(ds2, i);
		}
		delete [] ds2;
	}
}

void LSTM::GetGradient(double* Gradient, int offset){
	for (int i = 0; i < Amount; i++){
		Gradient[offset+i] = 0;
	}
	for (int l = 0; l < NeuronCount; l++) {
		double ds1,e;
		for (unsigned int n = 0; n < Stack[l].size(); n++) {
			for (int k = 0; k < 4; k++) {
				ds1=DeltaSigmas[l][n][k];
				for (int i = 0; i < EnterCount+1; i++){
					e=Stack[l][n][i];
					double ed=e*ds1;
					Gradient[offset+(4*l+k)*CoeffCount+i] += ed;
				}
				Gradient[offset+(4*l+k+1)*CoeffCount-1] += ds1;
			}
		}
	}
}

void LSTM::CalcDelta(double Alpha){
	int index;
	double* Gradient=new double[Amount];
	for (int i = 0; i < Amount; i++) {
        Gradient[i]=0;
	}
	for (int l = 0; l < NeuronCount; l++) {
		double ds1,e;
		for (unsigned int n = 0; n < Stack[l].size(); n++) {
			for (int k = 0; k < 4; k++) {
				ds1=DeltaSigmas[l][n][k];
				for (int i = 0; i < EnterCount+1; i++){
					e=Stack[l][n][i];
					index=(4*l+k)*CoeffCount+i;
					Gradient[index] += e*ds1;
				}
				index=(4*l+k+1)*CoeffCount-1;
				Gradient[index] += ds1;
			}
		}
	}
	for (int k = 0; k < Amount; k++){
		DeltaW[k] = Alpha * DeltaW[k] + Gradient[k];
	}
	delete [] Gradient;
}

void LSTM::ClearDeltaSigma(void){
	for (int l = 0; l < NeuronCount; l++) {
		for (unsigned int i=0;i<Stack[l].size();i++){
			for (int k=0;k<4;k++){
				DeltaSigmas[l][i][k]=0;
			}
		}
	}
}

// Embedding

Embedding::Embedding(void):Layer(){
    name="Embedding";
}

Embedding::~Embedding(void){
//Nothing
}

void Embedding::SetEnterCount(int EnterC, int NeuronC, int ExitC){
	EnterCount=EnterC;
	NeuronCount=NeuronC;
	ExitCount=ExitC;
	Enter = new double[1];
	int a=Amount=NeuronCount*ExitCount;
	w = new double[a];
	DeltaW = new double[a];
	DeltaSigma = new double[a];
	Out = new double[ExitCount];
}

void Embedding::Execute(double* Enter, int count){
	SetEnter(Enter, count);
	for (int e = 0; e < EnterCount; e++) {
		for (int i = 0; i < ExitCount; i++) {
			Out[i]=w[(int)Enter[e]*ExitCount+i];
		}
		if (next) {
			next->Execute(Out,ExitCount);
		}
	}
}

void Embedding::CalcDeltaSigma(double* ds, int stage){
	for (int i = 0; i < ExitCount; i++) {
		DeltaSigma[(int)Enter[stage]*ExitCount+i] += ds[i];
	}
}

void Embedding::GetGradient(double* Gradient, int offset){
	for (int i = 0; i < Amount; i++) {
		double dfo = DeltaSigma[i];
		Gradient[offset + i] = dfo;
	}
}

void Embedding::CalcDelta(double Alpha){
	for (int i = 0; i < Amount; i++) {
		double dfo = DeltaSigma[i];
		DeltaW[i] = Alpha*(DeltaW[i]) + dfo;
	}
}

void Embedding::Clear(void){
    //Nothing
}

void Embedding::ClearDeltaSigma(void){
	for (int i=0;i<Amount;i++){
		DeltaSigma[i]=0;
	}
}

// NN - Neural_Net

NN::NN(void):Layer(){
	name="NN";
}

NN::~NN(void) {
	if (EnterCount) {
		int s=layers.size();
		for (int i = 0; i < s; i++) {
			delete layers[i];
		}
		delete [] g;
		delete [] v;
		delete [] m;
	}
}

int NN::FindIndex(int& count){
	int a=layers[0]->GetCoeffAmount();
	int i=0;
	while (count>=a) {
		count-=a;
		i++;
		a=layers[i]->GetCoeffAmount();
	}
	return i;
}

void NN::SetCoeff(double val, int count){
	int i=FindIndex(count);
	layers[i]->SetCoeff(val,count);
}

double NN::GetCoeff(int count){
	int i=FindIndex(count);
	return layers[i]->GetCoeff(count);
}

void NN::SetDelta(double val, int count){
	int i=FindIndex(count);
	layers[i]->SetDelta(val,count);
}

double NN::GetDelta(int count){
	int i=FindIndex(count);
	return layers[i]->GetDelta(count);
}

bool check_if_file_exists(std::wstring path)
{
	std::ifstream ff(path.c_str());
	return ff.is_open();
}

void NN::Load(wstring FileName){
	int ec1, nc1, xc1, first=true;
	if (check_if_file_exists(FileName.c_str())) {
        int s=layers.size();
		for (int i = 0; i < s; i++) {
			delete layers[i];
		}
        layers.clear();
		ifstream f;
		f.open(FileName.c_str());
		string text;
		while (getline(f,text)){
			Layer* l;
			if (text=="Embedding") {
				l=new Embedding;
			}else if (text=="LSTM") {
				l=new LSTM;
			} else if (text=="Dense") {
				l=new Dense;
			} else {
				break;
			}
			int ec, nc, xc;
			string text, s, word;
			getline(f,text);
			ec = atoi(text.c_str());
			getline(f,text);
			nc = atoi(text.c_str());
			getline(f,text);
			xc = atoi(text.c_str());
			l->SetEnterCount(ec,nc,xc);
			if (first) {
				ec1=ec;
				nc1=nc;
				first=false;
			}
			xc1=xc;
			getline(f,text);
			s=text;
			SwapComma(s);
			for (int r = 0; r < l->GetCoeffAmount(); r++){
				word = ReadWord(s);
				double we = atof(word.c_str());
				l->SetCoeff(we,r);
			}
			layers.push_back(l);
		}
	}
	SetEnterCount(ec1,nc1,xc1);
	unsigned int ls=layers.size();
	for (unsigned int i = 0; i < ls; i++) {
		Layer* next=0;
		Layer* prev=0;
		if (i>0) {
			prev=layers[i-1];
		}
		if (i<layers.size()-1) {
			next=layers[i+1];
		}
		layers[i]->SetNeighbors(next,prev);
	}
}

void NN::Save(string FileName) {
	std::stringstream ss;
	ss.precision(16);
	for (unsigned int i = 0; i < layers.size(); i++) {
		ss<<layers[i]->GetName()<<endl;
		ss<<layers[i]->GetEnterCount()<<endl;
		ss<<layers[i]->GetNeuronCount()<<endl;
		ss<<layers[i]->GetExitCount()<<endl;
		int Amount=layers[i]->GetCoeffAmount();
		for (int k = 0; k < Amount; k++){
			ss<<layers[i]->GetCoeff(k)<<" ";
		}
		ss<<endl;
	}
	std::string s = ss.str();
	SwapDot(s);
	ofstream f;
	f.open(FileName.c_str());
	f<<s;
	f.close();
}

int NN::GetCoeffAmount(void){
	return Amount;
}

void NN::Clear(void){
	for (unsigned int i = 0; i < layers.size(); i++) {
		layers[i]->Clear();
	}
}

void NN::SetEnterCount(int EnterCount, int NeuronCount, int ExitCount){
	this->ExitCount=ExitCount;
	this->EnterCount=EnterCount;
	this->NeuronCount=NeuronCount;
	w=new double[1];
	DeltaW=0;
	DeltaSigma=new double[NeuronCount];
	Enter=new double[EnterCount];
	Out=new double[ExitCount];
	int a=0;
	for (unsigned int i = 0; i < layers.size(); i++) {
		a+=layers[i]->GetCoeffAmount();
	}
	Amount=a;

	g=new double[a];
	v=new double[a];
	m=new double[a];

}

void NN::Execute(double* Enter, int count){
	//SetEnter(Enter,count);
	layers[0]->Execute(Enter,count);
	int last=layers.size()-1;
	int s=layers[last]->GetNeuronCount();
	for (int i = 0; i < s; i++) {
		double ex=layers[last]->GetAnswer()[i];
		Out[i]=ex;
	}
}

void NN::CalcDeltaSigma(double* ds, int stage){
    ClearDeltaSigma();
	int s=layers.size()-1;
	layers[s]->CalcDeltaSigma(ds,0);
}

void NN::GetGradient(double* Gradient, int offset){
	int count=0;
	int s=layers.size();
	for (int i = 0; i < s; i++) {
		layers[i]->GetGradient(Gradient,offset);
		offset+=layers[i]->GetCoeffAmount();
	}
}

void NN::CalcDelta(double Alpha){
	int s=layers.size();
	for (int i = 0; i < s; i++) {
		layers[i]->CalcDelta(Alpha);
	}
}

void NN::FreeDelta(void){
	int s=layers.size();
	for (int i = 0; i < s; i++) {
		layers[i]->FreeDelta();
	}

	for (int i = 0; i < Amount; i++) {
		g[i]=0;
		v[i]=0;
		m[i]=0;
	}

}

void NN::GetNumericalGradient(const vector<double>& tests, double* Gradient, int offset){
	double delta=1.0E-6;
	double* enter=new double[200];
	double* Gr=new double[Amount];
	//int ec=EnterCount;
	int tc=tests.size()-1;
	GetGradient(Gr,0);
	for (int i = 0; i < Amount; i++) {
		double derivative;
		double ww=GetCoeff(i);
		SetCoeff(ww-delta,i);
		Clear();
		for (int k = 0; k < tc; k++) {
			enter[k]=tests[k];
		}
		Execute(enter,tc);
		double r1 = GetAnswer()[0];
		SetCoeff(ww+delta,i);
		Clear();
		for (int k = 0; k < tc; k++) {
			enter[k]=tests[k];
		}
		Execute(enter,tc);
		double r2 = GetAnswer()[0];
		derivative=(r2-r1)/(2*delta);
		SetCoeff(ww,i);
		Gradient[offset+i] = derivative;
	}
	delete[] enter;
	double sum=0;
	for (int i = 0; i < Amount; i++) {
		double d=Gradient[offset+i] - Gr[i];
		sum += fabs(d);
	}
	sum=sum;
	delete[] Gr;
}

void NN::CalcNumericalDelta(const vector<double>& tests, double r, double Alpha){
	double* Gradient=new double[Amount];
	GetNumericalGradient(tests,Gradient,0);
	for (int k = 0; k < Amount; k++) {
		double kof = GetDelta(k);
		kof = kof * Alpha + Gradient[k] * r;
        SetDelta(kof,k);
	}
}

void NN::ClearDeltaSigma(void){
	int s=layers.size();
	for (int i = 0; i < s; i++) {
		layers[i]->ClearDeltaSigma();
	}
}

void NN::AdaMax(int n,double beta1,double beta2){
	int s=layers.size();
	for (int i = 0; i < s; i++) {
		layers[i]->CalcDelta(0);
	}
	int fc=Amount;

	//double beta1=0.9l;
	//double beta2=0.999l;
	double epsilon=1E-8l;

	double gmax=0;
	for (int k=0;k<fc;k++) {
		g[k]=GetDelta(k);
		if (fabsl(g[k])>gmax) gmax=fabsl(g[k]);
	}

	// AdaMax
	for (int k=0;k<fc;k++) {
		m[k]=beta1*m[k]+(1-beta1)*g[k];
		if (beta2*v[k]>gmax)
			v[k]=beta2*v[k];
		else
			v[k]=gmax;
		double delta=(1.0/(1.0-pow(beta1,n+1)))*m[k]/v[k];
		SetDelta(delta,k);
	}
}

void NN::Adam(int n,double beta1,double beta2){
	int s=layers.size();
	for (int i = 0; i < s; i++) {
		layers[i]->CalcDelta(0);
	}
	int fc=Amount;

	//double beta1=0.9l;
	//double beta2=0.999l;

	double epsilon=1E-8l;


	double gmax=0;
	for (int k=0;k<fc;k++) {
		g[k]=GetDelta(k);
		if (fabsl(g[k])>gmax) gmax=fabsl(g[k]);
	}

	// Adam

	for (int k=0;k<fc;k++) {
		m[k]=beta1*m[k]+(1-beta1)*g[k];
		v[k]=beta2*v[k]+(1-beta2)*g[k]*g[k];
		double m_;
		double v_;
		m_=m[k]/(1-powl(beta1,n+1));
		v_=v[k]/(1-powl(beta2,n+1));
		double delta=m_/(sqrt(v_)+epsilon);
		SetDelta(delta,k);
	}
}

