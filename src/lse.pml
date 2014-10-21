#include "lse-pml.h"

#define N_SENIORS  6
#define DEATH_PROB 0.1

proctype senior(int id) {
   printf("Spawned senior %d\n", id);
}

init {
   int i = 0
   do
   :: (i <  N_SENIORS) -> run senior(i); i++;
   :: (i == N_SENIORS) -> break;
   od
}