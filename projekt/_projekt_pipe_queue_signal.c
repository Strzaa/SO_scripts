#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "Pids.c"
#include "Properties.h"
#include "Queue.c"
#include "Semaphore.c"
#include "SharedMemory.c"

int self;        // nr porządkowy obecnego procesu
int halted = 0;  // flaga wstrzymania działania

char charactersCount[INPUT_SIZE];  // zmienna przechuwująca ilość znaków
char data[INPUT_SIZE];  // bufor danych
int sum;                // zmienna przechuwująca sumę wyrażenia

char signalRecieved[NOTIFICATION_SIZE];
char signalToSend[NOTIFICATION_SIZE];

void cleanUp() {
    removePidsFile();         // usuniecie pliku z pidami
    removeSemaphores();       // usunięcie semaforów
    removeQueue();            // usunięcie fifo
    removeSharedMemory();     // usunięcie kolejki
    kill(getpid(), SIGKILL);  // samobójstwo procesu
}

void mainHandler(int signal) {  //obsluga sygnalow procesu macierzystego
    if (signal == SIGNAL_END) {
        fprintf(stderr, "\nTu proces macierzysty: %d, Koncze dzialanie\n", getpid());

        int processIndex;
        for (processIndex = 0; processIndex < PROCESSES_COUNT; processIndex++) {
            kill(getSavedPid(processIndex), SIGKILL);  // SIGKILL konczy pracę procesu
        }
        cleanUp();  // usunięcie używanych zasobów przez proces macierzysty
    }
}
void childHandler(int signal) {
    if (signal == SIGNAL_END) {
        kill(getppid(), signal);
        return;

    } else if ((signal == SIGNAL_RESUME && halted) || (signal == SIGNAL_STOP && !halted)) {
        strcpy(signalToSend, signal == SIGNAL_STOP ? NOTIFICATION_STOP : NOTIFICATION_RESUME);

        semlock(QUEUE_SEM_LOCK);     // zajęcie dostępu do kolejki
        writeToQueue(signalToSend);  // zapisanie sygnału do pamięci współdzielonej
        writeToQueue(signalToSend);  // zapisanie sygnału do pamięci współdzielonej
        semunlock(QUEUE_SEM_LOCK);   // odblokowanie dostępu do kolejki

        kill(getSavedPid(next()), SIGNAL_NOTIFY);      // powiadomienie następnego procesu o otrzymaniu sygnału
        kill(getSavedPid(previous()), SIGNAL_NOTIFY);  // powiadomienie poprzedniego procesu o otrzymaniu sygnału

        halted = signal == SIGNAL_STOP;  // ustawienie flagi wstrzymania
        if (signal == SIGNAL_STOP) fprintf(stderr, "Wstrzymuje prace!\n");
        if (signal == SIGNAL_RESUME) fprintf(stderr, "Wznawiam prace!\n");

    } else if (signal == SIGNAL_NOTIFY) {
        semlock(QUEUE_SEM_LOCK);                              // zajęcie dostępu do kolejki
        readFromQueue(signalRecieved);                        // odczytanie powiadomienia
        halted = !strcmp(signalRecieved, NOTIFICATION_STOP);  // ustawienie flagi wstrzymania (strcmp zwraca 0, jesli takie same)
        semunlock(QUEUE_SEM_LOCK);                            // odblokowanie dostępu do kolejki
    }
}

void readStdin(char *data) {
    fprintf(stderr, "\nPodaj dane:\n");
    fgets(data, INPUT_SIZE + 1, stdin);                         // wczytanie strumienia wejścia do bufora
    if (strlen(data) == 1) fgets(data, INPUT_SIZE + 1, stdin);  // strlen(data)==1 oznacza, że bufor został w całości zapełniony
    data[strcspn(data, "\n")] = '\0';                           // usunięcie entera (ukradzione ze stackoverflow)
}


void reading() {
    while (1) {
        semlock(self);
        readStdin(data);         // wywołanie funkcji odczytu
        while (halted) pause();  // wstrzymanie pracy

        if (DEBUG) fprintf(stderr, "[P1:%d] Dane: %s\n", getpid(), data);

        writeToSharedMemory(data);

        semunlock(next());
    }
}

void processing() {
    while (1) {
        semlock(self);
        while (halted) pause();
        readFromSharedMemory(data);
        sprintf(charactersCount, "%d", (int)strlen(data));  // strlen - liczba bajtów danych

        if (DEBUG) {
            fprintf(stderr, "[P2:%d] Dane: %s\n", getpid(), data); // wyświetlenie danych
        } 

        data[0] = '\0';  // wyczyszczenie bufora danych
        writeToSharedMemory(charactersCount);
        semunlock(next());
    }
}

void logging() {
    while (1) {
        semlock(self);
        while (halted) pause();
        readFromSharedMemory(charactersCount);

        if (DEBUG) {
            fprintf(stderr, "[P3:%d] Dane: %s\n", getpid(), charactersCount);
        }
        fprintf(stdout, "[P3:%d] Ile liczb: %s\n", getpid(), charactersCount);

        fflush(stdout);  // wyczyszczenie bufora strumienia wyjscia (może zle wyswietlac)
        semunlock(next());
    }
}

void createProcesses() {
    void (*processes[PROCESSES_COUNT])(void) = {reading, processing, logging};  // wskaźniki funckji procesów
    int processIndex;
    for (processIndex = 0; processIndex < PROCESSES_COUNT; processIndex++) {
        // tutaj tworzony jest proces potomny (funkcja fork())
        savePid(processIndex, fork());         // zapisanie pidu do tablicy pidów
        if (getSavedPid(processIndex) == 0) {  // pid == 0 oznacza, że jest to proces potomny
            // zapisanie numeru porządkowego
            self = processIndex;

            // wyswietlenie informacji o procesie potomnym
            fprintf(stderr, "Proces %d PID:%d PPID:%d\n", self + 1, getpid(), getppid());

            // obsługa wszystkich sygnałów w podprocesach
            int i;
            for (i = 1; i <= SIGNALS_COUNT; i++) signal(i, childHandler);

            // pobranie PID'ów pozostałych procesów z pliku
            semlock(self);      // wstrzymanie do czasu odblokowania
            getPids();          // odczytanie pidów z pliku
            semunlock(next());  // umożliwienie odczytu kolejnemu procesowi

            processes[self]();  //wywołanie odpowiedniej funkcji dla procesu
        }
    }
}

int main() {
    fprintf(stderr, "Proces macierzysty PID:%d PPID:%d\n", getpid(), getppid());

    createQueue();           // stworzenie kolejki komunikatów
    createSharedMemory(INPUT_SIZE);  // stworzenie pamięci współdzielonej
    createSemaphores();      // utworzenie semaforów
    semunlock(QUEUE_SEM_LOCK);

    int i;
    for (i = 1; i <= SIGNALS_COUNT; i++) signal(i, mainHandler);  // obsługa wszystkich sygnałów

    createProcesses();  // stworzenie procesów
    exportPids();       // wyeksportowanie pidów do pliku
    semunlock(0);       // odblokowanie pierwszego procesu

    while (1) pause();  // wsytrzymanie pracy procesu macierzystego
    return 0;
}