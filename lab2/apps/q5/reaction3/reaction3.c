#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the process that we're done
  sem_t s_N;         //Semaphore N
  sem_t s_O2;          //Semaphore O2
  sem_t s_NO2;        //Semaphore NO2
  int num_N_O2_NO2;       //number of reaction
  int i;             //index var

  if (argc != 6) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_signal_complete><s_H2O> <s_H2> <s_O2>  <num_H2O_H2_O2> \n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  s_N =dstrtol(argv[2],NULL,10);
  s_O2 =dstrtol(argv[3],NULL,10);
  s_NO2 =dstrtol(argv[4],NULL,10);
  num_N_O2_NO2 = dstrtol(argv[5],NULL,10);

  for(i=0;i<num_N_O2_NO2;i++){
      sem_wait(s_N);
      sem_wait(s_O2);
      Printf("Create a NO2\n");
      sem_signal(s_NO2);
  }
     
//  Printf("reaction3 completed\n");
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
}
}
