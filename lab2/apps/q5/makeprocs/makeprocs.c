#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int num_N2 ;               // number of N2 injection
  int num_H2O;                //number of H2O injection
  int num_N2_2N;               //number of reaction 1
  int num_H2O_H2_O2;            //number of reaction 2
  int num_N_O2_NO2;             //number of reaction 3
  int number_N2;                //number of N2 molecules
  int number_H2O;               //number of H2O molecules
  int numprocs=5;                //number of process

  sem_t s_N2;             //semarphore used to indicate number of N2 in the reaction
  sem_t s_N;           //semarphore used to indicate number of N in the reaction
  sem_t s_H2O;         //semarphore used to indicate number of H2O in the reactionn
  sem_t s_H2;         //semarphore used to indicate number of H2 in the reaction
  sem_t s_O2;        //semarphore used to indicate number of o2 in the reaction
  sem_t s_NO2;       //semarphore used to indicate number of NO2 in the reaction
  sem_t s_procs_completed; //semaphore used to indicate complete process

  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processess
  char s_N2_str[10];        //Used as command-line argument to pass N2 semaphore
  char s_N_str[10];      //Used as command-line argument to pass N Semarphore
  char s_H2O_str[10];    //Used as command-line argument to pass H2O semarphore
  char s_H2_str[10];     //Used as command-line argument to pass H2  semarphore
  char s_O2_str[10];     //Used as command-line argument to pass O2  semaphore
  char s_NO2_str[10];    //Used as command-line argument to pass N02 semaphore
  char num_N2_str[10];   //Used as command_line argument to pass N2 
  char num_H2O_str[10];   //Used as command_line argument to pass H2O   
  char num_N2_2N_str[10];   //Used as command_line argument to pass reaction1   
  char num_H2O_H2_O2_str[10];   //Used as command_line argument to pass reaction2   
  char num_N_O2_NO2_str[10];   //Used as command_line argument to pass reaction3   
 if (argc != 3) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of N2 molecules><number of H20 molecules\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  number_N2 = dstrtol(argv[1], NULL, 10); // the "10" means base 10
//  Printf("Inject %d N2\n", number_N2);
  number_H2O = dstrtol(argv[2],NULL, 10); 
//  Printf("Inject %d H2O\n",number_H2O);

  if(number_H2O%2==1){
     num_H2O = number_H2O; 
     num_H2O_H2_O2 = (number_H2O-1)/2;
     num_N2 = number_N2;
     num_N2_2N = number_N2;
     if (2*num_N2>num_H2O_H2_O2){     //number of N larger than number of O2
        num_N_O2_NO2 = num_H2O_H2_O2;
     }
     else{
         num_N_O2_NO2 = 2*num_N2;
     } 
   }
   else {
     num_H2O = number_H2O;
     num_H2O_H2_O2=(number_H2O)/2;
     num_N2 = number_N2;
     num_N2_2N = number_N2;
     if (2*num_N2>num_H2O_H2_O2){   //number of N larger than number of O2
        num_N_O2_NO2 = num_H2O_H2_O2;
     }
     else{
        num_N_O2_NO2 = 2*num_N2;
     }
   }
 // Printf("num_H2O:%d  num_H2O_H2_O2: %d  num_N2: %d  num_N2_2N: %d  num_N_O2_NO2: %d\n",num_H2O,num_H2O_H2_O2,num_N2,num_N2_2N,num_N_O2_NO2);    
    
  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  //create six semaphores representing six chemicals in reaction
  if ((s_N2=sem_create(0))== SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((s_H2O=sem_create(0))==SYNC_FAIL){
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((s_N=sem_create(0))==SYNC_FAIL){
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((s_H2=sem_create(0))==SYNC_FAIL){
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((s_O2=sem_create(0))==SYNC_FAIL){
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if ((s_NO2=sem_create(0))==SYNC_FAIL){
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(s_N2, s_N2_str);
  ditoa(s_H2O,s_H2O_str);
  ditoa(s_N,s_N_str);
  ditoa(s_H2,s_H2_str);
  ditoa(s_O2,s_O2_str);
  ditoa(s_NO2,s_NO2_str);
  ditoa(num_N2, num_N2_str);
  ditoa(num_H2O, num_H2O_str);
  ditoa(num_H2O_H2_O2, num_H2O_H2_O2_str);
  ditoa(num_N2_2N, num_N2_2N_str);
  ditoa(num_N_O2_NO2, num_N_O2_NO2_str);
 
  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
    process_create(INJECTION_N2,s_procs_completed_str,s_N2_str,num_N2_str, NULL);
    process_create(INJECTION_H2O,s_procs_completed_str,s_H2O_str,num_H2O_str, NULL);
    process_create(REACTION1,s_procs_completed_str,s_N2_str,s_N_str,num_N2_2N_str, NULL);
    process_create(REACTION2,s_procs_completed_str,s_H2O_str,s_H2_str,s_O2_str,num_H2O_H2_O2_str, NULL);
    process_create(REACTION3,s_procs_completed_str,s_N_str,s_O2_str,s_NO2_str,num_N_O2_NO2_str, NULL);

   // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
