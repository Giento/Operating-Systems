#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

void oslobodi_resurse(int sig);

int id; /* identifikacijski broj segmenta */
int *radna_ulazna;
int radna_izlazna;
int broj_ponavljanja;

/* posao izlazne dretve */
void *zapisi_u_file() {
	printf("Pokrenuta IZLAZNA DRETVA\n");
	for (int i = 0; i < broj_ponavljanja; i++) {
		FILE *ispis = fopen("ispis.txt", "a+");
		while (radna_izlazna == 0);
		fprintf(ispis, "%d\n", radna_izlazna);
		fclose(ispis);
		printf("IZLAZNA DRETVA: broj upisan u datoteku %d\n", radna_izlazna);
		radna_izlazna = 0;
	}
}

/* posao ulaznog procesa */
void generiraj_i_upisi() {
	srand(time(NULL));
	sleep(1);
	for(int i = 0; i < broj_ponavljanja; i++) {
		while (*radna_ulazna != 0) sleep(3);
		*radna_ulazna = rand() % 100 + 1;
		printf("\nULAZNI PROCES: broj %d\n", *radna_ulazna);
	}
}

void oslobodi_resurse(int sig) {
	shmdt((char *) radna_ulazna);
	shmctl(id, IPC_RMID, NULL);
	exit(0);
}


int main(int argc, char *argv[]) { 
    	broj_ponavljanja = atoi(argv[1]);
	pthread_t thr_id;
	
	/* ako dode do prekida, obrisi memoriju */
	struct sigaction act;
	act.sa_handler = oslobodi_resurse;
	sigaction(SIGINT, &act, NULL);
	
	/* zauzimanje zajednicke memorije */
	id = shmget(IPC_PRIVATE, sizeof(int), 0600);
	if (id == -1)
		exit(1); //nema zajednicke memorije (greska)
	radna_ulazna = (int *) shmat(id, NULL, 0);
	
	*radna_ulazna = 0;
	radna_izlazna = 0;
	
	printf("Pokrenuta RADNA DRETVA\n");
	
	/* stvaranje ulaznog procesa */
	if (fork() == 0) {
		printf("Pokrenut ULAZNI PROCES\n");
		generiraj_i_upisi();
		exit(0);
	}
	
	/* stvaranje izlazne dretve */
	if (pthread_create(&thr_id, NULL, zapisi_u_file, NULL) != 0) {
		printf("Greska kod stvaranja IZLAZNE DRETVE");
		exit(1);
	}
	
	/* posao radne dretve */
	for (int i = 0; i < broj_ponavljanja; i++) {	
		while (*radna_ulazna == 0)
			sleep(1);
		printf("RADNA DRETVA: pročitan broj %d", *radna_ulazna);
		printf(" i povećan na %d\n", ++(*radna_ulazna));
		radna_izlazna = *radna_ulazna;
		*radna_ulazna = 0;
	}
	
	sleep(1);
	printf("\n");
	printf("Završila RADNA DRETVA\n");
	pthread_join(thr_id, NULL);
	printf("Završila IZLAZNA DRETVA\n");
	wait(NULL);
	printf("Završio ULAZNI PROCES\n");
	oslobodi_resurse(0);

	return 0;
}




