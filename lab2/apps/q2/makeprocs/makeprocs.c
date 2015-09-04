#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs = 0;               // Used to store number of processes to create
  int i;                          // Loop index variable
  circular_buffer *mc;             // Used to get address of shared memory page
  uint32 h_mem;                   // Used to hold handle to shared memory page
  sem_t s_procs_completed_pc;        // Semaphore used to wait until  spawned producer_consumer processes have completed
  lock_t lock;                       //created lock 
  char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
  char s_procs_completed_str_pc[10]; // Used as command-line argument to pass page_mapped handle to new producer_consumer processess
  char id_str[10];                 //Used as command-line argument to pass process id to new producer and consumer
  char lock_str[10];               //Used as command-line argument to pass lock id
  char producer[10];               //Used as symbol that the process is producer
  char consumer[10];              //Used as symbol that the process is consumer
 if (argc != 2) {
    Printf("Usage: "); Printf(argv[0]); Printf(" <number of processes to create>\n");
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  numprocs = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("Creating %d producer processes and %d consumer processes\n", numprocs,numprocs);

  // Allocate space for a shared memory page, which is exactly 64KB
  // Note that it doesn't matter how much memory we actually need: we 
  // always get 64KB
  if ((h_mem = shmget()) == 0) {
    Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }

  // Map shared memory page into this process's memory space
  if ((mc = (circular_buffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
    Exit();
  }

  // Initial the head and tail
  mc->head=0;
  mc->tail=0;
  // Create a lock
  if((lock=lock_create())==SYNC_FAIL){
      Printf("Fail to create a lock");Exit(); 
     }
  else{Printf("lock= %d\n",lock);}

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed_pc = sem_create(-(numprocs*2-1))) == SYNC_FAIL) {
    Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
    Exit();
  }
  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(h_mem, h_mem_str);
  ditoa(s_procs_completed_pc, s_procs_completed_str_pc);
  ditoa(lock,lock_str);
  ditoa(PRODUCER,producer);
  ditoa(CONSUMER,consumer);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<numprocs; i++) {
    ditoa(i,id_str);
    process_create(PRODUCER_CONSUMER, h_mem_str, s_procs_completed_str_pc,id_str,lock_str,producer, NULL);
  //  Printf("Producer_Consumer Process %d created\n", i);
    process_create(PRODUCER_CONSUMER, h_mem_str, s_procs_completed_str_pc,id_str,lock_str,consumer, NULL);
  }


  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed_pc) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed_pc (%d) in ", s_procs_completed_pc); Printf(argv[0]); Printf("\n");
    Exit();
  }
  Printf("All other processes completed, exiting main process.\n");
}
