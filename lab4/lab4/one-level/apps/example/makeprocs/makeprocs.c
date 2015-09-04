#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD_0 "hello_world_0.dlx.obj"
#define HELLO_WORLD_1 "hello_world_1.dlx.obj"
#define HELLO_WORLD_2 "hello_world_2.dlx.obj"
#define HELLO_WORLD_3 "hello_world_3.dlx.obj"
#define HELLO_WORLD_4 "hello_world_4.dlx.obj"
#define HELLO_WORLD_5 "hello_world_5.dlx.obj"

void main (int argc, char *argv[])
{
  int num_hello_world = 0;             // Used to store number of processes to create
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  sem_t test_5;
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
  char test_5_str[10];
  if (argc != 2) {
    Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
    Exit();
  }

  // Convert string from ascii command line argument to integer number
  num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("makeprocs (%d): Creating %d test cases\n", getpid(), num_hello_world);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }
  if ((test_5 = sem_create(-29)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);
  ditoa(test_5,test_5_str);
  // Create Hello World processes
//  Printf("-------------------------------------------------------------------------------------\n");
//  Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
//  for(i=0; i<num_hello_world; i++) {
    Printf("makeprocs (%d): Creating Test case #0\n", getpid());
    process_create(HELLO_WORLD_0, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("makeprocs (%d): Creating Test case #1\n", getpid());
    process_create(HELLO_WORLD_1, s_procs_completed_str, NULL);
  //  sem_signal(s_procs_completed);
 //   if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
 //     Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
 //     Exit();
 //   }
    for(i=0;i<10000;i++);//loop for time to display error info
    Printf("Test case (1): Done!\n");
    Printf("------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating Test case #2\n", getpid());
    process_create(HELLO_WORLD_2, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("makeprocs (%d): Creating Test case #3\n", getpid());
    process_create(HELLO_WORLD_3, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("makeprocs (%d): Creating Test case #4\n", getpid());
    for(i=0;i<100;i++){
    process_create(HELLO_WORLD_4, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("running hello_world program %d times\n",i+1);
   }
    Printf("Test case (4): Done!\n");
    Printf("------------------------------------------------------------------------------------\n");
    Printf("makeprocs (%d): Creating Test case #5\n", getpid());
    for(i=0;i<30;i++){
    process_create(HELLO_WORLD_5, test_5_str, NULL);
    }
    if (sem_wait(test_5) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
    Printf("Test case (5):Done!\n");
    Printf("-------------------------------------------------------------------------------------\n"); 
//  }

  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
