//---------------------------------------------------------------------------
#ifndef NeuronH
#define NeuronH
//---------------------------------------------------------------------------

#include<string>
#include<stack>
#include<vector>

using namespace std;
double SigmaFunc(double Sigma);
double TanhFunc(double Sigma);
string ReadWord(string& s);
void SwapDot(string& s);
void SwapDot(wstring& s);
void SwapComma(string& s);
void SwapComma(wstring& s);

class Layer {
	public:
		Layer(void);
		virtual ~Layer(void);
		virtual void SetCoeff(double val, int count);
		virtual double GetCoeff(int count);
		virtual void SetDelta(double val, int count);
		virtual double GetDelta(int count);

		void SetNeighbors(Layer* next, Layer* prev);
		int GetCoeffAmount(void);
		int GetEnterCount(void);
		int GetNeuronCount(void);
		int GetExitCount(void);
		void Fill(int flag);
		double* GetAnswer(void);
		double* GetDeltaSigma(void);
		double* GetDelta(void);
		virtual void FreeDelta(void);
		void SetDifference(const double* Delta);
		void AplyError(double h);
		void SetEnter(const double* enter,const int count);
		void SetEnter(const double* enter);
		string GetName(void);
		virtual void SetEnterCount(int EnterCount, int NeuronCount, int ExitCount)=0;
		virtual void Execute(double* Enter, int count)=0;
		virtual void CalcDeltaSigma(double* ds, int stage)=0;
		virtual void GetGradient(double* Gradient, int offset)=0;
		virtual void CalcDelta(double Alpha)=0;
		virtual void Clear(void)=0;
		virtual void ClearDeltaSigma(void)=0;
	protected:
		int EnterCount;
		int NeuronCount;
        int ExitCount;
		int Amount;
		double* w;
		double* DeltaW;
		double* DeltaSigma;
		double* Enter;
		double* Out;
		Layer* next;
		Layer* prev;
        string name;
};

class Dense: public Layer{
	public:
		Dense(void);
		virtual void SetEnterCount(int EnterCount, int NeuronCount, int ExitCount);
		virtual void Execute(double* Enter, int count);
		virtual void CalcDeltaSigma(double* ds, int stage);
		virtual void GetGradient(double* Gradient, int offset);
		virtual void CalcDelta(double Alpha);
		virtual void Clear(void);
		virtual void ClearDeltaSigma(void);
};

class LSTM: public Layer {
	public:
		LSTM(void);
		virtual ~LSTM(void);
		virtual void SetEnterCount(int EnterCount, int NeuronCount, int ExitCount);
		virtual void Execute(double* Enter, int count);
		virtual void CalcDeltaSigma(double* ds, int stage);
		virtual void GetGradient(double* Gradient, int offset);
		virtual void CalcDelta(double Alpha);
		virtual void Clear(void);
		void ClearDeltaSigma(void);
	private:
		int CoeffCount;
		vector<vector<double*> > Stack;
		vector<vector<double*> > Sigma;
		vector<vector<double*> > DeltaSigmas;
		vector<vector<double> >  Pipe;
		vector<double> Exit;
		vector<double> C;
		void NewStage(void);
};

class Embedding: public Layer{
	public:
		Embedding(void);
		virtual ~Embedding(void);
		virtual void SetEnterCount(int EnterCount, int NeuronCount, int ExitCount);
		virtual void Execute(double* Enter, int count);
		virtual void CalcDeltaSigma(double* ds, int stage);
		virtual void GetGradient(double* Gradient, int offset);
		virtual void CalcDelta(double Alpha);
		virtual void Clear(void);
		virtual void ClearDeltaSigma(void);
};

class NN: public Layer {
	public:
		NN(void);
		virtual ~NN(void);
		void Load(wstring FileName);
		virtual void SetCoeff(double val, int count);
		virtual double GetCoeff(int count);
		virtual void SetDelta(double val, int count);
		virtual double GetDelta(int count);
		void Save(string FileName);
		int GetCoeffAmount(void);
		void GetNumericalGradient(const vector<double>& tests, double* Gradient, int offset);
		void CalcNumericalDelta(const vector<double>& tests, double r, double Alpha);
		virtual void SetEnterCount(int EnterCount, int NeuronCount, int ExitCount);
		virtual void Execute(double* Enter, int count);
		virtual void CalcDeltaSigma(double* ds, int stage);
		virtual void GetGradient(double* Gradient, int offset);
		virtual void CalcDelta(double Alpha);
		virtual void Clear(void);
		virtual void FreeDelta(void);
		virtual void ClearDeltaSigma(void);
		void AdaMax(int n,double beta1,double beta2);
		void Adam(int n,double beta1,double beta2);
	private:
        double* g;
		double* v;
		double* m;
		vector<Layer*> layers;
		int FindIndex(int& count);
};
#endif
