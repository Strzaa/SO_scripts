- Jak odpalić program?
	1. Potrzebujesz środowiska unixowego np. Ubuntu na Oracle VM Virtual Box, albo Windows Linux Subsystem (na WLS nie działają kolejki komunikatów)

	2. Otwierasz terminal skrótem klawiszowym ctrl(Windows)/cmd(Mac) + alt + T, bądź będąc w folderze z plikami - prawy przycisk myszy -> open in terminal

	3. Wszystkie pliki projektu muszą być w tym samym folderze (jesli cos jest w innym folderze, wypakuj do tego samego)

	4. w terminalu wpisujesz gcc --version, jeśli wyświetla się komunikat o wersji - świetnie, przejdz do następego punktu, jeśli nie - musisz zainstalować kompilator https://linuxize.com/post/how-to-install-gcc-compiler-on-ubuntu-18-04/

	5. Żeby skompilować program wpisujesz gcc -o 'nazwa skompilowanego programu' 'nazwa programu.c'
		przykład: gcc -o out projekt.c (przy czym ja nazywam pliczek wyjsciowy 'out')

	6. Żeby odpalić program wpisujesz ./'nazwa skompilowanego programu'
		przykład: ./out

	Aby wysłać sygnał można po prostu użyć komendy: kill -'numer sygnału' 'pid procesu'
		przykład: kill -12 12548 
		(przy czym 12 to nr sygnału SIGUSR2, a 12548 to PID procesu)

- Prezentacja programu
	Komendy:
		'ipcs' 		- pokazuje semafory, kolejki komunikatów i pamięci współdzielone
		'pstree' 	- pokazuje drzewo procesów
		'ls -al'	- wyswietla listę plików w danym direcory

	Stałe, które można zmienić (Properties.h):
		- DEBUG - po ustawieniu jej wartości na np 1, program będzie pokazywał dane, które otrzymują procesy
		- INPUT_SIZE - wielkość bufora - tyle program czyta danych na raz

	1. Kompilujesz i odpalasz program projektu i signal w dwóch osobnych terminalach
	2. Prezentujesz działanie w trzech trybach
	3. Prezentujesz wysylanie sygnałów (najlepiej w trybie odczytu z dev/urandom)
	4. (Program projektu nadal ma działać!) Włączasz trzeci terminal / zamykasz program signal i korzystasz z tego terminala i wpisujesz komendy:
		1. 'pstree | grep 'nazwa skompilowanego programu'' przykład: pstree | grep out
		(grep znajduje i wyświetla tylko to co jest po nim napisane)
		pokazujesz, że istnieje drzewko procesów naszego programu

		2. 'ls -al' -al są parametrami a - pokaż wszystkie pliki, l - wyswietlenie w postaci listy, dla czytelnosci przykład: ls -al
		pokazujesz, że istnieją pliki stworzone przez program

		3. 'ipcs' przykład: ipcs
		pokazujesz, że program stworzył semafory/kolejki komunikatów/ pamięci współdzielone
		(w systemie mogą być już inne segmenty pamięci współdzielonej)

	5. Kończysz działanie programu wysyłając mu odpowiedni sygnał
	6. Prezentujesz fakt usunięcia wszystkich wykorzystanych przez program zasobów wywołując te same polecenia co w pkt 4










