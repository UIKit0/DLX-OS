#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"
//#define PRODUCER 0
//#define CONSUMER 1

void producer(circular_buffer *mc,int ,lock_t,lock_t,lock_t, cond_t,cond_t);
void consumer(circular_buffer *mc,int ,lock_t,lock_t,lock_t, cond_t,cond_t);
//int flag;

void main (int argc, char *argv[])
{
  circular_buffer *mc;     // Used to access buffers in shared memory page
  uint32 h_mem;            // Handle to the shared memory page
  sem_t s_procs_completed_pc; // Semaphore to signal the original producer_consumer process that we're done
 // sem_t s_fullslots;          //Semaphore as full
 // sem_t s_emptyslots;        //Semaphore as empty
  cond_t c_notfull;         //condition variable notfull
  cond_t c_notempty;        //condition variable notempty 
  int id;                   //producer_consumer id
  lock_t lock;            //lock id
  lock_t lock_full; 
  lock_t lock_empty;
  int type;               //process type
 // int revtal             //signal for semarphore
  if (argc != 10) { 
    Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_complete_semaphore> <process type> <process id> <lock> <lock_full> <lock_empty> <c_notfull> <c_notempty>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
  s_procs_completed_pc = dstrtol(argv[2], NULL, 10);
  type= dstrtol(argv[3],NULL,10);
  id = dstrtol(argv[4],NULL,10);
  lock= dstrtol(argv[5],NULL,10);
  lock_full =dstrtol(argv[6],NULL,10);
  lock_empty =dstrtol(argv[7],NULL,10);
  c_notfull =dstrtol(argv[8],NULL,10);
  c_notempty =dstrtol(argv[9],NULL,10);
 // Printf("type is %d\n",type);
  // Map shared memory page into this process's memory space
  if ((mc = (circular_buffer *)shmat(h_mem)) == NULL) {
    Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
    Exit();
  }
  //decide which process to run
  if (type== PRODUCER){
               producer(mc,id,lock,lock_full,lock_empty,c_notfull,c_notempty);
}else if(type == CONSUMER){
               consumer(mc,id,lock,lock_full,lock_empty,c_notfull,c_notempty);
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
void producer (circular_buffer *mc,int id,lock_t lock,lock_t lock_full,lock_t lock_empty,cond_t c_notfull,cond_t c_notempty)
{       
    int i;
    char producer_code[11]={'H','e','l','l','o','\0','W','o','r','l','d'};
    for (i=0;i<BUFFER_SIZE;i++){
         lock_acquire(lock);                        //Mutual exclusion
         if((mc->head+1)%BUFFER_SIZE==mc->tail){   //buffer is full
         lock_acquire(lock_full);
         lock_release(lock);

         cond_wait(c_notfull);                    //waiting on condition variable

         lock_acquire(lock);
         }
         
     //  Printf("producer %d waiting,head is %d, tail is %d\n  ",id,mc->head,mc->tail);}
         mc->buffer[mc->head]=producer_code[i];                               //insert item
         Printf("Producer %d,	Produce: %c \n  ",id,mc->buffer[mc->head]);   //Critical Section
   //     Printf("head is:%d tail is %d  ",mc->head,mc->tail);
         mc->head=(mc->head+1)%BUFFER_SIZE;
  //      Printf("head is:%d  tail is %d\n  ",mc->head,mc->tail);
         if(mc->head==mc->tail){
          lock_acquire(lock_empty);
          cond_signal(c_notempty);
          lock_release(lock_empty);
         }          
         lock_release(lock);  
        }
}                        
void consumer (circular_buffer *mc,int id,lock_t lock,lock_t lock_full,lock_t lock_empty,cond_t c_notfull,cond_t c_notempty)
{
       int i;
       for(i=0;i<BUFFER_SIZE;i++){
         lock_acquire(lock);         
       if((mc->head==mc->tail)){                   //buffer is empty
         lock_acquire(lock_empty);
         lock_release(lock);
 
         cond_wait(c_notempty);

         lock_acquire(lock);
         }

     //  Printf("Consumer %d waiting, head is %d ,tail is %d\n ",id,mc->head,mc->tail);}
         Printf("Consumer %d,	Consume: %c \n ",id,mc->buffer[mc->tail]); //remove item
  //	 Printf("head is %d tail is %d ",mc->head,mc->tail);
         mc->tail=(mc->tail+1)%BUFFER_SIZE;
  //       Printf("head is %d tail is %d\n  ",mc->head,mc->tail);
         if((mc->head+1)%BUFFER_SIZE==mc->tail){
          lock_acquire(lock_full);
          cond_signal(c_notfull);
          lock_release(lock_full);
          }     
          lock_release(lock);                          //release lock
        }
}
