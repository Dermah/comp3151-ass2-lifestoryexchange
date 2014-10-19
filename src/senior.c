#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <mpi.h>


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
   printf("%d dies blaming the mushroom soup.\n", me->id+1);
   int ierr = MPI_Finalize();
   exit(0);
}

void announceExchange (int i, int them) {
   printf("%d exchanges life stories with %d.\n", i+1, them+1);
}

void announceVegetation (struct senior *me) {
   printf("%d has a seniors’ moment.\n", me->id+1);
   int ierr = MPI_Finalize();
   exit(0);
}

int pickRandomSenior (struct senior *me) {
   int i = 0;
   int alive[me->numSeniors];
   int numAlive = 0;
   while (i < me->numSeniors) {
      if (me->compat[i] == TRUE) {
         alive[numAlive] = i;
         numAlive++;
      }
      i++;
   }

   int iPick = NO_ONE;

   if (numAlive == 0) {

      // couldn't find anyone compatible
      int foundLaters = FALSE;

      i = 0;
      while (foundLaters == FALSE && i < me->numSeniors) {
         if (me->compat[i] == MAYBE_LATER) {
            me->compat[i] = TRUE;
            foundLaters = TRUE;
         }
         i++;
      }

      if (foundLaters == FALSE) {
         announceVegetation(me);
      } else {
         i--;
      }
      iPick = i;
   } else {
      int pick = devrand(numAlive);
      //printf(" = %d there are %d people that I can talk to\n", me->id, numAlive);
      //printf(" + %d I pick %d\n", me->id, alive[pick]);
      iPick = alive[pick];
   }
   
   return iPick;
}

void askPotentialMatch (struct senior *me) {
   if (me->souped == TRUE) {
      announceDeath(me);
   }
   int i = pickRandomSenior(me);

   int message = LSE_I_WANT_TO_EXCHANGE;
   int ierr = MPI_Send(&message, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
   //printf(" + %d + told %d that LSE_I_WANT_TO_EXCHANGE\n", me->id, i);
   me->waitingFor = i;
   me->waitTimer = 5;
   assert(me->id != me->waitingFor);
   //printf("- %d waiting for %d\n", me->id, me->waitingFor);
}

void seniorMatch (struct senior *me) {
   int ierr;

   askPotentialMatch(me);   

   // while (not matched) {
   while (me->pairedWith == NO_ONE) {
      // if message waiting
      int recieved = FALSE;
      ierr = MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recieved, MPI_STATUS_IGNORE);
      if (recieved == TRUE) {
         //printf(" + %d got a message\n", me->id);
         
         // get message
         int recMes = 3892523984;
         MPI_Status status;
         ierr = MPI_Recv(&recMes, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
         
         // decide what to do

         if (status.MPI_SOURCE == me->waitingFor && recMes == LSE_THAT_SOUNDS_GREAT) {
            // if it's who we want to hear from
            //printf(" + %d SHE SAID YES %d\n", me->id, me->waitingFor);
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE == me->waitingFor && recMes == LSE_I_WANT_TO_EXCHANGE) {
            // if it's who we want to hear from
            //printf(" + %d SHE WANTS ME TOO %d\n", me->id, me->waitingFor);
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE == me->waitingFor && recMes == LSE_NO_THANKS) {
            // if it's who we want to hear from and they say no
            me->compat[me->waitingFor] = MAYBE_LATER;
            me->waitingFor = NO_ONE;
            //printf(" + %d REJECTED\n", me->id );
         } else if (status.MPI_SOURCE != me->waitingFor && recMes == LSE_I_WANT_TO_EXCHANGE) {
            // if it's not who we wanted, tell them no thanks
            int say = LSE_NO_THANKS;
            int ierr = MPI_Send(&say, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            //printf(" + %d had to tell %d that it wasn't to be\n", me->id, status.MPI_SOURCE);
         } else {
            //printf("! WARNING UNHANDLED CASE %d got a %d from %d\n", me->id, recMes, status.MPI_SOURCE);
         }
      } else {
         // no message
         if (me->waitingFor == NO_ONE) {
            askPotentialMatch(me);
         } else {
            //printf(" + %d I'm waiting for %d to message me (timeout %d)\n", me->id, me->waitingFor, me->waitTimer);
            me->waitTimer--;
            if (me->waitTimer == 0) {
               //printf(" + %d I think %d died\n", me->id, me->waitingFor);
               //announceVegetation(me);
               me->compat[me->waitingFor] = FALSE;
               me->waitingFor = NO_ONE;
            } else {
               usleep(10000);
            }
            // go and wait for message i.e. goto start of loop
         }
      }
   }

   announceExchange(me->id, me->pairedWith);
}