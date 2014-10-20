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

   // get arguments
   char *progName = argv[0];
   char *fileName = argv[1];
   float deathProb = atof(argv[2]);

   // open the given compatibility file
   FILE *fp;
   fp = fopen(fileName, "r");
   if (fp == NULL) {
      fprintf(stderr, "Could not open file %s\n", argv[0]);
      exit(1);
   }

   // read number of seniors
   int numSeniors;
   fscanf(fp, "%d\n", &numSeniors);

   // read compatibility matrix and 
   // initialise poison information array
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

   // read poisons information
   i = 0;
   while (fscanf(fp, "%d", &i) != EOF) {
      souped[i-1] = TRUE;
   }

   // done with config matrix
   fclose(fp);

   // begin parallelism
   struct senior* me = senior_init(numSeniors, argc, argv);

   // check if we ate the soup   
   me->deathProb = deathProb;
   if (souped[me->id] == TRUE) {
      me->souped = TRUE;
   }

   // load compatibility array for this senior
   i = 0;
   while (i < numSeniors) {
      if (compatibility[me->id][i] == TRUE) {
         me->compat[i] = TRUE;
      } else {
         me->compat[i] = FALSE;
      }
      i++;
   }

   // see if we can find someone to LSE with
   seniorMatch(me);

   // done: free and exit
   senior_finalise(me);
}