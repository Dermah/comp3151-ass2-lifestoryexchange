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

   // make a list of compatible seniors that haven't
   // rejected us in recently
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

      // stop ignoring seniors that have rejected us recently
      // and pick one to exchange with
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
         // all my compatible seniors are dead or taken
         announceVegetation(me);
      } else {
         // i got overincremented in the loop above
         i--;
      }

      iPick = i;
   } else {
      // found seniors that we could talk to, pick one randomly
      int pick = (rand() % numAlive);
      iPick = alive[pick];
   }
   
   return iPick;
}

void askPotentialMatch (struct senior *me) {
   // check if we die from musroom soup before sending a message
   if (me->souped == TRUE) {
      float roll = (float)rand()/(float)(RAND_MAX);
      if (roll <= me->deathProb) {
         announceDeath(me);
      }
   }
   int match = pickRandomSenior(me);

   // ask our potential match if they want to exchange
   // and set a timeout for them to reply by
   int message = LSE_I_WANT_TO_EXCHANGE;
   int ierr = MPI_Send(&message, 1, MPI_INT, match, 0, MPI_COMM_WORLD);
   me->waitingFor = match;
   me->waitTimer = SENIOR_TIMEOUT_CHECKS;
   assert(me->id != me->waitingFor);
}

void seniorMatch (struct senior *me) {
   // main negotiation loop
   int ierr;

   askPotentialMatch(me);   

   while (me->pairedWith == NO_ONE) {
      
      // check if there is a message waiting
      int recieved = FALSE;
      ierr = MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recieved, MPI_STATUS_IGNORE);
      if (recieved == TRUE) {
         // open the message
         int theySaid = LSE_NO_MESSAGE;
         MPI_Status status;
         ierr = MPI_Recv(&theySaid, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
         
         // decide what to do
         if (status.MPI_SOURCE == me->waitingFor && theySaid == LSE_THAT_SOUNDS_GREAT) {
            // if it's who we want to hear from
            // They accepted our request! 
            // Announce
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE == me->waitingFor && theySaid == LSE_I_WANT_TO_EXCHANGE) {
            // if it's who we want to hear from
            // They want to exchange with us! And we've already asked them if they want to exchange.
            //      They obviously haven't recieved that request yet, but they will eventually...
            // Announce
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE == me->waitingFor && theySaid == LSE_NO_THANKS) {
            // if it's who we want to hear from and they reject us
            me->compat[me->waitingFor] = MAYBE_LATER;
            me->waitingFor = NO_ONE;
         } else if (me->waitingFor == NO_ONE && theySaid == LSE_I_WANT_TO_EXCHANGE) {
            // we weren't expecting to hear from anyone so let's accept the offer
            int reply = LSE_THAT_SOUNDS_GREAT;
            int ierr = MPI_Send(&reply, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            me->waitingFor = status.MPI_SOURCE;
            // Announce
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE != me->waitingFor && theySaid == LSE_I_WANT_TO_EXCHANGE) {
            // if it's not who we wanted, tell them no thanks and continue waiting
            int reply = LSE_NO_THANKS;
            int ierr = MPI_Send(&reply, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
         } else {
            // None of the cases above were fulfilled
            // printf("! WARNING UNHANDLED CASE %d got a %d from %d\n", me->id, theySaid, status.MPI_SOURCE);
            // printf("! WAS EXPECTING REPLY FROM %d\n", me->waitingFor);
         }
      } else {
         // no message waiting for us
         if (me->waitingFor == NO_ONE) {
            askPotentialMatch(me);
         } else {
            // waiting for a reply but they haven't got back to us
            me->waitTimer--;
            if (me->waitTimer == 0) {
               // reponse from potential match timed out. Either they died or they announced already
               me->compat[me->waitingFor] = FALSE;
               me->waitingFor = NO_ONE;
            } else {
               usleep(SENIOR_TIMEOUT_PAUSE);
            }
         }
      }
   }

   // if you got to here, you've been paired with some one
   // tell the world the good news
   announceExchange(me);
}