#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


// Bartosz Fiutka WCY20IX2S0

// Mutex jest mechanizmem wzajemnego wykluczania (MUTual EXclusion) sluzacym do chronienia danych
// dzielonych miedzy watkami przed rownoczesnymi modyfikacjami i uzywanym do implementacji
// sekcji krytycznych i monitorow.
//Mutex ma dwa mozliwe stany:
// otwarty (nie zajety przez zaden watek) lub zajety (przez jeden watek).

long double wynik = 0; //zdefiniowanie globalnie zmiennej końcowego wyniku
int threads = 10; //zdefiniowanie ilości wątków
float x = 0; //zmienna pobierana od urzytkownika

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //zdefiniowanie zmiennej mutex jako initializer

void licz_wartosci(int *nr_i) { //funkcja obliczająca sumę szeregu
    long double ulamek = 0, silnia=1, licznik = 0;

    int i;
    for (i=1; i <= *nr_i; i++) silnia *= i; //obliczenie mianownika (śilni)
    licznik = pow(x, *nr_i);  //obliczanie licznika
    ulamek = licznik / silnia; //wynik konkretnego wyrazu ciągu
    wynik += ulamek; //dodanie do ogólnej sumy

    pthread_mutex_unlock(&mutex); //zwalniamy wcześniej zajęty mutex
    printf("Numer wątku: %d Wyliczona wartość: %.5Lf\n", *nr_i, ulamek); //wypisanie nr wątku oraz jego wartość
}

int main(int tmp, char *arg_v[]) { //funkcja glówna
    int *space, i, p;
    space = (int*)calloc(threads, sizeof(int)); //alokujemy pamięć na każdy wątek

    pthread_t thr[10]; //tablica wątków
    for (i=0; i <= threads-1; i++) {
        void *space_void = &space[i]; //tworzymy zmienną typu void* potrzebną do funkcji pthread_creat
        pthread_create(&thr[i], NULL, (void*)licz_wartosci, space_void); //tworzenie wątku
        space[i] = i;
    }

	x = atof(arg_v[1]); //funkcja atof zmienia flout na double

	for (p=0; p <= threads-1; p++) pthread_detach(thr[p]); //zwalnianie/oddzielanie wątków
    sleep(1); // czekamy na wynik końcowy, żeby się pokazał po obliczeniu wszystkich wyrazów ciągu
    printf("\nWynik=%.5Lf exp(x)=%.5f\n", wynik, exp(x)); //wyświetlenie wyniku
	pthread_exit(NULL); //kończy wątek
}
