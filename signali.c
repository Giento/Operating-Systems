#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <signal.h>
#include <math.h>

/* funkcije za obradu signala, navedene ispod main-a */ 
void obradi_sigusr1(int sig); 
void obradi_sigterm(int sig); 
void obradi_sigint(int sig); 

int broj;
int nije_kraj = 1; 


int main(void) {
	struct sigaction act; 

   /* 1. maskiranje signala SIGUSR1 */
   act.sa_handler = obradi_sigusr1;   /* kojom se funkcijom signal obrađuje */ 
   sigemptyset(&act.sa_mask); 
   sigaddset(&act.sa_mask, SIGTERM); /* blokirati i SIGTERM za vrijeme obrade */ 
   act.sa_flags = 0; /* naprednije mogućnosti preskočene */ 
   sigaction(SIGUSR1, &act, NULL); /* maskiranje signala preko sučelja OS-a */

   /* 2. maskiranje signala SIGTERM */ 
   act.sa_handler = obradi_sigterm; 
   sigemptyset(&act.sa_mask); 
   sigaction(SIGTERM, &act, NULL); 

   /* 3. maskiranje signala SIGINT */ 
   act.sa_handler = obradi_sigint; 
   sigaction(SIGINT, &act, NULL);
   
   printf("Process ID pokrenutog programa je: %ld\n", (long) getpid());
   
   /* procitaj broj iz status.txt i spremi ga u brojStat*/
   char brojStat[10];
   FILE *statR = fopen("status.txt", "r");
   fgets(brojStat, 10, statR);		//cita liniju iz "status.txt"
   fclose(statR);
   sscanf(brojStat, "%d", &broj); 	//pretvara char array u int

	/* ako je broj == 0, onda pronadji zadnji u obrada.txt */
	if (broj == 0) {
		FILE *obrR = fopen("obrada.txt", "r");
		char brojObr[10];
		while (fgets(brojObr, 10, obrR)) {
			sscanf(brojObr, "%d", &broj);
			broj = sqrt(broj);
		}
		fclose(obrR);
	}
	
	/* zapisi 0 u status.txt */
	FILE *statW = fopen("status.txt", "w");
	fprintf(statW, "%d\n", 0);
	fclose(statW);
	
	/* posao koji program radi; upisuj kvadrate brojeva u "obrada.txt" */
	FILE *obrW = fopen("obrada.txt", "a+");
	int x;
	while(nije_kraj) {
		broj++;
		x = pow(broj, 2);
		printf("\nU tijeku obrada broja: %d\n", broj);
		sleep(5);
		fprintf(obrW, "%d\n", x);
	}
	fclose(obrW);
	
	return 0;
}

void obradi_sigusr1(int sig) {
   printf("\n--------- U obradi se trenutno koristi broj: %d ----------\n", broj); 
}

void obradi_sigterm(int sig) { 
   FILE *statS = fopen("status.txt", "w");
	fprintf(statS, "%d\n", broj);
	fclose(statS);
	printf("Primio signal SIGTERM, upisujem zadnji broj u \"status.txt\" i prekidam\n");
	nije_kraj = 0;
} 

void obradi_sigint(int sig) {
   printf("\nPrimio signal SIGINT, prekidam rad\n"); 
   exit(1); 
}
