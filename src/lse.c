#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#include "senior.h"

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

   fclose(fp);
   int ierr;
   ierr = MPI_Init(&argc, &argv);
   
   {
      int seed = devrand(9999999);
      srand(seed);

      struct senior *me = malloc(sizeof(struct senior));
      me->compat = malloc(sizeof(int)*numSeniors);

      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &me->id);
      me->pairedWith = NO_ONE;
      me->waitingFor = NO_ONE;
      me->numSeniors = numSeniors;
      me->souped = FALSE;
      if (souped[me->id] == TRUE) {
         me->souped = TRUE;
         //printf("I, %d, ate the soup!\n", me->id);
         //announceDeath(me->id);
      }

      i = 0;
      while (i < numSeniors) {
         if (compatibility[me->id][i] == TRUE) {
            me->compat[i] = TRUE;
         } else {
            me->compat[i] = FALSE;
         }
   me->deathProb = deathProb;

         i++;
      }


      //printf(" + %d got my rank\n", me->id);      

      seniorMatch(me);

      free(me->compat);
      free(me);
   }

   ierr = MPI_Finalize();
}