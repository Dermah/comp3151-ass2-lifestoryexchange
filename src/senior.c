#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <mpi.h>

#include "senior.h"

struct senior* senior_init(int numSeniors, int argc, char **argv) {
   int ierr;
   ierr = MPI_Init(&argc, &argv);

   int seed = devrand(9999999);
   srand(seed);
   
   struct senior *me = malloc(sizeof(struct senior));
   ierr = MPI_Comm_rank(MPI_COMM_WORLD, &me->id);
   me->numSeniors = numSeniors;
   me->compat = malloc(sizeof(int)*numSeniors);
   me->pairedWith = NO_ONE;
   me->waitingFor = NO_ONE;
   me->souped = FALSE;
   me->deathProb = 0;
   me->waitTimer = 0;
   return me;
}

void senior_finalise(struct senior *me) {
   free(me->compat);
   free(me);
   int ierr = MPI_Finalize();
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
      int pick = (rand() % numAlive);
      //printf(" = %d there are %d people that I can talk to\n", me->id, numAlive);
      //printf(" + %d I pick %d\n", me->id, alive[pick]);
      iPick = alive[pick];
   }
   
   return iPick;
}

void askPotentialMatch (struct senior *me) {
   if (me->souped == TRUE) {
      float roll = (float)rand()/(float)(RAND_MAX);
      if (roll <= me->deathProb) {
         //printf("UNLUCKY %f <= %f\n", roll,me->deathProb);
         announceDeath(me);
      }
   }
   int i = pickRandomSenior(me);

   int message = LSE_I_WANT_TO_EXCHANGE;
   int ierr = MPI_Send(&message, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
   //printf(" + %d + told %d that LSE_I_WANT_TO_EXCHANGE\n", me->id, i);
   me->waitingFor = i;
   me->waitTimer = SENIOR_TIMEOUT_CHECKS;
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
         int recMes = LSE_NO_MESSAGE;
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
         } else if (me->waitingFor == NO_ONE && recMes == LSE_I_WANT_TO_EXCHANGE) {
            // we weren't expecting anything so let's accept the offer
            int message = LSE_THAT_SOUNDS_GREAT;
            int ierr = MPI_Send(&message, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            //printf(" + %d + ACCEPTED %d 's' PROPOSTION WITH A LSE_THAT_SOUNDS_GREAT\n", me->id, status.MPI_SOURCE);
            me->waitingFor = status.MPI_SOURCE;
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE != me->waitingFor && recMes == LSE_I_WANT_TO_EXCHANGE) {
            // if it's not who we wanted, tell them no thanks
            int say = LSE_NO_THANKS;
            int ierr = MPI_Send(&say, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            //printf(" + %d had to tell %d that it wasn't to be\n", me->id, status.MPI_SOURCE);
            
         } else {
            //printf("! WARNING UNHANDLED CASE %d got a %d from %d\n", me->id, recMes, status.MPI_SOURCE);
            //printf("! NO THANKS WAS EXPECTING %d\n", me->waitingFor);
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
               usleep(SENIOR_TIMEOUT_PAUSE);
            }
            // go and wait for message i.e. goto start of loop
         }
      }
   }

   announceExchange(me);
}