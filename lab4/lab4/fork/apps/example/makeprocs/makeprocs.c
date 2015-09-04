#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"

void main (int argc, char *argv[])
{
  int num_hello_world = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  int child_pid;
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 2) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(-(num_hello_world-1))) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  // Create Hello World processes
/*  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
  for(i=0; i<num_hello_world; i++) {
    Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
    process_create(HELLO_WORLD, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
  }
*/
//---------------------------------------this section for testing process_fork()--------------------------------------------------
   printf("the main program process ID is %d\n",(int)getpid());
   child_pid=process_fork();
   if(child_pid!=0){
      Printf ("this is the parent process with id %d\n",(int)getpid());
      Printf ("the child's process ID is %d\n",child_pid);
   }else {
      Printf ("this is the child process,with id %d\n",(int)getpid());
   }

  Printf("makeprocs (%d): exiting main process.\n", getpid());
  Printf("-------------------------------------------------------------------------------------\n");

}
