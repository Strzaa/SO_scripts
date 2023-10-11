#include <stdio.h>
#include <unistd.h>

#include "Properties.h"

#define READ 0
#define WRITE 1

int pipePointer[PIPES_COUNT][2];
int pipeInputSize;

void createPipe(int size) {
    pipeInputSize = size;

    int pipeIndex;
    for (pipeIndex = 0; pipeIndex < PIPES_COUNT; pipeIndex++)
        if (pipe(pipePointer[pipeIndex]) == -1) fprintf(stderr, "Pipe failed");  //Tworzenie łącza
}

void writeToPipe(char* data, int pipeIndex) {
    write(pipePointer[pipeIndex][WRITE], data, pipeInputSize);  //zapis do pipe
}

void readFromPipe(char* data, int pipeIndex) {
    read(pipePointer[pipeIndex][READ], data, pipeInputSize);  //odczyt z pipe
}
