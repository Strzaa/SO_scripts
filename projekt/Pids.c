#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "Properties.h"

FILE *PIDs_file;            //wskaznik na plik z pidami
int pids[PROCESSES_COUNT];  // przechowuje PID'y procesów potomnych

int next() {  // zwraca numer porządkowy następnego procesu
    int pid = getpid();
    return (pid == pids[0]) + 2 * (int)(pid == pids[1]);
}
int previous() {  // zwraca numer porządkowy następnego procesu
    int pid = getpid();
    return (pid == pids[2]) + 2 * (int)(pid == pids[0]);
}
int getSavedPid(int processIndex) {
    return pids[processIndex];
}

void savePid(int processIndex, int pid) {
    pids[processIndex] = pid;
}

void exportPids() {  // zapisanie pidów do pliku PID_info.txt
    if (PIDs_file = fopen("PID_info.txt", "w")) {
        int i;
        for (i = 0; i < PROCESSES_COUNT; i++) fprintf(PIDs_file, "%d ", pids[i]);
        fclose(PIDs_file);
    } else {
        fprintf(stderr, "Plik przechowujacy PIDy nie zostal utworzony!\n");
    }
}

void getPids() {  // odczytanie pidów z pliku PID_info.txt
    if (PIDs_file = fopen("PID_info.txt", "r")) {
        int i;
        for (i = 0; i < PROCESSES_COUNT; i++) fscanf(PIDs_file, "%d", &pids[i]);
        fclose(PIDs_file);
    } else {
        fprintf(stderr, "Uwaga! Nie znaleziono pliku PID_info.txt");
    }
}

void removePidsFile() {
    system("rm PID_info.txt");
}