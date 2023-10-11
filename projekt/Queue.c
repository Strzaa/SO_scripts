#include <err.h>
#include <errno.h>
#include <fcntl.h> /* O_flags */
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "Properties.h"
struct mymsgbuf {
    long type;
    char data[NOTIFICATION_SIZE];
} msg;

int queue;

void createQueue() {
    key_t key = ftok(".", 'm');
    if ((queue = msgget(key, IPC_CREAT | 0660)) == -1) perror("Otwieranie_kolejki");
}

void removeQueue() {
    msgctl(queue, IPC_RMID, 0);
}

void writeToQueue(char *data) {
    msg.type = 1;
    strcpy(msg.data, data);
    if (msgsnd(queue, &msg, sizeof(msg), 0) == -1) perror("Wysylanie");
}

void readFromQueue(char *data) {
    if (msgrcv(queue, &msg, sizeof(msg), 0, 0) == -1) perror("Odbieranie");
    strcpy(data, msg.data);
}