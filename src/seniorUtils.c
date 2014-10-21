#include <stdio.h>
#include <stdlib.h>
#include "senior.h"

// come up with a random number between 0 and num-1
int devrand(int high) {
   FILE *fp = fopen("/dev/urandom", "r");

   int num;
   fread(&num, sizeof(int), 1, fp);
   fclose(fp);
   
   return abs((num%high));
}

void announceDeath (struct senior *me) {
   printf("%d dies blaming the mushroom soup.\n", (me->id)+1);
   senior_finalise(me);
   exit(0);
}

void announceExchange (struct senior *me) {
   printf("%d exchanges life stories with %d.\n", (me->id)+1, (me->pairedWith)+1);
}

void announceVegetation (struct senior *me) {
   printf("%d has a seniors’ moment.\n", (me->id)+1);
   senior_finalise(me);
   exit(0);
}