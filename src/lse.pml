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

chan c[N_SENIORS] = [16] of { int }

proctype senior(int id) {
   c[(id+1)%N_SENIORS]!LSE_I_WANT_TO_EXCHANGE;
   int x = LSE_NO_MESSAGE;
   c[id]?x
   //printf("%d got a %d\n", id, x)

   int i = 0;
   do
   :: (skip) -> i++;
   :: (skip) -> i++;
   :: (skip) -> i++;
   :: (skip) -> break;
   od

   // catch trying to communicate with self
   if
   :: (i == id)        -> i = (i + 1)%N_SENIORS;
   :: (i >= N_SENIORS) -> i = N_SENIORS -1;
   :: else             -> skip;
   fi

   printf("%d chose %d\n", id, i);
   announceDeath();
}

init {
   int i = 0
   do
   :: (i <  N_SENIORS) -> run senior(i); i++;
   :: (i == N_SENIORS) -> break;
   od
}