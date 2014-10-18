#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

#define TRUE  1
#define FALSE 0
  
int main(int argc, char **argv)
{
   if (argc == 1) {
      fprintf(stderr, "No arguments given\n");
      exit(1);
   }

   char *progName = argv[0];
   char *fileName = argv[1];
   char *deathProb = argv[2];

   FILE *fp;
   fp = fopen(fileName, "r");

   if (fp == NULL) {
      fprintf(stderr, "Could not open file %s\n", argv[0]);
      exit(1);
   }

   int numSeniors;

   fscanf(fp, "%d\n", &numSeniors);

   printf("%d\n", numSeniors);

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

   i = 0;
   j = 0;

   while (i < numSeniors) {
      while (j < numSeniors) {
         printf("%d", compatibility[i][j]);
         j++;
      }
      printf("\n");
      j = 0;
      i++;
   }

   printf("SOUP\n");

   i = 0;
   while (i < numSeniors) {
      printf("%d, ", souped[i]);
      i++;
   }

   printf("\n");
}