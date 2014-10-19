#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>

#define TRUE  1
#define FALSE 0

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

void seniorMatch (struct myInfo *me) {
   int ierr;

   // just tell the first person you are compatible with that you wanna chat
   int i = 0;
   while (me->compat[i] == FALSE) {
      i++;
   }
   int message = LSE_I_WANT_TO_EXCHANGE;
   ierr = MPI_Send(&message, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
   printf(" + %d + told %d that LSE_I_WANT_TO_EXCHANGE\n", me->id, i);
   me->waitingFor = i;

   me->waitTimer = 5;
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
      } else {
         // no message
         if (me->waitingFor == NO_ONE) {
            // formulate message
         } else {
            printf(" + %d I'm waiting for someone to message me\n", me->id);
            me->waitTimer--;
            if (me->waitTimer == 0) {
               printf(" + %d I think someone died\n", me->id);
               announceVegetation(me);
               me->pairedWith = me->id;
            } else {
               usleep(100);
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
      struct myInfo me;
      ierr = MPI_Comm_rank(MPI_COMM_WORLD, &me.id);
      me.pairedWith = NO_ONE;
      me.compat = malloc(sizeof(int)*numSeniors);
      me.waitingFor = NO_ONE;

      i = 0;
      while (i < numSeniors) {
         if (compatibility[me.id][i] == TRUE) {
            me.compat[i] = TRUE;
         }

         i++;
      }

      printf(" + %d got my rank\n", me.id);      

      if (souped[me.id] == TRUE) {
         printf("I, %d, ate the soup!\n", me.id);
         //announceDeath(me.id);
      }

      seniorMatch(&me);

      free(me.compat);
   }

   ierr = MPI_Finalize();
}