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

#define BROJ_MS 20
#define BROJ_LINUX 20

void udji(int vrsta);
void izadji(int vrsta);

pthread_mutex_t m;		//M - varijabla zaključavanja (monitorski semafor)
pthread_cond_t uv[2];	//2 reda uvjeta uv[2]

int brJedu[2]= {0,0};	// dva brojača koliko programera je u restoranu
 					// = 0 (MS) ili 1 (Linux)
int ceka[2] = {0,0}; 	// dva brojača koliko programera čeka
int siti[2] = {0,0}; 	// brojač koliko se programera jedne vrste najelo
int n;



void *programer(void *v) {
	int vr = *((int*)v);
	while(true) {
		udji(vr);
		sleep(2);
		sleep(2);
		izadji(vr);
     }
}

void udji(int vrsta) {
	pthread_mutex_lock(&m);

	ceka[vrsta]++;
	
	while((brJedu[1 - vrsta] > 0) || ((siti[vrsta] >= n) && (ceka[1 - vrsta] > 0))) 
		pthread_cond_wait(&uv[vrsta], &m);


	brJedu[vrsta]++;
	ceka[vrsta]--;
	siti[1 - vrsta] = 0;
	
	if (vrsta == 0)
		printf("-----> MS programer ulazi, %d MS programera je u restoranu\n", brJedu[vrsta]);
	else
		printf("-----> Linux programer ulazi, %d Linux programera je u restoranu\n", brJedu[vrsta]);
		
	if (ceka[1 - vrsta] > 0) 
		siti[vrsta]++;
	
	pthread_mutex_unlock(&m);
}


void izadji(int vrsta) {
	pthread_mutex_lock(&m);
	
	brJedu[vrsta]--;
	
	if (vrsta == 0)
		printf("<----- MS programer izlazi iz restorana, %d ih ostaje unutra\n", brJedu[vrsta]);
	else
		printf("<----- Linux programer izlazi iz restorana, %d ih ostaje unutra\n", brJedu[vrsta]);
	
	if (brJedu[vrsta] == 0) {
		pthread_cond_broadcast(&uv[1 - vrsta]);
	}
	
	pthread_mutex_unlock(&m);
}

int main(void) {
	printf("Unesite najveći broj programera jedne vrste koji mogu jesti ako programeri druge vrste cekaju na ulazak:\n");
	scanf("%d", &n);
	
	pthread_mutex_init(&m, NULL);
	
	pthread_cond_init(&uv[0],NULL);
	pthread_cond_init(&uv[1],NULL);
	
	pthread_t thr_id[BROJ_MS + BROJ_LINUX];   
	
	int vrsta = 0; 			// = 0 (MS) ili 1 (Linux)
	for(int i = 0; i < BROJ_MS; i++) {
		if (pthread_create(&thr_id[i], NULL, programer, &vrsta) != 0) {
			printf("Greska kod stvaranja %d. dretve", i + 1);
			exit(1);
		}
	}
	
	vrsta = 1;
	
	for(int i = 0; i < BROJ_LINUX; i++) {
		if (pthread_create(&thr_id[BROJ_MS + i], NULL, programer, &vrsta) != 0) {
			printf("Greska kod stvaranja %d. dretve", i + 1);
			exit(1);
		}
	}
	
	
	for(int i = 0; i < BROJ_MS + BROJ_LINUX; i++) {
		pthread_join(thr_id[i], NULL);
	}
	

	return 0;
}
