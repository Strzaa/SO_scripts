#include <sys/shm.h>

#include "Properties.h"

char *sharedMemory;
int shmid;

void createSharedMemory(int size) {
    key_t key;

    if ((key = ftok(".", 'c')) == -1)  // stworzenie klucza IPC
        errx(1, "Blad tworzenia klucza!");
    if ((shmid = shmget(key, sizeof(char) * size, IPC_CREAT | 0666)) < 0)  // alokacja segmentu pamięci współdzielonej
        errx(2, "Blad tworzenia segmentu pamieci dzielonej!");
    if ((sharedMemory = shmat(shmid, NULL, 0)) == (char *)-1)  // przyłączenie pamięci współdzielonej
        errx(3, "Blad przylaczania pamieci dzielonej!");

    if (DEBUG) {
        fprintf(stderr, "\nShared memory id: %d\n", shmid);
    }
}

void readFromSharedMemory(char *data) {
    strcpy(data, sharedMemory);  // skopiowanie zawartości pamięci współdzielonej do bufora
}

void writeToSharedMemory(char *data) {
    strcpy(sharedMemory, data);  // skopiowanie zawartości bufora do pamięci współdzielonej
    data[0] = '\0';              // usunięcie zawartości bufora
}

void removeSharedMemory() {
    shmdt(sharedMemory);            // odłączenie pamięci współdzielonej
    shmctl(shmid, IPC_RMID, NULL);  // oznaczenie segmentu do zniszczenia
}