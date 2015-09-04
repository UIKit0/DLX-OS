#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int numprocs;               // Used to store number of processes to create
  int i;                          // Loop index variable
  missile_code mc;                // Used as message for mailbox
  
  int num_N2;                     //numer of N2 injection
  int num_H2O;                    //number of H2O injection
  int num_N2_2N;                  //number of reaction1
  int num_H2O_H2_O2;              //number of reaction2
  int num_N_O2_NO2;               //number of reaction3
  int number_N2;                  //number of N2 molecules
  int number_H2O;                 //number of H2O molecules

  mbox_t mbox_N2;                  //N2 mailbox
  mbox_t mbox_N;                   //N mailbox
  mbox_t mbox_H2O;                 //H2O mailbox
  mbox_t mbox_H2;                  //H2 mailbox
  mbox_t mbox_O2;                  //O2 mailbox
  mbox_t mbox_NO2;                 //NO2 mailbox
  
  sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
  char mbox_N2_str[10];            // Used as command-line argument to pass mem_handle to new processes
  char mbox_N_str[10];
  char mbox_H2O_str[10];
  char mbox_H2_str[10];
  char mbox_O2_str[10];
  char mbox_NO2_str[10];
  char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 3) {
    Printf("Usage: ");Printf(argv[0]);Printf("<number of N2 molecules> <number of H2O molecules\n");
    Exit();
  }
  
  // Convert string from ascii command line argument to integer number
  number_N2 = dstrtol(argv[1],NULL,10); //the "10" means base 10
  number_H2O = dstrtol(argv[2],NULL,10);
  
  if(number_H2O%2 ==1){
     num_H2O = number_H2O;
     num_H2O_H2_O2 = (number_H2O-1)/2;
     num_N2 = number_N2;
     num_N2_2N = number_N2;
     if (2*num_N2>num_H2O_H2_O2){    //number of N larger than number of O2
         num_N_O2_NO2 = num_H2O_H2_O2;
     }
     else{
         num_N_O2_NO2 = 2*num_N2;
     }
   }
  else{
     num_H2O = number_H2O;
     num_H2O_H2_O2 =(number_H2O)/2;
     num_N2 = number_N2;
     num_N2_2N = number_N2;
     if (2*num_N2>num_H2O_H2_O2){  //number of N larger than number of O2
        num_N_O2_NO2 = num_H2O_H2_O2;
     }
     else{
         num_N_O2_NO2 = 2*num_N2;
     }
   }
   
   numprocs=num_H2O+num_N2+num_H2O_H2_O2+num_N2_2N+num_N_O2_NO2;

  // Allocate space for a mailbox
  if ((mbox_N2 = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }

  if ((mbox_N = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }

  if ((mbox_O2 = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }
  
  if ((mbox_NO2 = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }

  if ((mbox_H2O = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }

  if ((mbox_H2 = mbox_create()) == MBOX_FAIL) {
    Printf("makeprocs (%d): ERROR: could not allocate mailbox!", getpid());
    Exit();
  }

  // Open mailbox to prevent deallocation
  if (mbox_open(mbox_N2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), mbox_N2);
    Exit();
  }

  if (mbox_open(mbox_N) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), mbox_N);
    Exit();
  }

  if (mbox_open(mbox_H2O) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), mbox_H2O);
    Exit();
  }

  if (mbox_open(mbox_H2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), mbox_H2);
    Exit();
  }
  if (mbox_open(mbox_O2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), mbox_O2);
    Exit();
  }
  if (mbox_open(mbox_NO2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not open mailbox %d!\n", getpid(), mbox_NO2);
    Exit();
  }
  // Put some values in the mc structure to send as a message
  mc.numprocs = numprocs;
  mc.really_important_char = 'A';

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.  To do this, we will initialize
  // the semaphore to (-1) * (number of signals), where "number of signals"
  // should be equal to the number of processes we're spawning - 1.  Once 
  // each of the processes has signaled, the semaphore should be back to
  // zero and the final sem_wait below will return.
  if ((s_procs_completed = sem_create(-(numprocs-1))) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new process.  We're going to
  // pass the handles to the shared memory page and the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(mbox_N2, mbox_N2_str);
  ditoa(mbox_H2O, mbox_H2O_str);
  ditoa(mbox_N, mbox_N_str);
  ditoa(mbox_H2, mbox_H2_str);
  ditoa(mbox_O2, mbox_O2_str);
  ditoa(mbox_NO2, mbox_NO2_str);
  ditoa(s_procs_completed, s_procs_completed_str);

  // Now we can create the processes.  Note that you MUST end your call to
  // process_create with a NULL argument so that the operating system
  // knows how many arguments you are sending.
  for(i=0; i<num_N2; i++) {
    process_create(FILENAME_TO_RUN_0, 0, 0, mbox_N2_str,s_procs_completed_str, NULL);
 //   Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<num_H2O; i++) {
    process_create(FILENAME_TO_RUN_1, 0, 0, mbox_H2O_str, s_procs_completed_str, NULL);
//    Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<num_N2_2N; i++) {
    process_create(FILENAME_TO_RUN_2, 0, 0, mbox_N2_str,mbox_N_str, s_procs_completed_str, NULL);
//    Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<num_H2O_H2_O2; i++) {
    process_create(FILENAME_TO_RUN_3, 0, 0, mbox_H2O_str,mbox_H2_str,mbox_O2_str, s_procs_completed_str, NULL);
//    Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }
  for(i=0; i<num_N_O2_NO2; i++) {
    process_create(FILENAME_TO_RUN_4, 0, 0, mbox_N_str,mbox_O2_str,mbox_NO2_str, s_procs_completed_str, NULL);
//    Printf("makeprocs (%d): Process %d created\n", getpid(), i);
  }

  // And finally, wait until all spawned processes have finished.
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
    Exit();
  }
  if (mbox_close(mbox_N2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not close mailbox %d!\n", getpid(), mbox_N2);
    Exit();
  }
  if (mbox_close(mbox_N) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not close mailbox %d!\n", getpid(), mbox_N);
    Exit();
  }
  if (mbox_close(mbox_O2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not close mailbox %d!\n", getpid(), mbox_O2);
    Exit();
  }
  if (mbox_close(mbox_NO2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not close mailbox %d!\n", getpid(), mbox_NO2);
    Exit();
  }
  if (mbox_close(mbox_H2O) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not close mailbox %d!\n", getpid(), mbox_H2O);
    Exit();
  }
  if (mbox_close(mbox_H2) == MBOX_FAIL) {
    Printf("makeprocs (%d): Could not close mailbox %d!\n", getpid(), mbox_H2);
    Exit();
  }
  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());
}
