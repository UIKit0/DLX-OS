#include "usertraps.h"
#include "misc.h"
void hello(int);
void main (int argc, char *argv[])
{
  static int  *p=0;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  
 // Printf("hello_world (%d): Hello world!\n", getpid());
 
 // hello(i); //used to test stack grow
 // Printf(p[100000000]);  //used to test reaching somewhere outside the range of virtual memory
  Printf("Accessing address within virtual memory but not in user stack\n");
  Printf("Accessing address:%d\n",p[125000]); //used to test reaching withn virtual memory but not in user stack
//  for(i=0;i<10000;i++);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("Test case (2): Done!\n");
  Printf("-----------------------------------------------------------------------------------------------\n");
}
void hello(int i)
{
  int test[100];
  int j;
  for(j=0;j<100;j++){
  test[j]=i;
  }
  Printf("hello_world, %d times\n",i);
  if(i>0){
  i--;
  hello(i);
  }
}
