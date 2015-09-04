#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  missile_code mc;         // Used to access missile codes from mailbox
  mbox_t mbox_N;           // Handle to the mbox_N
  mbox_t mbox_O2;
  mbox_t mbox_NO2;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 5) { 
    Printf("Usage: %s <handle_to_mbox_N><handle_to_mbox_O2><handle_to_mbox_NO2> <handle_to_page_mapped_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  mbox_N = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  mbox_O2 = dstrtol(argv[2],NULL,10);
  mbox_NO2 = dstrtol(argv[3],NULL,10);

  s_procs_completed = dstrtol(argv[4], NULL, 10);

  // Open the mailbox
  if (mbox_open(mbox_N) == MBOX_FAIL) {
    Printf("Could not open the mbox_N!\n");
    Exit();
  }
  if (mbox_open(mbox_O2) == MBOX_FAIL) {
    Printf("Could not open the mbox_N!\n");
    Exit();
  }
  if (mbox_open(mbox_NO2) == MBOX_FAIL) {
    Printf("Could not open the mbox_N!\n");
    Exit();
  }

  // Wait for a message from the mailbox
  if (mbox_recv(mbox_N, sizeof(mc), (void *)&mc) == MBOX_FAIL) {
    Printf("Could not receive message from mbox_N!\n");
    Exit();
  }
  if (mbox_recv(mbox_O2, sizeof(mc), (void *)&mc) == MBOX_FAIL) {
    Printf("Could not receive message from mbox_O2!\n");
    Exit();
  }
  if (mbox_send(mbox_NO2, sizeof(missile_code), (void *)&mc) == MBOX_FAIL) {
    Printf("Could not send message to mbox_NO2!\n");
    Exit();
  }
  else Printf("Got 1 NO2!\n");
 
  // Now print a message to show that everything worked
 // Printf("spawn_me (%d): Received missile code: %c\n", getpid(), mc.really_important_char);

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("spawn_me (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

//  Printf("spawn_me (%d): Done!\n", getpid());
}
