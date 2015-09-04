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
static int nfreepage=477;
static int freemapmax;
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
     int i;
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
  printf("debug into pagefault\n");
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
  
  //   for(i=0;i<16;i++){
  //   printf("freemap is 0x%x\n",freemap[i]);
 //    }
  
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
   MemorySetFreemap (page,1);
   nfreepage += 1;
 //  printf("Freed page 0x%x, %d remaining.\n",page,nfreepage);    
}
void MemorySetFreemap(int p, int b)
{
  uint32 wd = p /32;
  uint32 bitnum = p %32;
  freemap[wd] = (freemap[wd] & invert(1 << bitnum)) | (b << bitnum);
//  dbprintf ('m',"Set freemap entry %d to 0x%x.\n",wd,freemap[wd]);
}

int findtree(int order,PCB *pcb,node *node,int memsize);

void *malloc(PCB *currentPCB, int memsize){
  int order=0;
  int size,base=32;
  int address;
  uint32 virtual_address,physical_address;
  
  if(memsize<=0){
    return NULL;
  }
  if(memsize>4096){
    return NULL;
  }

  size = memsize;
  while(base<size){
  base=base*2;  //tries to find the order
  order++;
  }
//  printf("allocate order is %d\n",order);
  if((address=findtree(order,currentPCB,currentPCB->heapnode,memsize))!=-1){
 // printf("relative address in the heap is %d\n",address);
  virtual_address=(uint32)(16384+address);
  physical_address=MemoryTranslateUserToSystem (currentPCB,virtual_address);
  printf("Created a heap block of size <%d> bytes; virtual address <0x%x>, physical address <0x%x>\n",memsize,virtual_address,physical_address);
  return((uint32 *)virtual_address); 
  }
  else{
  return NULL;
  }
}

int findnode(PCB *pcb){
    int i;
    for(i=1;i<256;i++){
      if(pcb->heapnode[i].inuse==1)
      break;
    }
    pcb->heapnode[i].inuse=0;
    return(i);
}
int findtree(int order,PCB *pcb,node *node,int memsize){
    int f;
  if(node->order==order){//find the right order
    if(!((node->left==NULL)&&(node->right==NULL))){ //not leaf but chunk could only be allocated at leaf
       return(-1);
    }
    else{   //the leaf node
       if(node->free_status==0){  //not free
          return(-1);
       }
       else{   //find one heap chunk!
           node->free_status=0;
           printf("Allocated the block: order = %d, addr= %d , requested mem size = %d, block size = %d\n",node->order,node->address,memsize,node->size); 
           return(node->address);
           }
    }
  }
  else{ //fail to locate correct order,keep searching
    if((node->left==NULL)&&(node->right==NULL)){     //leaf with higher order, should create new node
      if(node->free_status==0){ //not free
         return(-1);
      }
      else{ //insert children
        node->left=&pcb->heapnode[findnode(pcb)];
        node->left->order=node->order-1;
        node->left->address=node->address;
        node->left->size=node->size/2;
        node->left->left=NULL;
        node->left->right=NULL;
        node->left->free_status=1;  //initiate as free
        printf("Created a left child node(order= %d, addr= %d, size= %d) of parent (order= %d, addr= %d, size= %d)\n",node->left->order,node->left->address,node->left->size,node->order,node->address,node->size);
        node->right=&pcb->heapnode[findnode(pcb)];
        node->right->order=node->order-1;
        node->right->address=node->address+(32*(1<<node->right->order));
        node->right->size=node->size/2;
        node->right->left=NULL;
        node->right->right=NULL;
        node->right->free_status=1; //initiate as free
        printf("Created a right child node(order= %d, addr= %d, size= %d) of parent (order= %d, addr= %d, size= %d)\n",node->right->order,node->right->address,node->right->size,node->order,node->address,node->size);
        if(((f=findtree(order,currentPCB,node->left,memsize))==-1)){
          //  printf("f= %d\n",f);
            return(findtree(order,currentPCB,node->right,memsize));
        }
        else{
          //  printf("f= %d\n",f);
            return (f);
        }
      }
    }
    else{ //not a leaf, search in left child first, then right child
      if(((f=findtree(order,currentPCB,node->left,memsize))==-1)){
          return(findtree(order,currentPCB,node->right,memsize));
      }
      else{
          return (f);
      }
    }
  }
}   

int freenode(int address,node *node);
int freetree(node *node);

int mfree(PCB *currentPCB, void *ptr){
    int address;
    int size;
    uint32 virtual_address,physical_address;
    
    if(ptr==NULL){
    return -1;
    }
    if(((int)ptr<16384)||((int)ptr>=20480)){ //out of range
    return -1;
    }
    address=((int)ptr)-16384;
    virtual_address=ptr;
    physical_address=MemoryTranslateUserToSystem(currentPCB,virtual_address);
   // printf("free address is %d\n",address);
    if(((size=freenode(address,currentPCB->heapnode)))!=-1){
    printf("Freeing heap block of size <%d> bytes: virtual address <0x%x> , physical address <0x%x>\n",size,virtual_address,physical_address);
    freetree(currentPCB->heapnode);
    return(size);
    }
    else{
    printf("Cannot free the heap block of size<%d> bytes:virtual address <0x%x>, physicsal address <0x%x>\n",size, virtual_address,physical_address);
    return NULL;
    }
}

int freetree(node *node){
    if((node->left==NULL)&&(node->right==NULL)){//leaf
      if(node->free_status==1){
         return(1);
      }
      else{
         return(-1);
      }
    }
    if((node->left->left==NULL)&&(node->left->right==NULL)&&(node->right->left==NULL)&&(node->right->right==NULL)){ //childern are leaves
       if((node->left->free_status==1)&&(node->right->free_status==1)){//both children are free
           printf("Coalesced buddy nodes(order = %d, addr= %d, size= %d) & (order = %d, addr = %d, size = %d) ",node->left->order,node->left->address,node->left->size,node->right->order,node->right->address,node->right->size);
           node->left->inuse=1;
           node->right->inuse=1;
           node->left=NULL;
           node->right=NULL;
           node->free_status=1;
           printf("into the parent node (order= %d, addr= %d, size= %d)\n",node->order,node->address,node->size);
           return(1);
       }
       else{
          printf("the other child node is not free now,parent->order = %d, parent->addr =%d,parent-> size=%d\n",node->order,node->address,node->size); 
          return(-1);
       }
    }
    else{
       if((freetree(node->left)==1)&&(freetree(node->right)==1)){
           printf("Coalesced buddy nodes(order = %d, addr= %d, size= %d) & (order = %d, addr = %d, size = %d) ",node->left->order,node->left->address,node->left->size,node->right->order,node->right->address,node->right->size);
           node->left->inuse=1;
           node->right->inuse=1;
           node->left=NULL;
           node->right=NULL;
           node->free_status=1;
           printf("into the parent node (order= %d, addr= %d, size= %d)\n",node->order,node->address,node->size);
           return(1);
       }
       else{
    //      printf("the other child node is not free now,parent->order = %d, parent->addr =%d,parent-> size=%d\n",node->order,node->address,node->size);
          return(-1);
       }
    }
 }

      
int freenode(int address, node *node){
    int f;
    if((node->left==NULL)&&(node->right==NULL)){//only leaf on the tree could be freed
       if(node->address==address){
          node->free_status=1;
          printf("Free the block: order= %d, addr= %d,size= %d\n",node->order,node->address,node->size);
          return(node->size);
       }
       else{
          return(-1);
       }
    }
    else{// it is not leaf
        if((f=freenode(address,node->left))==-1){//not found on left child
           return(freenode(address,node->right));
        }
        else{
           return(f);
        }
    }
}
