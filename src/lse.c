#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
  
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
}