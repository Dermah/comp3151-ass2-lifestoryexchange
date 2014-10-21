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
//                            what, who

inline pickFriend () {
   do
   :: friend = (friend + 1)%N_SENIORS;
   :: break;
   od
   // catch trying to communicate with self
   if
   :: (friend == id)   -> friend = (friend + 1)%N_SENIORS;
   :: else             -> skip;
   fi
}

proctype senior(int id) {
   // communicate
   //c[(id+1)%N_SENIORS]!LSE_I_WANT_TO_EXCHANGE;
   //int x = LSE_NO_MESSAGE;
   //c[id]?x
   //printf("%d got a %d\n", id, x)


   // pick a random senior {
      int friend = 0;
      pickFriend();
   // } pick a random senior

   // communicate {
      c[friend]!LSE_I_WANT_TO_EXCHANGE, id;
   //} communicate

   // recieve {
      int message = LSE_NO_MESSAGE;
      int who = NO_ONE;
      if
      :: c[id]?[message,who] -> c[id]?message,who;
                                printf("I, %d was told %d by %d\n", id, message, who);
      :: else                -> printf("I, %d, have no message\n", id);
      fi
      
   // } recieve
   assert(friend >= 0);
   assert(friend != id);
   assert(friend < N_SENIORS);
   announceDeath();
}

init {
   int i = 0
   do
   :: (i <  N_SENIORS) -> run senior(i); i++;
   :: (i == N_SENIORS) -> break;
   od
}