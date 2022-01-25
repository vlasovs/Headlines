// LSTM.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "Neuron.h"
#include <vector>
#include <map>
#include "Matrix.h"
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <algorithm>
#include <time.h>
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <locale>
#include <io.h>
#include <fcntl.h>

//---------------------------------------------------------------------------

using namespace std;

NN* nn;
vector<vector<double>> tests;
vector<vector<wstring>> unsup;
bool stop;
int NumberOfProc;
map<wstring, int> dict;
double max_;
double min_;
//---------------------------------------------------------------------------

const int max_len = 80;
const int dict_length = 100;

// Convert a wide Unicode string to an UTF8 string
std::string utf8_encode(const std::wstring& wstr, UINT cp)
{
	if (cp != -1) {
		if (wstr.empty()) return std::string();
		int size_needed = WideCharToMultiByte(cp, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string strTo(size_needed, 0);
		WideCharToMultiByte(cp, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
		return strTo;
	}
	else {
		return std::string();
	}
}

// Convert an UTF8 string to a wide Unicode String
std::wstring utf8_decode(const std::string& str, UINT cp, size_t str_size)
{
	if (str_size == 0) return std::wstring();
	int size_needed = MultiByteToWideChar(cp, 0, &str[0], (int)str_size, NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(cp, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

wstring ReadWord(wstring& s) {
	int k;
	k = 0;
	s += L" ";
	while (k + 1 < (int)s.length() && !((s[k] != L' ') && (s[k + 1] == L' ')))
		k++;
	wstring result = s.substr(0, k + 1);
	while (k + 1 < (int)s.length() && !((s[k] == L' ') && (s[k + 1] != L' ')))
		k++;
	s = s.substr(k + 1, s.length() - k - 1);
	/*
	wstring w;

	for (int i = 0; i < result.length(); i++) {
		if (iswalpha(result[i])) {
			w+=towlower(result[i]);
		}
	}
	result=w;
	*/
	return result;
}

int NCount(double* delta, int ts, bool IsChecked) {
	int count = 0;
	for (int i = 0; i < ts; i++) {
		if (IsChecked) {
			static const double eps = -log(0.5) / 3;
			count += fabs(delta[i]) < eps ? 1 : 0;
		}
		else {
			count += fabs(delta[i]) < 0.25 ? 1 : 0;
		}
	}
	return count;
}

vector<wstring> AddTest(wstring path, double rating) {
	vector<wstring> test;
	wifstream f;
	f.open(path);
	wstring line;
	int i = 0;
	while (getline(f, line)) {
		while (i < max_len && line != L"") {
			wstring word = ReadWord(line);
			test.push_back(word);
			i++;
		}
	}
	f.close();
	return test;
}

void shuffle(vector<int>& arr) {
	for (int i = arr.size() - 1; i >= 1; i--)
	{
		int j = rand() % (i + 1);
		int tmp = arr[j];
		arr[j] = arr[i];
		arr[i] = tmp;
	}
}

double Crossentropy(double ans, double f, double& diff) {
	const double eps = 1.0E-15;
	double f1 = f;
	double f2 = 1 - f;
	double ff = log(0.25) * sqrt(ans * (1 - ans));
	if (f1 > eps) {
		f1 = log(f1);
	}
	else {
		f1 = log(eps);
	}
	ff += -ans * f1;
	if (f2 > eps) {
		f2 = log(f2);
	}
	else {
		f2 = log(eps);
	}
	ff += -(1 - ans) * f2;
	diff = -ans / f + (1 - ans) / (1 - f);
	return ff;
}

//---------------------------------------------------------------------------
void __fastcall Reload(void)
{
	if (nn) delete nn;
	nn = new NN();
	nn->Load(L"NN.txt");
}

void LoadTests(void)
{

	dict.clear();
	tests.clear();

	ifstream f;
	f.open(L"Dists.txt");
	f.seekg(0, std::ios_base::end);
	long long len = f.tellg();
	char* c = new char[len + 10];
	for (int i = 0; i < len + 10; i++) {
		c[i] = 0;
	}
	f.seekg(0, std::ios_base::beg);
	f.read(c, len);
	f.close();
	c[len] = 0;
	string s = c;
	delete[] c;
	wstring w = utf8_decode(s, CP_UTF8, len);
	wstringstream ss;
	ss << w;
	vector<double> d;
	int count = 0;
	while (getline(ss, w)) {
		//SwapComma(w);
		wstring w1 = ReadWord(w);
		count++;		
		if (w1 == L"") {
			continue;
		}
		double dist;		
		swscanf_s(w1.c_str(), L"%lf", &dist);

		d.push_back(dist);
		vector<wstring> v1;
		while (w != L"") {
			w1 = ReadWord(w);
			v1.push_back(w1);
		}
		unsup.push_back(v1);
	}	

	max_ = 0;
	min_ = 1.0E300;

	for (unsigned int i = 0; i < d.size(); i++) {
		if (max_ < d[i])
			max_ = d[i];
		if (min_ > d[i])
			min_ = d[i];
	}

	for (unsigned int i = 0; i < unsup.size(); i++) {
		for (unsigned int j = 0; j < unsup[i].size(); j++) {
			wstring word = unsup[i][j];
			dict[word]++;
		}
	}
	wcout << dict.size() << endl;
	vector<pair<int, wstring>> tmp;
	map<wstring, int>::iterator it;
	for (it = dict.begin(); it != dict.end(); ++it) {
		int c = it->second;
		wstring word = it->first;
		tmp.push_back(make_pair(c, word));
	}
	sort(tmp.begin(), tmp.end(), greater<pair<int, wstring>>());
	dict.clear();
	int dl = dict_length > tmp.size() ? tmp.size() : dict_length;
	for (int i = 0; i < dl - 1; i++) {
		wstring word = tmp[i].second;
		dict[word] = i + 1;
		wcout << word.c_str() << L" " << tmp[i].first << L"\n";
	}
	vector<double> records;
	for (unsigned int i = 0; i < unsup.size(); i++) {
		records.clear();
		for (unsigned int j = 0; j < unsup[i].size(); j++) {
			wstring word = unsup[i][j];
			if (dict.count(word) == 0) {
				records.push_back(0);
			}
			else {
				records.push_back(dict[word]);
			}
		}
		records.push_back((d[i] - min_) / (max_ - min_));
		tests.push_back(records);
	}

	for (unsigned int i = 0; i < tests.size(); i++) {
		string s;
		for (unsigned int j = 0; j < tests[i].size(); j++) {
			wcout << tests[i][j] << L" ";
		}
		wcout << L"\n";
	}

}

//---------------------------------------------------------------------------

void ConjugateGradient(int tag, bool IsChecked, int tcount = 1000, double Step = 0.0001, double Beta1 = 0.9, double Beta2 = 0.999)
{
	if (!stop) return;
	long long time = GetTickCount64();
	stop = false;	
	double Alpha = 1.0 / 3.0;	
	int ts = tests.size();
	double* delta = new double[ts];
	double* ans = new double[1];
	double* enter = new double[max_len + 10];
	for (int t = 0; t < tcount; t++) {
		nn->FreeDelta();
		vector<int> x(ts);
		for (int i = 0; i < ts; i++) {
			x[i] = i;
		}
		shuffle(x);
		for (int i = 0; i < ts; i++) {
			nn->Clear();
			int index = x[i];
			int size1 = tests[index].size() - 1;
			for (int j = 0; j < size1; j++) {
				enter[j] = tests[index][j];
			}
			nn->Execute(enter, size1);
			ans[0] = tests[index][size1];
			double a = nn->GetAnswer()[0];
			double f, f1, d1, f2, d2;
			f2 = ans[0] - a;
			d2 = f2;
			f1 = Crossentropy(ans[0], a, d1);
			if (IsChecked) {
				f = f1;
				delta[i] = d1;
			}
			else {
				f = f2;
				delta[i] = d2;
			}
			nn->CalcDeltaSigma(delta + i, 0);
			delta[i] = f;
			if (tag == 0) nn->CalcDelta(Alpha);
			else if (tag == 1) nn->AdaMax(i, Beta1, Beta2);
			else if (tag == 2) nn->Adam(i, Beta1, Beta2);
			//nn->CalcDeltaSigma(0);
			//nn->CalcNumericalDelta(tests[i],delta[i],Alpha);
			nn->AplyError(Step);

			double norm = 0;
			if (IsChecked) {
				for (int k = 0; k <= i; k++) {
					norm += delta[k];
				}
				norm /= i + 1;
			}
			else {
				norm = NormL2_(delta, i + 1);
			}

			if (stop)
				break;
		}

		//if (t%1==0) {
		double norm = 0;
		if (IsChecked) {
			for (int i = 0; i < ts; i++) {
				norm += delta[i];
			}
			norm /= ts;
		}
		else {
			norm = NormL2_(delta, ts);
		}
		long long tt = GetTickCount64() - time;
		wcout << norm << L" " << NormC_(delta, ts) << L" " << (t + 1) << L" " << tt << L" ms\n";

		if (stop)
			break;
	}
	delete[] delta;
	delete[] enter;
	delete[] ans;
	stop = true;
}
//---------------------------------------------------------------------------

void Fill(void)
{
	nn->Fill(-1);
}

//---------------------------------------------------------------------------

void Stop(void)
{
	stop = true;
}

//---------------------------------------------------------------------------

void Save(void)
{
	nn->Save("NN.txt");
}

//---------------------------------------------------------------------------

void Autoregression(int Iteration = 10)
{
	long long t = GetTickCount64();
	stop = false;
	int k = nn->GetCoeffAmount();
	double Lambda = 10.0;
	//double Step = StrToFloat(Edit2->Text);
	int size;
	//int Iteration = StrToInt(Edit1->Text);	
	double* Jacobi;
	CreateMatrix(Jacobi, tests.size(), k);
	double* JacobiT;
	CreateMatrix(JacobiT, k, tests.size());
	double* ans;
	CreateMatrix(ans, tests.size(), 1);
	double* enter = new double[max_len + 10];
	double* err = new double[k];
	double* M;
	CreateMatrix(M, k, k);
	double* M2;
	CreateMatrix(M2, k, k);
	double* D1;
	CreateMatrix(D1, k, 1);
	double* B;
	CreateMatrix(B, 1, k);
	int* p = new int[k];
	double* Delta = new double[k];
	double* vect1;
	CreateMatrix(vect1, 1, tests.size());
	for (int t = 0; t < Iteration; t++) {
		for (unsigned int i = 0; i < tests.size(); i++) {
			nn->Clear();
			size = tests[i].size() - 1;			
			for (int j = 0; j < size; j++) {
				enter[j] = tests[i][j];
			}
			nn->Execute(enter, size);
			double r = nn->GetAnswer()[0];
			ans[i] = tests[i][size] - r;
			nn->CalcDeltaSigma(0, 0);
			nn->GetGradient(Jacobi, i * k);
			//nn->GetNumericalGradient(tests[i],Jacobi,i*k);
		}
		Transpose(Jacobi, JacobiT, tests.size(), k);
		//Mul(JacobiT, Jacobi, M1, k, k, tests.size());
		//NumberOfProc
		Mul_Threading(JacobiT, Jacobi, M, k, k, tests.size(), NumberOfProc);
		for (int i = 0; i < k; i++)
		{
			M[i * k + i] *= (1 + Lambda);
		}
		Mul(JacobiT, ans, D1, k, 1, tests.size());
		Transpose(D1, B, k, 1);
		//GetLU(M, p, k, k);
		Copy(M, M2, k, k);
		GetLU_Threading(M, p, k, NumberOfProc);		
		GetAnswer(M, p, B, Delta, k);
		GetError(M2, B, Delta, err, k);
		nn->SetDifference(Delta);
		double nl2 = NormL2_(Delta, k);
		double nc = NormC_(Delta, k);
		double step = 1 / nc;
		if (step > 1) step = 1;
		//nn->AplyError(0.01);
		nn->AplyError(step);
		Transpose(ans, vect1, tests.size(), 1);
		wcout << NormL2_(vect1, tests.size()) << L" " << NormC_(vect1, tests.size()) << L" " << nl2 << L" " << nc << L" " << NormC_(err, k) << L" " << t + 1 << L"\n";
		//Application->ProcessMessages();
		if (stop) break;
	}
	stop = true;
	DestroyMatrix(Jacobi, tests.size(), k);
	DestroyMatrix(JacobiT, k, tests.size());
	DestroyMatrix(ans, tests.size(), 1);
	DestroyMatrix(M2, k, k);
	delete[] enter;
	//DestroyMatrix(M1, k, k);
	DestroyMatrix(M, k, k);
	DestroyMatrix(D1, k, 1);
	DestroyMatrix(B, 1, k);
	delete[] p;
	delete[] Delta;
	delete[] err;
	DestroyMatrix(vect1, 1, tests.size());
	t = GetTickCount64() - t;
	wcout << t << L" ms\n";
}
//---------------------------------------------------------------------------

void Test(void)
{
	int ts = tests.size();
	double* ans = new double[1];
	double* enter = new double[max_len + 10];
	vector<double> v1;
	for (int i = 0; i < ts; i++) {
		nn->Clear();
		int size1 = tests[i].size() - 1;
		for (int j = 0; j < size1; j++) {
			enter[j] = tests[i][j];
		}
		nn->Execute(enter, size1);
		ans[0] = tests[i][size1];
		double a = min_ + nn->GetAnswer()[0] * (max_ - min_);
		double f = min_ + ans[0] * (max_ - min_);
		//Memo1->Lines->Add(s + FloatToStr(a) + L" \t" + FloatToStr(f)+ L" \t" + FloatToStr(f-a));
		v1.push_back(a);
	}
	int count = v1.size();
	vector<int> order(ts);
	for (int i = 0; i < ts; i++) {
		order[i] = i;
	}
	sort(order.begin(), order.end(),
		[&v1](const int& i1, const int& i2) {
			return v1[i1] < v1[i2];
		});
	for (int i = 0; i < ts; i++) {
		int index = order[i];
		int size1 = tests[index].size() - 1;
		double f = min_ + tests[index][size1] * (max_ - min_);
		double a = v1[index];
		wstring s = L"";		
		for (unsigned int j = 0; j < unsup[index].size(); j++) {
			s += unsup[index][j].c_str();
			s += L" ";
		}
		wstring b = std::to_wstring(a);
		while (b.length() < 20) b += L"0";		
		wcout << b << L"   " << s << L"\n";
		
	}
	delete[] enter;
	delete[] ans;
	stop = true;
}
//---------------------------------------------------------------------------

int main()
{
	_setmode(_fileno(stdout), _O_U16TEXT);
	//std::setlocale(LC_ALL, "Russian.UTF16");
	//wcout.imbue(locale("rus_rus.866"));
	//wcin.imbue(locale("rus_rus.866"));
	const int look_back = 3;

	stop = true;
	_SYSTEM_INFO si;
	GetSystemInfo(&si);
	NumberOfProc = si.dwNumberOfProcessors;
	//wcout.precision(20);

	nn = new NN();
	nn->Load(L"NN.txt");
	LoadTests();
	
	wcout << L"\n";
	Test();
	//ConjugateGradient(1, true,10);
	Autoregression(10);
	Save();
	Test();

	if (nn) delete nn;

	std::wcout << L"Good Bye!\n";
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

//---------------------------------------------------------------------------
