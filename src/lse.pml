#include "lse-pml.h"

#define N_SENIORS  6
#define DEATH_PROB 0.1

#define TRUE        1
#define FALSE       0
#define MAYBE_LATER 2

#define LSE_NO_MESSAGE           -1
#define LSE_I_WANT_TO_EXCHANGE   0
#define LSE_THAT_SOUNDS_GREAT    1
#define LSE_NO_THANKS            2

#define NO_ONE -1 

chan c[N_SENIORS] = [16] of { int,  int  }
                           /* what, who */

inline pickRandomSenior () {
   do
   :: friend = (friend + 1)%N_SENIORS;
   :: break;
   od
   /* catch trying to communicate with self */
   if
   :: (friend == id)   -> friend = (friend + 1)%N_SENIORS;
   :: else             -> skip;
   fi

   assert(friend >= 0);
   assert(friend != id);
   assert(friend < N_SENIORS);
}

inline handleIncoming () {
   if 
   :: (waitingFor == from && theySaid == LSE_THAT_SOUNDS_GREAT) 
      ->
         pairedWith = waitingFor;
   :: (waitingFor == from && theySaid == LSE_I_WANT_TO_EXCHANGE)
      ->
         pairedWith = waitingFor;
   :: (waitingFor == from && theySaid == LSE_NO_THANKS)
      ->
         myCompat[waitingFor] = MAYBE_LATER;
         waitingFor = NO_ONE;
   :: (waitingFor == NO_ONE && theySaid == LSE_I_WANT_TO_EXCHANGE)
      ->
         c[from]!LSE_THAT_SOUNDS_GREAT, id;
         waitingFor = from;
         pairedWith = waitingFor;
   :: (waitingFor != from && theySaid == LSE_I_WANT_TO_EXCHANGE && waitingFor != NO_ONE)
      ->
         c[from]!LSE_NO_THANKS, id;
   :: else
      -> 
         skip;
         /* this happens sometimes but I don't know why */
         /* assert(false); */
   fi
}

inline handleNoIncoming () {
   if
   :: (waitingFor == NO_ONE)  -> /* pick friend and ask */
   :: else                    -> /* timout */
   fi
}

inline askPotentialMatch () {
   /* pick a random senior {*/
      int friend = 0;
      pickRandomSenior();
      waitingFor = friend;
   /* } pick a random senior*/

   /* communicate { */
      c[waitingFor]!LSE_I_WANT_TO_EXCHANGE, id;
   /*} communicate */
}

active [4] proctype senior(int id) {
   /* init */
   int myCompat[N_SENIORS];
   int pairedWith = NO_ONE;
   int waitingFor = NO_ONE;
   int souped = FALSE;
   int waitTimer = 0;

   askPotentialMatch();

   /* recieve { */
   int theySaid = LSE_NO_MESSAGE;
   int from = NO_ONE;
   if
   :: c[id]?[theySaid,from] ->   c[id]?theySaid,from;
                                 printf("I, %d was told %d by %d\n", id, theySaid, from);
                                 handleIncoming();
   :: else                 ->    printf("I, %d, have no message\n", id);
                                 /* communicate or recieve */
   fi
   /* } recieve */

   announceDeath();
}
/*
init {
   int i = 0
   do
   :: (i <  N_SENIORS) -> run senior(i); i++;
   :: (i == N_SENIORS) -> break;
   od
}*/