
#include <ilcplex/ilocplex.h>

ILOSTLBEGIN

#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <iostream>


int ILOSC_KLOCKOW = 6;

IloInt a = 0;//gap frame
IloInt b = 8, c = 8;//rozmiar palety

struct Klocek{
	Klocek(const IloEnv &env,unsigned int w, unsigned int h):
		w(w),o(env,0,1),h(h),pole(w*h),p(env,0,1), x1(env, a, b-a), x2(env, a, b-a),y1(env, a, c-a), y2(env, a, c-a)
	{

	}

	IloInt w;//szerokoœæ produktu
	IloInt h;//wysokoœæ produktu
	IloIntVar x1; //bli¿sza krawêdŸ na osi X
	IloIntVar x2; //dalsza krawêdŸ na osi X
	IloIntVar y1; //bli¿sza krawedŸ na osi Y
	IloIntVar y2; //dalsza krawêdŸ na osi Y
	IloNum pole; //pole a*b
	IloBoolVar o; //orientacja o {0,1} bez obrotu, obrót
	IloBoolVar p;// 0 nie wybrany, 1 wybrany
};


// wczytywanie danych z pliku tekstowego
bool wczytanieDanych(const IloEnv& env, const string nazwa_pliku, std::vector<Klocek> &ref)
{
	fstream fInputFile;
	fInputFile.open(nazwa_pliku);
	if(fInputFile.good())
	{
		std::string sLine;
		std::getline(fInputFile,sLine);
		std::stringstream stream(sLine);
		stream>>b>>c>>ILOSC_KLOCKOW;

		if(b<1||c<1||ILOSC_KLOCKOW<1)
		{
			cout<<"nieprawidlowy rozmiar palety albo ilosc klockow"<<endl;
			return false;
		}

		for(int i=0; i<ILOSC_KLOCKOW; i++)
		{
			int x=0,y=0;
			std::getline(fInputFile,sLine);
			std::stringstream stream(sLine);
			stream>>x>>y;
			if(x>b || x<1 || y>c || y<1)
			{
				cout<<"nieprawidlowy rozmiar klockow"<<endl;
				return false;
			}

			Klocek k(env,x,y);
			ref.push_back(k);
		}
		return true;
	}
	else
	{
		cout<<"nieprawidlowy plik!"<<endl;
		return false;
	}
}

// ograniczenia wspólne dla obu wariantów problemu
void dodajWspolneOgraniczenia(IloModel& modelRef, const Klocek & klocek)
{
	modelRef.add(klocek.x1 >= a);                       
	modelRef.add(klocek.x2 <= b-a);                     
	modelRef.add(klocek.y1 >= a);						
	modelRef.add(klocek.y2 <= c-a);						

	modelRef.add(klocek.x1 + klocek.w + klocek.h*klocek.o - klocek.w*klocek.o - klocek.x2 == 0);
	modelRef.add(klocek.y1 + klocek.h + klocek.w*klocek.o - klocek.h*klocek.o - klocek.y2 == 0);
}

// ograniczenia dla problemu zape³nienia palety
void dodajOgraniczeniaPierwszegoWariantu(IloModel& modelRef, const Klocek& klocek, const Klocek& kolejnyKlocek)
{
	modelRef.add(b-b*klocek.p + kolejnyKlocek.x1 - klocek.x2 -a >=0 ||
		b-b*klocek.p + klocek.x1 - kolejnyKlocek.x2 -a >=0 ||
		c-c*klocek.p + kolejnyKlocek.y1 - klocek.y2 -a >=0 ||
		c-c*klocek.p + klocek.y1 - kolejnyKlocek.y2 -a >=0);		
}

// ograniczenia dla problemu uk³adania do rogu palety
void dodajOgraniczeniaDrugiegoWariantu(IloModel& modelRef, const Klocek & klocek, const Klocek & kolejnyKlocek)
{
	modelRef.add( kolejnyKlocek.x1 - klocek.x2 -a >=0	
		|| klocek.x1 - kolejnyKlocek.x2 -a >=0
		|| kolejnyKlocek.y1 - klocek.y2 -a >=0
		|| klocek.y1 - kolejnyKlocek.y2 -a >=0);		
}

// wyœwietlenie wyników na ekran oraz zapis komend matlaba do pliku
void wyswietlenieWynikow(const IloCplex & cplex, const int Wariant, const std::vector<Klocek> & wszystkieKlocki)
{
	cout << "-----------------------------------------------------------"<<endl;
	cout << "status = "<< cplex.getStatus()<<endl;
	cout << "-----------------------------------------------------------"<<endl;
	if(1 == Wariant)
	{
		cout << "Max po optymalizacji i ograniczeniach =" << cplex.getObjValue() << endl;
	}
	else
	{
		cout <<"Minimum po optymalizacji i ograniczeniach ="<<cplex.getObjValue()<<endl;
	}
	cout << "------------------------------------------------------------------------"<<endl;

	fstream outputFile;
	outputFile.open("matlab.txt",'w');
	if(outputFile.good())
	{
		outputFile<<"plot(";
	}

	for(int i = 0; i<ILOSC_KLOCKOW;i++)
	{
		cout<< "klocek["<<i<<"]:"<<endl;
		if(1 == Wariant)
		{
			cout<< "p="<<cplex.getValue(wszystkieKlocki[i].p)<<endl;
		}

		if(outputFile.good())
		{

			//	'k-',[0 6 6 0 0],[0 0 6 6 0],'k-',[6 12 12 0 0],[0 0 6 6 0],'k-',[12 18 18 12 12],[0 0 6 6 0],'k-'
			outputFile<<"["<<cplex.getValue(wszystkieKlocki[i].x1)<<" "<<cplex.getValue(wszystkieKlocki[i].x2)<<" "<<cplex.getValue(wszystkieKlocki[i].x2)
				<<" "<<cplex.getValue(wszystkieKlocki[i].x1)<<" "<<cplex.getValue(wszystkieKlocki[i].x1)<<"],["<<cplex.getValue(wszystkieKlocki[i].y1)<<" "<<cplex.getValue(wszystkieKlocki[i].y1)
				<<" "<<cplex.getValue(wszystkieKlocki[i].y2)
				<<" "<<cplex.getValue(wszystkieKlocki[i].y2)<<" "<<cplex.getValue(wszystkieKlocki[i].y1)<<"],'k-',";
		}
		cout<< "\t[x1,y1]=["<<cplex.getValue(wszystkieKlocki[i].x1)<<","<<cplex.getValue(wszystkieKlocki[i].y1)<<"]"<<endl;
		cout<< "\t[x2,y2]=["<<cplex.getValue(wszystkieKlocki[i].x2)<<","<<cplex.getValue(wszystkieKlocki[i].y2)<<"]"<<endl;
	}

	cout << "-----------------------------------------------------------"<<endl;
}

// obrazowanie wyników w pliku tekstowym
void rysowanieWPliku(const IloCplex &cplex, int Wariant, const vector<Klocek> & wszystkieKlocki)
{
	std::vector<std::vector<char> > tablica;
	for(int i=0;i<b;i++)
	{
		tablica.emplace_back();
		for(int j=0;j<c;j++)
		{
			tablica.at(i).push_back('\0');
		}
	}
	char wyswietlanie='a';

	for(int i = 0; i<ILOSC_KLOCKOW; i++)
	{
		if(1 != Wariant || true == cplex.getValue(wszystkieKlocki[i].p))
		{
			for(int x=cplex.getValue(wszystkieKlocki[i].x1);x<cplex.getValue(wszystkieKlocki[i].x2);x++)
			{
				for(int y=cplex.getValue(wszystkieKlocki[i].y1);y<cplex.getValue(wszystkieKlocki[i].y2);y++)
				{
					tablica[x][y] =wyswietlanie;
				}
			}
		}
		wyswietlanie++;
	}

	fstream outputFile;
	outputFile.open("wynik.txt",'w');
	if(outputFile.good())
	{
		for(int y=c -1;y>=0;y--)
		{
			for(int x =0; x<b;x++)
			{
				outputFile<<tablica[x][y];
			}
			outputFile<<endl;
		}
	}
	else
	{
		cout<<"problem z zapisem wyniku do pliku"<<endl;
	}
}

// g³ówna funkcja programu
int main()
{
	IloEnv env;
	// wektor przechowuj¹cy wszystkie klocki
	std::vector<Klocek> wszystkieKlocki;
	if(true == wczytanieDanych(env, "dane.txt",wszystkieKlocki))
	{
		try
		{
			// model
			IloModel model(env);

			// stworzenie tablicy pól wszystkich klocków
			IloNumArray iPola(env);

			// tablica zmiennych decyzyjnych decyduj¹cych czy dany element zosta³ wybrany
			IloBoolVarArray iTabWybranych(env);

			// Obliczenie sumy wszystkich pól
			IloNum Sumapol=0;
			for(int i = 0; i<ILOSC_KLOCKOW; i++)
			{
				iPola.add(wszystkieKlocki[i].pole);
				iTabWybranych.add(wszystkieKlocki[i].p);
				Sumapol=Sumapol+wszystkieKlocki[i].pole;
			}

			// zmienne decyzyjne dla wariantu upchniêcia do rogu palety
			IloIntVar x_max(env, a, b-a);
			IloIntVar y_max(env, a, c-a);

			// Wybór wariantu problemu do rozwi¹zania
			int Wariant = 1;
			cout<<"Nr problemu do rozwiazania:"<<endl;
			cout<<"(1) Maksymalizacja pola powierzchni (domyslne)"<<endl;
			cout<<"(2) Ukladanie elementow do rogu palety. Funkcja celu x+y->min"<<endl;
			cout<<"(3) Ukladanie elementow do rogu palety. Funkcja celu x*y->min"<<endl;
			cout<<":";
			cin>>Wariant;

			//domyslna wartosc
			if (Wariant>3 || Wariant<1)
			{
				Wariant = 1;
			} 
			cout<<endl<<endl;


			// Dodanie ograniczeñ do modelu
			for(int i = 0; i<ILOSC_KLOCKOW; i++) //i to k 
			{
				dodajWspolneOgraniczenia(model, wszystkieKlocki[i]);

				if(1 == Wariant)
				{
					for(int j = i+1; j<ILOSC_KLOCKOW; j++) //j to f
					{
						dodajOgraniczeniaPierwszegoWariantu(model, wszystkieKlocki[i], wszystkieKlocki[j]);
					}
				}
				else
				{
					for(int j = i+1; j<ILOSC_KLOCKOW ; j++) //j to f
					{
						dodajOgraniczeniaDrugiegoWariantu(model, wszystkieKlocki[i], wszystkieKlocki[j]);
					}
					model.add(wszystkieKlocki[i].x2 <= x_max);
					model.add(wszystkieKlocki[i].y2 <= y_max);
				}
			}

			// Dodanie do modelu funkcji celu
			if(1 == Wariant)
			{
				model.add(IloMaximize(env ,IloScalProd(iPola,iTabWybranych)));
			}
			else if (2 == Wariant)
			{
				model.add(IloMinimize(env, x_max + y_max));
			}
			else
			{
				model.add(IloMinimize(env, x_max * y_max));
			}
		
			// uruchomienie solvera
			IloCplex cplex(model);
			cplex.setParam(IloCplex::TiLim, 60);//limit czasowy
			cplex.solve();

			// wyœwietlenie i zapis wyniku programu
			cout<<"Suma pol wszystkich klockow = "<<Sumapol<<", Pole palety = "<< c*b << endl;
			wyswietlenieWynikow(cplex, Wariant, wszystkieKlocki);
			rysowanieWPliku(cplex, Wariant, wszystkieKlocki);
		}
		catch (IloException& ex) {
			cerr << "Error: " << ex << endl;
		}
		catch (...) {
			cerr << "Error" << endl;
		}
	}

	env.end();
	system("Pause"); 
	return 0;
}