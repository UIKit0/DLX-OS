#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  missile_code mc;         // Used to access missile codes from mailbox
  mbox_t mbox_N2;           // Handle to the mbox_N2
  mbox_t mbox_N;            //Handle to mbox_N
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 4) { 
    Printf("Usage: %s <handle_to_mailbox> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  mbox_N2 = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  mbox_N = dstrtol(argv[2], NULL ,10);
  s_procs_completed = dstrtol(argv[3], NULL, 10);

  // Open the mailbox
  if (mbox_open(mbox_N2) == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not open the mailbox_N2!\n", getpid());
    Exit();
  }
  if (mbox_open(mbox_N) == MBOX_FAIL) {
    Printf("spawn_me (%d): Could not open the mailbox_N!\n", getpid());
    Exit();
  }

  // Wait for a message from the mailbox
  if (mbox_recv(mbox_N2, sizeof(mc), (void *)&mc) == MBOX_FAIL) {
    Printf("Could not receive message from mbox_N2!\n");
    Exit();
  }
  if (mbox_send(mbox_N, sizeof(missile_code),(void *)&mc) == MBOX_FAIL){
    Printf("Could not send message to mbox_N!\n");
    Exit();
  } 
  if (mbox_send(mbox_N, sizeof(missile_code),(void *)&mc) == MBOX_FAIL){
    Printf("Could not send message to mbox_N!\n");
    Exit();
  } 
  // Now print a message to show that everything worked
//  Printf("spawn_me (%d): Received missile code: %c\n", getpid(), mc.really_important_char);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("spawn_me (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

 // Printf("spawn_me (%d): Done!\n", getpid());
}
