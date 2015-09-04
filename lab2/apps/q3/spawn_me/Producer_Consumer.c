#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"
//#define PRODUCER 0
//#define CONSUMER 1

void producer(circular_buffer *mc,int ,lock_t, sem_t,sem_t);
void consumer(circular_buffer *mc,int ,lock_t, sem_t,sem_t);
//int flag;

void main (int argc, char *argv[])
{
  circular_buffer *mc;     // Used to access buffers in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed_pc; // Semaphore to signal the original producer_consumer process that we're done
  sem_t s_fullslots;          //Semaphore as full
  sem_t s_emptyslots;        //Semaphore as empty
  int id;                   //producer_consumer id
  lock_t lock;            //lock id
  int type;               //process type
 // int revtal             //signal for semarphore
  if (argc != 8) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore><process id> <process type> <s_full> <s_empty>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed_pc = dstrtol(argv[2], NULL, 10);
  id = dstrtol(argv[3],NULL,10);
  lock= dstrtol(argv[4],NULL,10);
  type= dstrtol(argv[5],NULL,10);
  s_fullslots =dstrtol(argv[6],NULL,10);
  s_emptyslots=dstrtol(argv[7],NULL,10);
 // Printf("type is %d\n",type);
  // Map shared memory page into this process's memory space
  if ((mc = (circular_buffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //decide which process to run
 // flag=0;
  if (type== PRODUCER){
               producer(mc,id,lock,s_fullslots,s_emptyslots);
}else if(type == CONSUMER){
               consumer(mc,id,lock,s_fullslots,s_emptyslots);
}else{
     Printf("Wrong process\n");
}
  // Signal the semaphore to tell the original process that we're done
  Printf("Process_%d_type_%d is complete.\n", id,type);
  if(sem_signal(s_procs_completed_pc) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed_pc); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
}
void producer (circular_buffer *mc,int id,lock_t lock,sem_t s_fullslots,sem_t s_emptyslots)
{       
    int i;
    char producer_code[11]={'H','e','l','l','o','\0','W','o','r','l','d'};
    for (i=0;i<BUFFER_SIZE;i++){
         sem_wait(s_emptyslots);
         lock_acquire(lock);                        //Acquire lock
     //  if(!((mc->head+1)%BUFFER_SIZE==mc->tail)){   //not full
     //  Printf("producer %d waiting,head is %d, tail is %d\n  ",id,mc->head,mc->tail);}
         mc->buffer[mc->head]=producer_code[i];
         Printf("Producer %d,	Produce: %c \n  ",id,mc->buffer[mc->head]);   //Critical Section
   //     Printf("head is:%d tail is %d  ",mc->head,mc->tail);
         mc->head=(mc->head+1)%BUFFER_SIZE;
  //      Printf("head is:%d  tail is %d\n  ",mc->head,mc->tail);
         lock_release(lock);                 //release lock
         sem_signal(s_fullslots);
        }
}                        
void consumer (circular_buffer *mc,int id,lock_t lock,sem_t s_fullslots, sem_t s_emptyslots)
{
       int i;
       for(i=0;i<BUFFER_SIZE;i++){
         sem_wait(s_fullslots);
         lock_acquire(lock);                       //Acquire lock
     //   if(!(mc->head==mc->tail)){                   //not empty
     //  Printf("Consumer %d waiting, head is %d ,tail is %d\n ",id,mc->head,mc->tail);}
         Printf("Consumer %d,	Consume: %c \n ",id,mc->buffer[mc->tail]);//Critical Section
  //	 Printf("head is %d tail is %d ",mc->head,mc->tail);
         mc->tail=(mc->tail+1)%BUFFER_SIZE;
  //       Printf("head is %d tail is %d\n  ",mc->head,mc->tail);
         lock_release(lock);                          //release lock
         sem_signal(s_emptyslots);
        }
}
