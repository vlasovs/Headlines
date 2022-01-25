//---------------------------------------------------------------------------

#ifndef MatrixH
#define MatrixH
//---------------------------------------------------------------------------

void CreateMatrix(double*& a, int n, int m);
void DestroyMatrix(double* a, int n, int m);
void Copy(double* a, double* b, int n, int m);
void Transpose(double* a, double* b, int n, int m);
double NormL2_(double* a, int n);
double NormC_(double* a, int n);
void Mul(double* a, double* b, double* c, int k, int m, int n);
void Mul_Threading(double* a, double* b, double* c, int k, int m, int n, int np);
void SumM(double* a, double* b, double* c, int n);
void Diagonal(double* a, int n);
bool WithGauss(double* M, double* A, int n);
bool GetLU(double* lu, int* p, int n, int m);
bool GetLU_Threading(double* lu, int* p, int n, int np);
void GetAnswer(double* lu, int* p, double* b, double* ans, int n);
void GetError(double* m, double* b, double* ans, double* err, int n);

#endif
