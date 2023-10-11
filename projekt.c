// program dla Renczewskiego
// kolejnosc sygnalow: K1 pamiec dzielona, K2 kolejka komunikatow, K3 kolejka fifo
// tresc zadania w pliku projekt.pdf
// oceniony na 3 w II terminie po 6 godzinach i 15 minutach obrony (gdybym oddawal w I terminie by poszedl na 5)
// caly napisany poprawnie - jedyny blad to nie zamykanie procesow po wykonaniu pliku skonczonego
// 
// obs³uga:
// gcc -o program projekt.c - kompilacja
// ./program < /dev/urandom - uruchomienie programu i przesylanie do niego wartosci losowych
// ./program < file - uruchomienie programu i przeslanie skoncoznego pliku file ktory wczesniej utworzysz
// 
// w drugiej konsoli:
// kill -int nr_pidu   	- zatrzymujesz proces o zadanym pidzie
// kill -cont nr_ pidu  - wznawiasz proces o zadanym pidzie
// kill -term nr_pidu   - konczysz proces o zadanym pidzie
//
// po zakonczeniu wprowadzania wpisujesz "ps aux | grep program" by zobaczyc procesy posiadajace fraze "program"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>

#define BUF_SIZE 64 //bufor do wczytywania danych

int pid[3];// tablica pidow procesow potomnych
int memId;//id pamieci wspoldzileonej
int pracuj=1;//stan 
int koniec =0; //czy sie konczy
int queue; //id kolejki
int fifo; //id fifo
char * mem; //wskaznik na pamiec wspoldzielona
FILE * fp;//plik przez ktory przesylane sa pidy procesow

void Clear(int id){ // zwalnia zasoby
	close(fifo); 

	if(id<=1){
		shmdt(mem); // odlaczenie pamieci wspoldzielonej dla p1 i p2
	}
	if(id==0){ // dla procesu 1 usuwa pamiec dzielona oraz kolejke oraz kolejke fifo i plik z pidami
		shmctl(memId,IPC_RMID,0);
		msgctl(queue,IPC_RMID,0);
		remove("/tmp/fifo");
		remove("/tmp/pid");
	}
}

void err(char * tekst){ // wyswietla komunikat bledu oraz wysyla sygnal do pozostalych procesow o awaryjnym zamknieciu oraz sprzata po sobie i konczy proces
	perror(tekst);
	int pp = getpid();
	int j;
	int i=0;
	if(pp==pid[1]) i =1;
	if(pp==pid[2]) i =2;
	for(j=0;j<3;j++)
		if(j!=i){
			kill(pid[j],SIGUSR2);	
		}
	Clear(i);
	exit(-1);
}

void errorProc(int signo){ //procedura obslugi sygnalu sigusr2 awza
	int pp = getpid();
	int i=0;
	if(pp==pid[1]) i =1;
	if(pp==pid[2]) i =2;
	Clear(i);
	exit(-1);
}

void ReadPid(){ // odczytanie pidow
	fp = fopen("/tmp/pid","r");
	int r=sizeof(int)*3;
	int readed;	
	char * out = (char*) pid;
	while(r>0){
		readed = fread(out,1,sizeof(pid),fp); 
		if(readed!= EOF){
		out+=readed;
		r-=readed;
		}		
	}
	fclose(fp);
}

void signalProc(int signo){ //obsluga sygnalu
	int pp = getpid();
	int j;
	int i=0;
	if(pp==pid[1]) i =1;
	if(pp==pid[2]) i =2;
	char cmd[2]; // dwa bajty
	switch(signo){ //identyfikacja sygnalu
		case SIGINT:
			if(!pracuj) return;
			pracuj =0;
			printf("proces %d  - stop\n",i+1);
			cmd[0]=cmd[1]= 0;
		break;
		case SIGCONT:
			if(pracuj) return;
			pracuj = 1;
			printf("proces %d - start\n",i+1);
			cmd[0]=cmd[1]= 1;
		break;
		case SIGTERM:
			if(koniec) return;
			koniec = 1;
			printf("proces %d - koniec\n",i+1);
			cmd[0]=cmd[1]= 2;
		break;
	}

	if(write(fifo,cmd,2)<0) err("fifo write"); //zapisanie do fifo dwoch bajtow
	for(j=0;j<3;j++)
		if(j!=i)
			kill(pid[j],SIGUSR1); //S4
	if(signo==SIGTERM){ // czy to koniec
		Clear(i);
		exit(0);
	}
}

void procGetCmd(int signo){
	int pp = getpid();
	int i=0;
	if(pp==pid[1]) i =1;
	if(pp==pid[2]) i =2;
	char cmd;
	if(read(fifo,&cmd,1)<0) err("fifo read "); //czyta jeden bajt z kolejki fifo
	switch(cmd){
		case 0:
			if(!pracuj) return;
			pracuj = 0;
			printf("proces %d zostal zatrzymany\n",i+1);
		break;
		case 1:
			if(pracuj) return;
			pracuj =1;
			printf("proces %d wystartowal\n",i+1);
		break;
		case 2:
			if(koniec) return;
			koniec=1;
			printf("proces %d sie zakonczyl\n",i+1);
			Clear(i);
			exit(0);
		break;
	}
}

typedef struct{ //struktura komunikatu wyslanego przez kolejke
	long type;
	int  len;
} SMessage;

SMessage msgr;
SMessage msgs;

void proc3(){
	ReadPid();
	while(1){
		if( pracuj ){
			int er= msgrcv(queue,&msgr,4,0,0); //odczytanie z kolejki komunikatu
			if(er==-1){
				if(errno == ENOMSG || errno == EINTR) //bark komunikatu lub przerwanie
					continue;
				else	
					err("blad przyjscia sygnalu - proces 3");
			}else{
				printf("Dlugosc: %d\n",msgr.len); //wyswietl na erkan dlugosc
			}
		}
	}
}

void proc2(){
	ReadPid();
	mem = (char*) shmat(memId,0,0); //wskaznik do pamieci wspoldzielonej
	if ((int)mem ==-1) err("blad wskaznika do pamieci wspoldzielonej - proces 2");
	int len=0;
	int i;
	mem[1]=1; 
	while(1){
		if(pracuj && mem[1]==2){
			for( i=0; i< mem[0]; i++){
				if(mem[2+i]!='\n'){
					len++;
				}else{
					msgs.type=1;
					msgs.len = len;
					len=0;
					int er = msgsnd(queue,&msgs,4,IPC_NOWAIT); //wyslanie komunikatu do kolejki
					if(er==-1){
						if(errno != EINTR && errno != EAGAIN)
							err("blad podczas wysylania komunikatu do kolejki - proces 2");
					}
				}	
			}		
			mem[1]=1;
		}
	}
}

void proc1(){
	int n;
	ReadPid();//czyta pidy i je wyswielta
	mem = (char*) shmat(memId,0,0); //mapowanie adresu pamieci
	if ((int)mem ==-1) err("blad podczas mapowania pamieci - proces 1");
	mem[1]=1;
	printf("\nPIDY procesow: %d %d %d\n",pid[0],pid[1],pid[2]);
	while(1){
		if(pracuj && mem[1]==1){			
			n = read(STDIN_FILENO,&mem[2],BUF_SIZE);//czytanie ze standardowego wejscia
			if(n>0 ){ 				
				mem[0] = n; //ilosc przeczytanych znakow
				mem[1] = 2;
			}			
		}
	}
}

int main(void){	
	sigset_t mask;
	int fl = fcntl(STDIN_FILENO,F_GETFL,0); //odczyt ze standardowego wejscia nie bedzie blokowany
	fcntl(STDIN_FILENO,F_SETFL,fl| O_NONBLOCK);
	remove("/tmp/fifo");
	if(mknod("/tmp/fifo",O_CREAT|S_IFIFO | 0666,0)==-1) err("blad podczas tworzenia kolejki fifo");
	fifo = open("/tmp/fifo",O_RDWR);
	if(fifo==-1) err("fifo nie chce sie otworzyc");
	fp = fopen("/tmp/pid","w");
	if(fp==NULL) err("nie mozna otworzyc pliku /tmp/pid");

	queue = msgget(IPC_PRIVATE,IPC_CREAT | 0660); //nie musimy klucza podowac (same sie generuja) dostep do kolejki maja procesy potomne
	if(queue==-1) err("blad podczas tworzenia kolejki");

	memId = shmget(IPC_PRIVATE,BUF_SIZE+2,IPC_CREAT | 0666); //pamiec wspoldzielona
	if(memId < 0) err("blad podczas utworzenia pamieci wspoldzielonej");

// procedury obslugi sygnalu
	signal(SIGINT,signalProc);
	signal(SIGCONT,signalProc);
	signal(SIGTERM,signalProc);
	signal(SIGUSR1,procGetCmd);
	signal(SIGUSR2,errorProc);
	
	if((pid[2]=fork())==0)
		proc3();
	else
		if((pid[1]=fork())==0)
			proc2();
		else
			if((pid[0]=fork())==0)
				proc1();
			else{
				fwrite(pid,1,sizeof(pid),fp);
				fclose(fp);
			}

	sigfillset(&mask);
	sigprocmask(SIG_BLOCK,&mask,NULL);
	return 0;
}
