#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "synch.h"
#include "queue.h"
#include "mbox.h"

static mbox mboxs[MBOX_NUM_MBOXES]; //All mail boxes in the system
//static mbox_message[MBOX_NUM_BUFFERS];//All buffers in the system
//-------------------------------------------------------
//
// void MboxModuleInit();
//
// Initialize all mailboxes.  This process does not need
// to worry about synchronization as it is called at boot
// time.  Only initialize necessary items here: you can
// initialize others in MboxCreate.  In other words, 
// don't waste system resources like locks and semaphores
// on unused mailboxes.
//
//-------------------------------------------------------

void MboxModuleInit() {
   int i,j; //Loop Index variable
   dbprintf ('p',"SynchModuleInit: Entering SynchModuleInit\n");
   for(i=0; i< MBOX_NUM_MBOXES; i++){
      mboxs[i].inuse=0;            //all mailboxes available
      for(j=0;j<NUM_PROCESS;j++){
      mboxs[i].users[j]=-1;        //no users right now
      }
      for(j=0;j<MBOX_MAX_BUFFERS_PER_MBOX;j++){  //no messages right now
            mboxs[i].mbox_msg[j].msg_content="";
      }
     }
     
   dbprintf ('p',"SynchModuleInit: Leaving SynchModuleInit\n");         
}

//-------------------------------------------------------
//
// mbox_t MboxCreate();
//
// Allocate an available mailbox structure for use. 
//
// Returns the mailbox handle on success
// Returns MBOX_FAIL on error.
//
//-------------------------------------------------------
mbox_t MboxCreate() {
  mbox_t mbox_handle;
  int j;
  for(mbox_handle=0;mbox_handle<MBOX_NUM_MBOXES;mbox_handle++){
     if(mboxs[mbox_handle].inuse==0){
        mboxs[mbox_handle].inuse=1;
        mboxs[mbox_handle].user_num=0;
        mboxs[mbox_handle].lock=LockCreate();
        mboxs[mbox_handle].empty = SemCreate(MBOX_MAX_BUFFERS_PER_MBOX);
        mboxs[mbox_handle].full = SemCreate(0);
        mboxs[mbox_handle].head=0;
        mboxs[mbox_handle].tail=0;  //message pointer initialization

         for(j=0;j<NUM_PROCESS;j++){
            mboxs[mbox_handle].users[j]=-1;        //mail box initialization
             }
         for(j=0;j<MBOX_MAX_BUFFERS_PER_MBOX;j++){ 
                mboxs[mbox_handle].mbox_msg[j].msg_content="\0";
           //    printf("%s\n",mboxs[mbox_handle].mbox_msg[j].msg_content);
             }
        return mbox_handle;       
        }
      }
  return MBOX_FAIL;
}

//-------------------------------------------------------
// 
// void MboxOpen(mbox_t);
//
// Open the mailbox for use by the current process.  Note
// that it is assumed that the internal lock/mutex handle 
// of the mailbox and the inuse flag will not be changed 
// during execution.  This allows us to get the a valid 
// lock handle without a need for synchronization.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxOpen(mbox_t handle) {
  int i;
  int id;
  id=GetCurrentPid();
  if(handle<0 || handle>=MBOX_NUM_MBOXES){
     printf("Invalid handle\n");
     return MBOX_FAIL;
    }
  if( mboxs[handle].inuse==0){
     printf("The mailbox not created!\n");
     return MBOX_FAIL;
   }
  for(i=0;i<NUM_PROCESS;i++){
     if(id == mboxs[handle].users[i]){
     printf("Mailbox already opened by this process!\n");
     return MBOX_FAIL;
   }
   }
  for(i=0;i<NUM_PROCESS;i++){
     if(mboxs[handle].users[i]<0){
     printf("Process %d opened mailbox %d!\n",id,handle);
     mboxs[handle].users[i]=id;
     mboxs[handle].user_num++;
     return MBOX_SUCCESS;
     }
}  
  return MBOX_FAIL;
}

//-------------------------------------------------------
//
// int MboxClose(mbox_t);
//
// Close the mailbox for use to the current process.
// If the number of processes using the given mailbox
// is zero, then disable the mailbox structure and
// return it to the set of available mboxes.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxClose(mbox_t handle) {
  int i;
  int id;
  id=GetCurrentPid();
  if(handle<0 || handle>=MBOX_NUM_MBOXES){
     printf("Invalid handle\n");
     return MBOX_FAIL;
    }
  if( mboxs[handle].inuse==0){
     printf("The mailbox not created!\n");
     return MBOX_FAIL;
   }
  for(i=0;i<NUM_PROCESS;i++){
     if(id == mboxs[handle].users[i]){
     printf("Process %d closed mail box %d!\n",id,handle);
     mboxs[handle].users[i]=-1;
     mboxs[handle].user_num-- ;
     if(mboxs[handle].user_num==0){
        mboxs[handle].inuse=0;        //mail box being reallocated
       }
     return MBOX_SUCCESS;
   }
  }
  printf("Mail box not opened yet!\n");
  return MBOX_FAIL;
}

//-------------------------------------------------------
//
// int MboxSend(mbox_t handle,int length, void* message);
//
// Send a message (pointed to by "message") of length
// "length" bytes to the specified mailbox.  Messages of
// length 0 are allowed.  The call 
// blocks when there is not enough space in the mailbox.
// Messages cannot be longer than MBOX_MAX_MESSAGE_LENGTH.
// Note that the calling process must have opened the 
// mailbox via MboxOpen.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//-------------------------------------------------------
int MboxSend(mbox_t handle, int length, void* message) {
  int i;
  int id;
  id=GetCurrentPid();
  if(handle<0 || handle>=MBOX_NUM_MBOXES){
     printf("Invalid handle\n");
     return MBOX_FAIL;
    }
  if( mboxs[handle].inuse==0){
     printf("The mailbox not created!\n");
     return MBOX_FAIL;
   }
  if( length>MBOX_MAX_MESSAGE_LENGTH){
     printf("Invalid length of message!\n");
     return MBOX_FAIL;
   }
  for(i=0;i<NUM_PROCESS;i++){
     if(id == mboxs[handle].users[i]){
       SemHandleWait(mboxs[handle].empty);
       LockHandleAcquire(mboxs[handle].lock);
       bcopy (message,mboxs[handle].mbox_msg[mboxs[handle].head].msg_content,length);  //bcopy found in ostraps.h
       mboxs[handle].mbox_msg[mboxs[handle].head].length=length;
       mboxs[handle].head=(mboxs[handle].head+1) % MBOX_MAX_BUFFERS_PER_MBOX;
      //    printf("the head point is %d \n",mboxs[handle].head);
      //    printf("%s, %s\n",mboxs[handle].mbox_msg[mboxs[handle].head-1].msg_content,message);
       SemHandleSignal(mboxs[handle].full);
       LockHandleRelease(mboxs[handle].lock);
      return MBOX_SUCCESS;
   }
  }
   printf("mail box not opened yet\n");
  return MBOX_FAIL;
}

//-------------------------------------------------------
//
// int MboxRecv(mbox_t handle, int maxlength, void* message);
//
// Receive a message from the specified mailbox.  The call 
// blocks when there is no message in the buffer.  Maxlength
// should indicate the maximum number of bytes that can be
// copied from the buffer into the address of "message".  
// An error occurs if the message is larger than maxlength.
// Note that the calling process must have opened the mailbox 
// via MboxOpen.
//   
// Returns MBOX_FAIL on failure.
// Returns number of bytes written into message on success.
//
//-------------------------------------------------------
int MboxRecv(mbox_t handle, int maxlength, void* message) {
  int i;
  int id;
  int msg_length;
  id=GetCurrentPid();
  if(handle<0 || handle>=MBOX_NUM_MBOXES){
     printf("Invalid handle\n");
     return MBOX_FAIL;
    }
  if( mboxs[handle].inuse==0){
     printf("The mailbox not created!\n");
     return MBOX_FAIL;
   }
  for(i=0;i<NUM_PROCESS;i++){
     if(id == mboxs[handle].users[i]){
       SemHandleWait(mboxs[handle].full);
       LockHandleAcquire(mboxs[handle].lock);
       msg_length=mboxs[handle].mbox_msg[mboxs[handle].tail].length;

         if(msg_length>maxlength){
           printf("message size larger than maxlength!\n");
           return MBOX_FAIL;
           break;
          }
       bcopy (mboxs[handle].mbox_msg[mboxs[handle].tail].msg_content,message,msg_length);  //read message
       mboxs[handle].tail=(mboxs[handle].tail+1) % MBOX_MAX_BUFFERS_PER_MBOX;
     //  printf("the tail pointer is %d \n",mboxs[handle].tail);
       SemHandleSignal(mboxs[handle].empty);
       LockHandleRelease(mboxs[handle].lock);
      return msg_length;
   }
  }
   printf("mail box not opened yet\n");
  return MBOX_FAIL;
}

//--------------------------------------------------------------------------------
// 
// int MboxCloseAllByPid(int pid);
//
// Scans through all mailboxes and removes this pid from their "open procs" list.
// If this was the only open process, then it makes the mailbox available.  Call
// this function in ProcessFreeResources in process.c.
//
// Returns MBOX_FAIL on failure.
// Returns MBOX_SUCCESS on success.
//
//--------------------------------------------------------------------------------
int MboxCloseAllByPid(int pid) {
  int i,j,count=0;
  for (i=0; i< MBOX_NUM_MBOXES;i++){
        if(mboxs[i].inuse==1){
        for(j=0;j<NUM_PROCESS;j++){      //scanning mbox[i]
             if (mboxs[i].users[j]==pid){
                 mboxs[i].users[j]=-1;
                 mboxs[i].user_num--;
                 count++;
                   if(mboxs[i].user_num==0){
                      mboxs[i].inuse=0;
                    } 
              }
        }
      }
     }
  if(count>=MBOX_NUM_MBOXES){
  return MBOX_FAIL;
  }
  else
  return MBOX_SUCCESS;
}
