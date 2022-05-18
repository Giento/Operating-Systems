#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>

#define BROJ_MJESTA 8

/*
int sem_init(sem_t *sem, int mprocesi, unsigned int koliko);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
*/

int idZauzeto;
int idSjeo;
int idKraj;
int idIzasao;

sem_t *zauzeto; 	//globalna varijabla = za kazaljku na 
				//objekt u zajedničkoj memoriji
sem_t *sjeo;
sem_t *kraj;
sem_t *izasao;

void posjetitelj(int i){
	while(true) {
		sem_wait(zauzeto);
		//udi i sjedi;
		printf("%d sjeda na slobodno mjesto\n", i);
		sem_post(sjeo);
		
		sem_wait(kraj);
		//ustani i izadi;
		printf("%d se ustaje i izlazi\n", i);
		sem_post(izasao);
	}
}

void vrtuljak(){
	while(true) {
		printf("Mozete uci u vrtuljak\n");
		for(int i = 1; i <= BROJ_MJESTA; i++) {
			sem_post(zauzeto);
		}
		for(int i = 0; i < BROJ_MJESTA; i++)
			sem_wait(sjeo);
			
		//pokreni vrtuljak;
		printf("Svih %d mjesta je popunjeno.\n", BROJ_MJESTA);
		printf("VEZITE SE, POLIJECEMO!!!\n");
		printf("------------------------------------------------------\n");
		sleep(6);
		//zaustavi vrtuljak;
		printf("Voznja je zavrsila, molimo vas da se ustanete i izadete\n");
		sleep(2);
			
		for(int i = 0; i < BROJ_MJESTA; i++)
			sem_post(kraj);
		for(int i = 0; i < BROJ_MJESTA; i++)
			sem_wait(izasao);
	}
}

int main(void) {
	
	idZauzeto = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	idSjeo = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	idKraj = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	idIzasao = shmget (IPC_PRIVATE, sizeof(sem_t), 0600);
	/*
	printf("%d\n", idZauzeto);
	printf("%d\n", idSjeo);
	printf("%d\n", idKraj);
	printf("%d\n", idIzasao);
	*/
	
	zauzeto = shmat (idZauzeto, NULL, 0);
	sjeo = shmat (idSjeo, NULL, 0);
	kraj = shmat (idKraj, NULL, 0);
	izasao = shmat (idIzasao, NULL, 0);	 	
	
	sem_init(zauzeto, 1, 0);
	sem_init(sjeo, 1, 0);
	sem_init(kraj, 1, 0);
	sem_init(izasao, 1, 0);
	//sad je sve inicijalizirano
	
	if (fork() == 0) {
		vrtuljak();
		exit(0);
	}
	for (int i = 0; i < 2 * BROJ_MJESTA; i++) {
		if (fork() == 0) {
			posjetitelj(i + 1);
			exit(0);
		}
	}
	
	//tu pocinje unistavanje resursa
	
	for(int i = 0; i < 2 * BROJ_MJESTA; i++)
		wait(NULL);

	sem_destroy(zauzeto);
	sem_destroy(sjeo);
	sem_destroy(kraj);
	sem_destroy(izasao);
	
	shmdt(zauzeto);
	shmdt(sjeo);
	shmdt(kraj);
	shmdt(izasao);
	
	shmctl (idZauzeto, IPC_RMID, NULL);
	shmctl (idSjeo, IPC_RMID, NULL);
	shmctl (idKraj, IPC_RMID, NULL);
	shmctl (idIzasao, IPC_RMID, NULL);


	return 0;
}
