#define TRUE        1
#define FALSE       0
#define MAYBE_LATER 2

#define LSE_I_WANT_TO_EXCHANGE 0
#define LSE_THAT_SOUNDS_GREAT  1
#define LSE_NO_THANKS          2

#define NO_ONE -1 

struct senior {
   int id;
   int *compat; //of size numSeniors
   int waitingFor;
   int pairedWith;
   int waitTimer;
   int numSeniors;
   int souped;
   float deathProb;
};

int devrand(int high);

void announceDeath (struct senior *me);

void announceExchange (int i, int them);

void announceVegetation (struct senior *me);

int pickRandomSenior (struct senior *me);

void askPotentialMatch (struct senior *me);

void seniorMatch (struct senior *me);