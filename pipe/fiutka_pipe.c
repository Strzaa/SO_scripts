#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main()
{
int deskryptor1[2], deskryptor2[2], deskryptor3[2], deskryptor4[2], i;
int pid, pid1, pid2, root;
int liczba, dokl;

pid = fork();
if(pid == 0)
{
    sleep(3);
    printf("child.1 = %d, parent = %d\n", getpid(), getppid());
}else
{
    pid1 = fork();
    if (pid1 == 0) {
        sleep(2);
        printf("child.2 = %d, parent = %d\n", getpid(), getppid());
    }else
    {
        pid2 = fork();
        if(pid2 == 0)
        {
            printf("child.3 = %d, parent = %d\n", getpid(), getppid());
        }else
        {
            sleep(3);
            printf("parent = %d\n", getpid());
            root = getpid();
        }
    }
}

if (getpid() == root) {
    if (((pipe(deskryptor1)) == -1) && ((pipe(deskryptor2)) == -1) && ((pipe(deskryptor3)) == -1) && ((pipe(deskryptor4)) == -1)) {
            printf("Błąd");
    }
}

if(getpid() == getppid() + 1)
{
    printf("\n\nPodaj x: ");
    scanf("%d", &liczba);
    printf("\nPodaj dokladnosc: ");
    scanf("%d", &dokl);
    //close(deskryptor1[0]);
    if ((write(deskryptor1[1], &liczba, sizeof(int))) == -1) printf("Bład pisania liczba des1");
    //close(deskryptor1[1]);
    //close(deskryptor2[0]);
    if ((write(deskryptor2[1], &dokl, sizeof(int))) == -1) printf("Bład pisania dokladnosc des2");
    //close(deskryptor2[1]);
}

if(getpid() == getppid() + 2)
{
    //close(deskryptor1[1]);
    if ((read(deskryptor1[0], &liczba, sizeof(liczba)) == -1)) printf("Bład czytania liczba des1");
    //close(deskryptor1[0]);
    //close(deskryptor2[1]);
    if ((read(deskryptor2[0], &dokl, sizeof(dokl))) == -1) printf("Bład czytania dokladnosc des2");
    //close(deskryptor2[0]);
    printf("\n%d, %d", liczba, dokl);
}
    sleep(10);
}
