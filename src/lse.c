#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define TRUE  1
#define FALSE 0

#define LSE_I_WANT_TO_EXCHANGE 0

#define NO_ONE -1 

struct myInfo {
   int id;
   int *compat; //of size numSeniors
   int waitingFor;
   int pairedWith;
   int waitTimer;
};

void announceDeath (struct myInfo *me) {
   printf("%d dies blaming the mushroom soup.\n", me->id);
   int ierr = MPI_Finalize();
   exit(0);
}

void announceExchange(int i, int them) {
   printf("%d exchanges life stories with %d.\n", i, them);
}

void announceVegetation(struct myInfo *me) {
   printf("%d has a seniors’ moment.\n", me->id);
}
int main(int argc, char **argv)
{
   if (argc == 1) {
      fprintf(stderr, "No arguments given\n");
      exit(1);
   }

   char *progName = argv[0];
   char *fileName = argv[1];
   float deathProb = atof(argv[2]);

   FILE *fp;
   fp = fopen(fileName, "r");

   if (fp == NULL) {
      fprintf(stderr, "Could not open file %s\n", argv[0]);
      exit(1);
   }

   int numSeniors;
   fscanf(fp, "%d\n", &numSeniors);

   int compatibility[numSeniors][numSeniors];
   int souped[numSeniors];

   int i = 0;
   int j = 0;
   while (j < numSeniors) {
      souped[j] = FALSE;
      while (i < numSeniors) {
         fscanf(fp, "%1d ", &compatibility[i][j]);
         i++;
      }
      i = 0;
      j++;
   }

   i = 0;
   while (fscanf(fp, "%d", &i) != EOF) {
      souped[i-1] = TRUE;
   }

   int ierr;
   ierr = MPI_Init(&argc, &argv);
   
   {
      struct myInfo me;
      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &me.id);
      me.pairedWith = NO_ONE;
      me.compat = malloc(sizeof(int)*numSeniors);
      me.waitingFor = NO_ONE;

      i = 0;
      while (i < numSeniors) {
         if (compatibility[me.id][i] == TRUE) {
            me.compat[i] = TRUE;
         }

         i++;
      }

      printf(" + %d got my rank\n", me.id);      

      if (souped[me.id] == TRUE) {
         printf("I, %d, ate the soup!\n", me.id);
         //announceDeath(me.id);
      }

      // just tell the first person you are compatible with that you wanna chat
      i = 0;
      while (compatibility[me.id][i] == FALSE) {
         i++;
      }
      int message = LSE_I_WANT_TO_EXCHANGE;
      ierr = MPI_Send(&message, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      printf(" + %d + told %d that LSE_I_WANT_TO_EXCHANGE\n", me.id, i);
      me.waitingFor = i;


   }

   ierr = MPI_Finalize();
}