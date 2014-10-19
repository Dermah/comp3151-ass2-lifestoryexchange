#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <mpi.h>

#define TRUE        1
#define FALSE       0
#define MAYBE_LATER 2

#define LSE_I_WANT_TO_EXCHANGE 0
#define LSE_THAT_SOUNDS_GREAT  1
#define LSE_NO_THANKS          2

#define NO_ONE -1 

struct myInfo {
   int id;
   int *compat; //of size numSeniors
   int waitingFor;
   int pairedWith;
   int waitTimer;
   int numSeniors;
};

// come up with a random number between 0 and num-1
int devrand(int high) {
   FILE *fp = fopen("/dev/urandom", "r");

   int num;
   fread(&num, sizeof(int), 1, fp);
   fclose(fp);
   
   return abs((num%high));
}

void announceDeath (struct myInfo *me) {
   printf("%d dies blaming the mushroom soup.\n", me->id+1);
   int ierr = MPI_Finalize();
   exit(0);
}

void announceExchange(int i, int them) {
   printf("%d exchanges life stories with %d.\n", i+1, them+1);
}

void announceVegetation(struct myInfo *me) {
   printf("%d has a seniors’ moment.\n", me->id+1);
   int ierr = MPI_Finalize();
   exit(0);
}

void recieveMessage (struct myInfo *me) {
   int ierr;
   int recieve = FALSE;
   MPI_Status status;
   while (!recieve) {
      ierr = MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recieve, &status);
   }
   if (recieve) {
      printf(" + %d + there is a message waiting from %d\n", me->id, status.MPI_SOURCE);
      int recMes = 3892523984;
      ierr = MPI_Recv(&recMes, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
      printf(" + %d + recieved a %d from %d\n", me->id, recMes, status.MPI_SOURCE);
      if (me->compat[status.MPI_SOURCE] == TRUE) {
         printf(" + %d + I AM COMPATIBLE WITH %d\n", me->id, status.MPI_SOURCE);
         me->pairedWith = TRUE;
         printf("%d PAIR3D %d\n", me->id, me->pairedWith);
      }
      
   } else {

   }
}

int pickRandomSenior (struct myInfo *me) {
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
      printf(" = %d there are %d people that I can talk to\n", me->id, numAlive);
      printf(" + %d I pick %d\n", me->id, alive[pick]);
      iPick = alive[pick];
   }
   
   return iPick;
}

void askPotentialMatch (struct myInfo *me) {
   int i = pickRandomSenior(me);

   int message = LSE_I_WANT_TO_EXCHANGE;
   int ierr = MPI_Send(&message, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
   printf(" + %d + told %d that LSE_I_WANT_TO_EXCHANGE\n", me->id, i);
   me->waitingFor = i;
   me->waitTimer = 5;
   assert(me->id != me->waitingFor);
   printf("- %d waiting for %d\n", me->id, me->waitingFor);
}

void seniorMatch (struct myInfo *me) {
   int ierr;

   askPotentialMatch(me);   

   // while (not matched) {
   while (me->pairedWith == NO_ONE) {
      // if message waiting
      int recieved = FALSE;
      ierr = MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &recieved, MPI_STATUS_IGNORE);
      if (recieved == TRUE) {
         printf(" + %d got a message\n", me->id);
         
         // get message
         int recMes = 3892523984;
         MPI_Status status;
         ierr = MPI_Recv(&recMes, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
         
         // decide what to do

         if (status.MPI_SOURCE == me->waitingFor && recMes == LSE_THAT_SOUNDS_GREAT) {
            // if it's who we want to hear from
            printf(" + %d SHE SAID YES %d\n", me->id, me->waitingFor);
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE == me->waitingFor && recMes == LSE_I_WANT_TO_EXCHANGE) {
            // if it's who we want to hear from
            printf(" + %d SHE WANTS ME TOO %d\n", me->id, me->waitingFor);
            me->pairedWith = me->waitingFor;
         } else if (status.MPI_SOURCE == me->waitingFor && recMes == LSE_NO_THANKS) {
            // if it's who we want to hear from and they say no
            me->compat[me->waitingFor] = MAYBE_LATER;
            me->waitingFor = NO_ONE;
            printf(" + %d REJECTED\n", me->id );
         } else if (status.MPI_SOURCE != me->waitingFor && recMes == LSE_I_WANT_TO_EXCHANGE) {
            // if it's not who we wanted, tell them no thanks
            int say = LSE_NO_THANKS;
            int ierr = MPI_Send(&say, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            printf(" + %d had to tell %d that it wasn't to be\n", me->id, status.MPI_SOURCE);
         } else {
            printf("! WARNING UNHANDLED CASE %d got a %d from %d\n", me->id, recMes, status.MPI_SOURCE);
         }
      } else {
         // no message
         if (me->waitingFor == NO_ONE) {
            askPotentialMatch(me);
         } else {
            printf(" + %d I'm waiting for %d to message me (timeout %d)\n", me->id, me->waitingFor, me->waitTimer);
            me->waitTimer--;
            if (me->waitTimer == 0) {
               printf(" + %d I think %d died\n", me->id, me->waitingFor);
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

int main(int argc, char **argv)
{
   if (argc == 1) {
      fprintf(stderr, "No arguments given\n");
      exit(1);
   }

   char *progName = argv[0];
   char *fileName = argv[1];
   float deathProb = atof(argv[2]);

   FILE *fp;
   fp = fopen(fileName, "r");

   if (fp == NULL) {
      fprintf(stderr, "Could not open file %s\n", argv[0]);
      exit(1);
   }

   int numSeniors;
   fscanf(fp, "%d\n", &numSeniors);

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

   int ierr;
   ierr = MPI_Init(&argc, &argv);
   
   {
      struct myInfo *me = malloc(sizeof(struct myInfo));
      me->compat = malloc(sizeof(int)*numSeniors);

      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &me->id);
      me->pairedWith = NO_ONE;
      me->waitingFor = NO_ONE;
      me->numSeniors = numSeniors;

      i = 0;
      while (i < numSeniors) {
         if (compatibility[me->id][i] == TRUE) {
            me->compat[i] = TRUE;
         } else {
            me->compat[i] = FALSE;
         }

         i++;
      }


      printf(" + %d got my rank\n", me->id);      

      if (souped[me->id] == TRUE) {
         printf("I, %d, ate the soup!\n", me->id);
         //announceDeath(me->id);
      }

      seniorMatch(me);

      free(me->compat);
      free(me);
   }

   ierr = MPI_Finalize();
}