#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"


void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the process that we're done
  sem_t s_N2;          //Semaphore as N2
  int num_N2;          //Number of N2
  int i;              //index var
  if (argc != 4) { 
    Printf("Usage: "); Printf(argv[0]); Printf("  <handle_to_pass_complete_signal><s_procs_completed_str> <s_N2_str> <num_N2_str>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);
  s_N2 = dstrtol(argv[2],NULL,10);
  num_N2 =dstrtol(argv[3],NULL,10);
  
  for(i=0;i<num_N2;i++){
     Printf("Inject a N2\n");
     sem_signal(s_N2);
  }
  // Signal the semaphore to tell the original process that we're done
 //  Printf("injection_n2 completed!\n");
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
