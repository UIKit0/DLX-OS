#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the  process that we're done
  sem_t s_H2O;          //Semaphore of H2O
  int num_H2O;          //Number of H2O
  int i;               //index var
  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_signal_complete><s_H2O><num_H2O>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  s_H2O =dstrtol(argv[2],NULL,10);
  num_H2O =dstrtol(argv[3],NULL,10);
 
  for(i=0;i<num_H2O;i++){
     Printf("Inject a H2O\n");
      sem_signal(s_H2O);
  }

 // Printf("injection_h2o completed!");
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
