#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the process that we're done
  sem_t s_N2;          //Semaphore N2
  sem_t s_N;        //Semaphore N
  int num_N2_2N;       //number of N2
  int i;           //index var
  
  if (argc != 5) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_signal_complete><s_N2> <s_N> <num_N2> \n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  s_N2 =dstrtol(argv[2],NULL,10);
  s_N =dstrtol(argv[3],NULL,10);
  num_N2_2N = dstrtol(argv[4],NULL,10);
  
  for(i=0;i<num_N2_2N;i++){
     sem_wait(s_N2);
     sem_signal(s_N);
     sem_signal(s_N);
  }
//  Printf("reaction1 completed!\n");
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
}
}
