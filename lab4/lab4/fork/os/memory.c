//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 freemap[MEM_MAX_PAGES/32];
static uint32 pagestart; 
static int nfreepage=MEM_MAX_PAGES;
static int freemapmax;
int counter[MEM_MAX_PAGES]={0};
  
//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void SetPara(){
     pagestart = (lastosaddress+(MEM_PAGESIZE))/(MEM_PAGESIZE);
     freemapmax=((MemoryGetSize()/(MEM_PAGESIZE))+31)/32;
     nfreepage=(MemoryGetSize()/(MEM_PAGESIZE))-pagestart+1;
     printf("nfreepage is %d\n",nfreepage);  
}

void MemoryModuleInit() {
     int i;
     int curpage;
   
     dbprintf('m',"MemoryGetsize() is %d\n",MemoryGetSize());
     dbprintf('m',"MEM_PAGESIZE is %d\n",MEM_PAGESIZE);
     SetPara();    
    //All memory is in use
     for (curpage=0;curpage<(MemoryGetSize()/(MEM_PAGESIZE));curpage++) {
//         printf("curpage is %d\n",curpage);
         MemorySetFreemap(curpage,0);
     }
         
     //all the other memory space is unused
     for (curpage = ((lastosaddress+(MEM_PAGESIZE))/(MEM_PAGESIZE));curpage <(MemoryGetSize()/(MEM_PAGESIZE)); curpage++)
      {
 //        printf("curpage is %d\n",curpage);
         MemorySetFreemap (curpage, 1);
      }
     
     for(i=0;i<MEM_MAX_PAGES;i++){
         counter[i]=0;
      //  printf("counter[%d]=%d\n",i,counter[i]);
     }

    //print for debug 
     for(i=0;i<16;i++){
     dbprintf('m',"freemap is 0x%x\n",freemap[i]);
     }
}

//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
      int page = (addr)/(MEM_PAGESIZE);
      int offset = addr % (MEM_PAGESIZE);

      if((page > (MEM_PTSIZE)-1)) {
        return 0;
      }
      return ((pcb->pagetable[page] & MEM_PTE_MASK)+offset);   
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied
  
//    dbprintf('m',"debug into Memory MoveBetweenSpaces but n=%d\n",n);
  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
  //  dbprintf('m',"user address is 0x%x\n",(uint32)user);
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);
  //  dbprintf('m',"curUser address is 0x%x\n",curUser);
    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

   // dbprintf('m',"debug into MemoryMoveBetweenSpaces\n");
    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
   // dbprintf('m',"n is %d\n",n);
   // dbprintf('m',"(uint32)curUser & (MEM_ADDRESS_OFFSET_MASK)= 0x%x\n",(uint32)curUser & (MEM_ADDRESS_OFFSET_MASK));
    bytesToCopy = ((MEM_PAGESIZE) - ((uint32)curUser & (MEM_ADDRESS_OFFSET_MASK)));
   // dbprintf('m',"bytesToCopy is %d\n",bytesToCopy); 
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
 //   dbprintf('m',"debug n=%d\n",n);
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
  int newPage;
//  printf("debug into pagefault\n");
  if (pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]>= pcb->currentSavedFrame[PROCESS_STACK_FAULT])
   { 
     newPage = MemoryAllocPage ();
      if (newPage == MEM_FAIL) {
          printf("FATAL:couldn't allocate memory - no free pages!\n");
          ProcessKill();
    }
     pcb->npages +=1;
     printf("(int)pcb->currentSavedFrame[PROCESS_STACK_FAULT]=0x%x\n",(int)pcb->currentSavedFrame[PROCESS_STACK_FAULT]);
     printf("pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]=0x%x\n",pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
     pcb->pagetable[(int)pcb->currentSavedFrame[PROCESS_STACK_FAULT]/(MEM_PAGESIZE)] = MemorySetupPte(newPage);
     printf("Allocate a newpage, the page number is %d\n",newPage);
     return MEM_SUCCESS;
   }
   else{
     printf("FATAL: page fault!\n");
     ProcessKill();
     return MEM_FAIL;
   }
}

//ROP_ACCESS_HANDLER
int RopAccessHandler(PCB *pcb) {
 int vPage,pPage,newPage;
 char *paddress;
 int i;
 
 printf("---------------------------------ROP_ACCESS_HANDLER-----------------------------\n");
 vPage=(int)pcb->currentSavedFrame[PROCESS_STACK_FAULT]/(MEM_PAGESIZE);
 printf("RopAccessHandler:virtual page number of PROCESS_STACK_FAULT is 0x%x\n",vPage); 
 pPage=(int)(((pcb->pagetable[vPage])&(0xfffff000))>>(MEM_L1FIELD_FIRST_BITNUM));
 printf("RopAccessHandler:physical page number is 0x%x\n",pPage);
 if(counter[pPage]>1){
 //allocate a new page
 printf("more than 1 process using the page, ready to allocate a new page!\n");
 newPage = MemoryAllocPage ();
   if (newPage == MEM_FAIL) {
       printf("FATAL:couldn't allocate memory - no free pages!\n");
       ProcessKill();
   }
 paddress=(char *)( newPage*(MEM_PAGESIZE));
 bcopy((char*)((pcb->pagetable[vPage])&(MEM_PTE_MASK)),paddress,(int)MEM_PAGESIZE);
 printf("Allocated a new page with page number 0x%x\n",newPage);
 printf("You copied from 0x%x to 0x%x, %d bytes\n",(pcb->pagetable[vPage])&(MEM_PTE_MASK),paddress,MEM_PAGESIZE);
 pcb->pagetable[vPage]= MemorySetupPte(newPage);
 printf("After allocating a new page, the revised pagetable is:\n");
 for(i=0;i<MEM_PTSIZE;i++){
    if(pcb->pagetable[i]!=0){
       printf("Page_number %d -> PTE: 0x%x\n",i,pcb->pagetable[i]);
      }
 }
 counter[newPage]+=1;
 counter[pPage]-=1;
 printf("---------------------------Leaving ROP_ACCESS_HANDLER----------------------------------------\n");
 return MEM_SUCCESS;
 }
 else if(counter[pPage]==1){
 //Clear the read-only bit
 pcb->pagetable[vPage]=(pcb->pagetable[vPage]&(0xfffffffb));
 printf("You have cleared the read-only bit,the new PTE is 0x%x\n",pcb->pagetable[vPage]);
 printf("The revised pagetable is:\n");
 for(i=0;i<MEM_PTSIZE;i++){
    if(pcb->pagetable[i]!=0){
       printf("Page_number %d -> PTE: 0x%x\n",i,pcb->pagetable[i]);
      }
 }
 printf("---------------------------Leaving ROP_ACCESS_HANDLER----------------------------------------\n");
 return MEM_SUCCESS;
 }
 else{
 printf("Error in RopAccessHandler,you are tring to write to an invalid page\n");
 printf("---------------------------Leaving ROP_ACCESS_HANDLER----------------------------------------\n");
 return MEM_FAIL;
 }
}
//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

uint32 MemoryAllocPage(void) {
   int mapnum;
   int bitnum;
   int i;
   int page;
   uint32 v;

     dbprintf('m',"There are still %d free pages.\n",nfreepage);

   page=(lastosaddress+(MEM_PAGESIZE))/(MEM_PAGESIZE);
   mapnum=page/32;
   if (nfreepage == 0) {
      return MEM_FAIL;
   }

   dbprintf('m',"Allocating memory, starting with entry %d\n",mapnum);
   while (freemap[mapnum] == 0) {
     mapnum +=1;
     if (mapnum >= ((MemoryGetSize()/(MEM_PAGESIZE))+31)/32) {
        mapnum = page/32;
     }
    }
    v=freemap[mapnum];
    for (bitnum = 0; (v & (1 << bitnum)) == 0; bitnum++) {
    }
    freemap[mapnum] &= invert(1 << bitnum);
    v = (mapnum *32)+bitnum;
    dbprintf('m',"Allocated memory, from map %d, page 0x%x, map=0x%x.\n",mapnum,v,freemap[mapnum]);
    nfreepage--;
     
//     printf("After allocation,the freemap is:\n"); 
//     for(i=0;i<16;i++){
//     printf("freemap is 0x%x\n",freemap[i]);
//     }
  
    return (v);
}

uint32 MemoryInitPte (uint32 page){
  return (0);
}

uint32 MemorySetupPte (uint32 page) {
  return ((page * MEM_PAGESIZE) | MEM_PTE_VALID);
}

void MemoryFreePte (uint32 pte)
{
  MemoryFreePage ((pte & MEM_PTE_MASK) /(MEM_PAGESIZE));
}

void MemoryFreePage(uint32 page)
{
   int i;
   MemorySetFreemap (page,1);
   nfreepage += 1;
//   printf("Freed physical page 0x%x, %d remaining.\n",page,nfreepage);    
  //   printf("After MemoryFreePage,the freemap is:\n"); 
  //   for(i=0;i<16;i++){
 //    printf("freemap is 0x%x\n",freemap[i]);
 //    }
}
void MemorySetFreemap(int p, int b)
{
  uint32 wd = p /32;
  uint32 bitnum = p %32;
  freemap[wd] = (freemap[wd] & invert(1 << bitnum)) | (b << bitnum);
//  dbprintf ('m',"Set freemap entry %d to 0x%x.\n",wd,freemap[wd]);
}

void *malloc(PCB *currentPCB, int memsize){
}

int mfree(PCB *currentPCB, void *ptr){
}
