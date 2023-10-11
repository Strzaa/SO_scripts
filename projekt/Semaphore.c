#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#include "Properties.h"
int sem[PROCESSES_COUNT + 1];  // id semaforów

union semun {  //unia semafora, tak jest w dokumentacji, wiec tak ma być
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
} ctl;

void semlock(int semIndex) {  //funkcja "opuszczająca" semafor
    int semid = sem[semIndex];

    struct sembuf opr;
    opr.sem_num = 0;
    opr.sem_op = -1;
    opr.sem_flg = 0;
    while (semop(semid, &opr, 1) == -1) {  // blokowanie do skutku
        ;
    }
}

void semunlock(int semIndex) {  //funkcja "podnosząca" semafor
    int semid = sem[semIndex];

    struct sembuf opr;
    opr.sem_num = 0;
    opr.sem_op = 1;
    opr.sem_flg = 0;
    while (semop(semid, &opr, 1) == -1) {
        if (errno != EINTR) {
            warn("Blad blokowania semafora!");
            break;
        }
    }
}

int createSem(int seed) {  //funkcja tworzy i zwraca semafor
    key_t key;
    int semid;
    if ((key = ftok(".", seed)) == -1) errx(1, "Blad tworzenia klucza!");
    if ((semid = semget(key, 1, IPC_CREAT | 0600)) == -1) errx(2, "Blad tworzenia semafora!");
    ctl.val = 0;  // ustawienie wartosci semafora
    if (semctl(semid, 0, SETVAL, ctl) == -1) errx(3, "Blad ustawiania semafora!");
    return semid;
}

void createSemaphores() {
    int processIndex;
    for (processIndex = 0; processIndex < PROCESSES_COUNT + 1; processIndex++) {
        sem[processIndex] = createSem(processIndex);
    }
}

void removeSemaphores() {
    int processIndex;
    for (processIndex = 0; processIndex < PROCESSES_COUNT + 1; processIndex++) {
        semctl(sem[processIndex], 0, IPC_RMID, ctl);  //usunięcie semaforów
    }
}