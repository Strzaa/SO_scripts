#ifndef _PROPERTIES_H
#define _PROPERTIES_H

#define DEBUG 1  // tryb debugowania

#define INPUT_SIZE 1024       // wielkość bufora danych
#define NOTIFICATION_SIZE 64  // wielkosc bufora notyfikacji

#define PROCESSES_COUNT 3  // liczba procesów potomnych
#define SIGNALS_COUNT 64   // liczba wszystkich sygnałów
#define PIPES_COUNT 2      // połączenie między procesami

#define FIRST_PIPE 0
#define SECOND_PIPE 1

#define SIGNAL_STOP 10    // SIGUSR1
#define SIGNAL_RESUME 12  // SIGUSR2
#define SIGNAL_END 2      // SIGINT
#define SIGNAL_NOTIFY 18  // SIGCONT - sygnał notyfikacji o rządaniu operatora

#define NOTIFICATION_STOP "stop"
#define NOTIFICATION_RESUME "resume"

#define QUEUE_SEM_LOCK 3

#endif