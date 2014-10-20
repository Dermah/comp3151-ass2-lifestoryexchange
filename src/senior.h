#define TRUE        1
#define FALSE       0
#define MAYBE_LATER 2

#define LSE_I_WANT_TO_EXCHANGE 0
#define LSE_THAT_SOUNDS_GREAT  1
#define LSE_NO_THANKS          2

#define NO_ONE -1 

// total timeout for a senior is 
// SENIOR_TIMEOUT_CHECKS * SENIOR_TIMEOUT_PAUSE useconds
#define SENIOR_TIMEOUT_CHECKS 5
#define SENIOR_TIMEOUT_PAUSE  6000

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

// initialises the senior struct and calls MPI_Init
struct senior* senior_init(int numSeniors, int argc, char **argv);

// frees memory and calls MPI_Finalize
void senior_finalise(struct senior *me);

// generate a random number from 0 to high-1 using /dev/urandom
int devrand(int high);

// announce the senior's death from food poisoning. calls senior_finalise
void announceDeath (struct senior *me);

// announce exchange with another senior. senior should not do much else after this
void announceExchange (struct senior *me);

// announce a period of vegetative thought. calls senior_finalise
void announceVegetation (struct senior *me);

// pick a random senior that this senior is compatible with
int pickRandomSenior (struct senior *me);

// send a message to a compatible senior asking to LSE with them
void askPotentialMatch (struct senior *me);

// decide to LSE with another senior, think vegetatively, or die
void seniorMatch (struct senior *me);