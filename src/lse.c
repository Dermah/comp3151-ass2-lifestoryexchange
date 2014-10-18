#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define TRUE  1
#define FALSE 0

void announceDeath (int me) {
   printf("%d dies blaming the mushroom soup.\n", me);
   int ierr = MPI_Finalize();
   exit(0);
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
      int me;
      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &me);
      
      if (souped[me] == TRUE) {
         printf("I, %d, ate the soup!\n", me);
         announceDeath(me);
      }

      printf("I, %d, am compatible with ", me);
      int i = 0;
      while (i < numSeniors) {
         if (compatibility[me][i] == TRUE) {
            printf("%d ", i);
         }
         i++;
      }
      printf("\n");
   }

   ierr = MPI_Finalize();
}