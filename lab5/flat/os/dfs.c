#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

static dfs_inode inodes[DFS_INODE_MAX_NUM ]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM]; // Free block vector
static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

lock_t lock_fbv;
lock_t lock_inode;
// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().
   int i,j;
   DfsInvalidate();
   lock_fbv = LockCreate();
   lock_inode = LockCreate();
   
   if (DfsOpenFileSystem()== DFS_FAIL){
      printf("Failed to open the file syatem\n");
   }
}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
   sb.valid = 0;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
    int i;
    int num;
    dfs_block dfs_b;
    disk_block disk_b;
   
//Basic steps:
// Check that filesystem is not already open

// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.
 //  printf("In DfsOpenFileSystem\n");
// Copy the data from the block we just read into the superblock in memory

// All other blocks are sized by virtual block size:
// Read inodes
// Read free block vector
// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory
  if (sb.valid == 1){
       printf("The filesystem is already open!\n");
       return DFS_FAIL;
     }

  if ((num = DiskReadBlock(1,&disk_b)) == DISK_FAIL){
       printf("Cannot read superblock from disk!\n");
       return DFS_FAIL;
     }

  bcopy((char *)(&disk_b),(char *)(&sb),sizeof(dfs_superblock));

//print superblock info from disk
/*  printf("superblock.valid %d\n",sb.valid);
  printf("superblock.filesys_blocksize %d\n",sb.filesys_blocksize);
  printf("superblock.filesys_blocknum %d\n",sb.filesys_blocknum);
  printf("superblock.filesys_inodestart %d\n",sb.filesys_inodestart);
  printf("superblock.inodenum %d\n",sb.inodenum);
  printf("superblock.filesys_fbvstart %d\n",sb.filesys_fbvstart);
  printf("superblock.filesys_datastart %d\n",sb.filesys_datastart);
*/
  for(i=sb.filesys_inodestart;i<sb.filesys_fbvstart;i++){
     if (( num = DfsReadBlock (i,&dfs_b)) == DFS_FAIL){
       printf("Cannot read inode from disk!\n");
       return DFS_FAIL;
        }
      bcopy((char *)(&dfs_b),(char *)(&inodes[(i-sb.filesys_inodestart)*(sb.filesys_blocksize)/sizeof(dfs_inode)]),sb.filesys_blocksize);
     }
//print inode info from disk
//  for(i=0;i<192;i++){
//     printf("inodes[%d].inuse=%d,inodes[%d].filesize=%d\n",i,inodes[i].inuse,i,inodes[i].filesize);
//     }
//

  for(i=sb.filesys_fbvstart;i<sb.filesys_datastart;i++){
     if (( num = DfsReadBlock(i, &dfs_b)) == DFS_FAIL){
           printf("Cannot read free block vector from disk!\n");
           return DFS_FAIL;
         }
     bcopy((char *)(&dfs_b),(char *)(&fbv[(i-sb.filesys_fbvstart)*(sb.filesys_blocksize)/sizeof(uint32)]),sb.filesys_blocksize);
    }

//print fbv info from disk
//  for(i=0;i<DFS_FBV_MAX_NUM;i++){
//     printf("fbv[%d]=0x%x\n",i,fbv[i]);
//     }
//

   DfsInvalidate();
   bzero((char *)(&disk_b),sizeof(disk_block));
   bcopy((char *)(&sb),(char *)(&disk_b),sizeof(dfs_superblock));
   if (DiskWriteBlock(1,&disk_b) == DISK_FAIL){
           printf("Cannot write superblock back to disk!\n");
           return DFS_FAIL;
   }
   sb.valid = 1;
 //  printf("Existing DfsOpenFileSystem\n");
   return DFS_SUCCESS;
}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {
    int i;
    int num;
    dfs_block dfs_b;
    disk_block disk_b;
    
 //   printf("In DfsCloseFileSystem\n");
    if(sb.valid == 0){
       printf("The file system is NOT open!\n");
       return DFS_FAIL;
    }

    bzero((char *)(&dfs_b),sizeof(dfs_block));
    for(i=sb.filesys_inodestart;i<sb.filesys_fbvstart;i++){
        bcopy((char *)(&inodes[(i-sb.filesys_inodestart)*(sb.filesys_blocksize)/sizeof(dfs_inode)]),(char *)(&dfs_b),sb.filesys_blocksize);
        if((num = DfsWriteBlock (i, &dfs_b))== DFS_FAIL){
           printf("Cannot write inodes to disk!\n");
           return DFS_FAIL;
        }
     }

    bzero((char *)(&dfs_b),sizeof(dfs_block));
    for(i=sb.filesys_fbvstart;i<sb.filesys_datastart;i++){
        bcopy((char *)(&fbv[(i-sb.filesys_fbvstart)*(sb.filesys_blocksize)/sizeof(uint32)]),(char *)(&dfs_b),sb.filesys_blocksize);
        if((num = DfsWriteBlock (i, &dfs_b)) == DFS_FAIL){
           printf("Cannot write free block vector to disk!\n");
           return DFS_FAIL;
        }   
    }

    bzero((char *)(&disk_b),sizeof(disk_block));
    bcopy((char *)(&sb),(char *)(&disk_b),sizeof(dfs_superblock));
    if (DiskWriteBlock (1, &disk_b) == DISK_FAIL){
        printf("Cannot write superblock into Disk!\n");
        return DFS_FAIL;
    }

    DfsInvalidate();
    return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

int DfsAllocateBlock() {
// Check that file system has been validly loaded into memory
// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block
   int fbvnum;
   int bitnum;
   int block;
   uint32 v;
   int i;
 //  printf("In DfsAllocateBlock\n");
   if(sb.valid ==0){
      printf("Filesystem not open!\n");
      return DFS_FAIL;
   }
   LockHandleAcquire(lock_fbv);
   block = sb.filesys_datastart;
   fbvnum = block/32;
   while(fbv[fbvnum] == 0){
       fbvnum+=1;
       if(fbvnum>=(DFS_FBV_MAX_NUM)){
          fbvnum = block/32;
       }
   }
   v = fbv[fbvnum];
   for (bitnum = 0;(v & (1 << bitnum)) == 0; bitnum++);
   fbv[fbvnum] &= invert(1 << bitnum);
   v = (fbvnum * 32) + bitnum;
   LockHandleRelease(lock_fbv);
   
//   for(i=0;i<10;i++){
//   printf("fbv[%d]: 0x%x\n",i,fbv[i]);
//   }
   return (int)v;    
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(int blocknum) {
    int fbvnum;
    int bitnum;
    int i;
    if((blocknum<0)||(blocknum>sb.filesys_blocknum)){
       printf("Invalid blocknum = %d\n",blocknum);
       return DFS_FAIL;
    }

     fbvnum = (blocknum)/32;
     bitnum = (blocknum)%32;
    
//    printf("In DfsFreeBlock\n");
    if (sb.valid == 0){
        printf("File system not open!\n");
        return DFS_FAIL;
    }
    LockHandleAcquire(lock_fbv);
    fbv[fbvnum] = (fbv[fbvnum] & invert(1<<bitnum))|(1 << bitnum);
    LockHandleRelease(lock_fbv);
    
//    for(i=0;i<10;i++){
//     printf("fbv[%d] : 0x%x\n",i,fbv[i]);
//    }
    return DFS_SUCCESS;

}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(int blocknum, dfs_block *b) {
    int i;
    int ratio;
    disk_block disk_b;
    int disk_blocknum;

  //  printf("InDfsReadBlock\n");
    if (sb.valid == 0){
       printf("File system not open!\n");
       return DFS_FAIL;
    }
    if((blocknum<0)||(blocknum>sb.filesys_blocknum)){
       printf("Invalid blocknum = %d\n",blocknum);
       return DFS_FAIL;
    }
    if(Dfscheck(blocknum)== DFS_FAIL){
       printf("Error,DFS block %d not allocated!\n",blocknum);
       return DFS_FAIL;
     }
    
    ratio = sb.filesys_blocksize/(DISK_BLOCKSIZE);
    disk_blocknum=(blocknum*ratio);
    for(i=0;i<ratio;i++){
       if (DiskReadBlock((uint32)(disk_blocknum + i),&disk_b) == DISK_FAIL){
           printf("Cannot read block %d from disk!\n",disk_blocknum);
           return DISK_FAIL;
          }
       bcopy((char *)(&disk_b),(char *)((int)b+i*(DISK_BLOCKSIZE)),DISK_BLOCKSIZE);
    }
    return (sb.filesys_blocksize);
   
}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(int blocknum, dfs_block *b){ 
    int i;
    int ratio;
    disk_block disk_b;
    int disk_blocknum;
//    printf("In DfsWriteBlock\n");
    if (sb.valid == 0){
       printf("File system not open!\n");
       return DFS_FAIL;
    }
    if((blocknum<0)||(blocknum>sb.filesys_blocknum)){
       printf("Invalid blocknum = %d\n",blocknum);
       return DFS_FAIL;
    }
    if(Dfscheck(blocknum)== DFS_FAIL){
       printf("Error,DFS block %d not allocated!\n",blocknum);
       return DFS_FAIL;
     }
    
    ratio = sb.filesys_blocksize/(DISK_BLOCKSIZE);
    disk_blocknum=(blocknum*ratio);
    for(i=0;i<ratio;i++){
       bcopy((char *)((int)b+i*(DISK_BLOCKSIZE)),(char *)(&disk_b),DISK_BLOCKSIZE);
       if (DiskWriteBlock((uint32)(disk_blocknum + i),&disk_b) == DISK_FAIL){
           printf("Cannot write to block %d on disk!\n",disk_blocknum);
           return DISK_FAIL;
          }
    } 
    return (sb.filesys_blocksize);
}

int Dfscheck(int blocknum){
    int fbvnum;
    int bitnum;

  //  printf("In Dfscheck\n");
    if((blocknum<0)||(blocknum>sb.filesys_blocknum)){
       printf("Invalid blocknum = %d\n",blocknum);
       return DFS_FAIL;
    }
     fbvnum = (blocknum)/32;
     bitnum = (blocknum)%32;
    
    if(sb.valid == 0){
       printf("File system not open!\n");
       return DFS_FAIL;
      }
    if ((fbv[fbvnum] & (1<<bitnum))==0){
//       printf("dfs_block %d is allocated\n",blocknum);
       return DFS_SUCCESS;
      }
    return DFS_FAIL;
}
////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

int DfsInodeFilenameExists(char *filename) {
   int i;
   uint32 handle;
 //  printf("In DfsInodeFilename Exists\n");
   if (sb.valid == 0){
      printf("File system not open!\n");
      return DFS_FAIL;
   }
   for (i=0; i<sb.inodenum;i++){
       if(inodes[i].inuse == 1){
          if(dstrncmp(inodes[i].filename,filename,FILENAME_LENGTH) == 0){
            handle = i; 
            return handle;
            }
       }
    }
   return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.  
//-----------------------------------------------------------------

int DfsInodeOpen(char *filename) {
     int i,j;
     uint32 handle = DFS_FAIL;
  //   printf("In DfsInodeOpen\n");
     if(sb.valid == 0){
        printf("File system not open!\n");
        return DFS_FAIL;
     }
     if((handle = DfsInodeFilenameExists(filename))!= DFS_FAIL){
         printf("Open existing file!\n");
         return handle;
       }
     else{
      for(i=0;i<sb.inodenum;i++){
         if(inodes[i].inuse == 0){
           LockHandleAcquire(lock_inode);
            inodes[i].inuse = 1;
            inodes[i].filesize = 0;
            for(j=0;j<FILENAME_LENGTH;j++){
                inodes[i].filename[j]=filename[j];
             }
            handle = i;
           LockHandleRelease(lock_inode);
            return handle;
          }  
       }
    }
   printf("No free inodes!\n");
   return DFS_FAIL;
}

//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(int handle) {
    int i;
    dfs_block indirect_table;
    uint32 *table;
  //  printf("In DfsInodeDelete\n");
    if (sb.valid == 0){
       printf("File system not open!\n");
       return DFS_FAIL;
    }
    if((handle<0)||(handle>DFS_INODE_MAX_NUM)){
       printf("Invalid inode handle = %d\n",handle);
       return DFS_FAIL;
    }
     if(inodes[handle].inuse == 0){
       printf("Inode not allocated!\n");
       return DFS_FAIL;
    }
    LockHandleAcquire(lock_inode);
    //Free direct table then check if there is indirect address
    for(i=0;i<10;i++){
      if(inodes[handle].direct[i]!=0){
      if(DfsFreeBlock((int)inodes[handle].direct[i])==DFS_FAIL){
         printf("Cannot free blocks in direct table!\n");
         return DFS_FAIL;
      }
      inodes[handle].direct[i]=0;
     }
    }
   //Free the indirect table
    if(inodes[handle].indirect != 0){
      table =(uint32 *)(&indirect_table);
      DfsReadBlock(inodes[handle].indirect,&indirect_table);
        for(i=0;i<(sb.filesys_blocksize)/sizeof(uint32);i++){
           if(table[i]!=0){
             if (DfsFreeBlock((int)table[i])== DFS_FAIL){
                 printf("Cannot free blocks in indirect table\n");
                 return DFS_FAIL;
               }
           }
        }
       if(DfsFreeBlock((int)inodes[handle].indirect)==DFS_FAIL){
          printf("Cannot free blocks in indirect table\n");
       }
       inodes[handle].indirect=0;
     }
    inodes[handle].filesize = 0;
    for(i=0;i< FILENAME_LENGTH;i++){
        inodes[handle].filename[i]='\0';
    }
    inodes[handle].inuse = 0;
    LockHandleRelease(lock_inode);
    return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(int handle, void *mem, int start_byte, int num_bytes) {
    int i;
    int end_byte = start_byte+num_bytes-1;
    int byte_start,byte_end;
    int block_start,block_end;
    dfs_block block;
    int blocknum;
    int byte_read;

  //  printf("In DfsInodeReadBytes\n");
    if(sb.valid == 0){
      printf("File system not open!\n");
      return DFS_FAIL;
    }
    if((handle<0)||(handle>DFS_INODE_MAX_NUM)){
      printf("Invalid inode handle = %d\n",handle);
      return DFS_FAIL;
    }
    if(inodes[handle].inuse == 0){
      printf("Inode not allocated!\n");
      return DFS_FAIL;
    } 
    if((start_byte<0)||(num_bytes<0)||((end_byte)>inodes[handle].filesize)){
      printf("Invalid start_byte = %d  or num_byte= %d\n",start_byte,num_bytes);
      return DFS_FAIL;
    }
    
    block_start = (start_byte/sb.filesys_blocksize);
    byte_start = (start_byte%sb.filesys_blocksize);
    block_end = (end_byte/sb.filesys_blocksize);
    byte_end = (end_byte%sb.filesys_blocksize);
    if(block_start == block_end){
       blocknum = DfsInodeTranslateVirtualToFilesys(handle,block_start);
   //    printf("read from block(start=end) %d\n",blocknum);
       DfsReadBlock(blocknum,&block);
       bcopy((char *)((int)&block+byte_start),(char *)mem,num_bytes);
       return (num_bytes);
     }
    else{
       blocknum = DfsInodeTranslateVirtualToFilesys(handle,block_start);
    //   printf("read from block(1st) %d\n",blocknum);
       DfsReadBlock(blocknum,&block);
       byte_read = sb.filesys_blocksize-(byte_start);
       bcopy((char *)((int)&block+byte_start),(char *)mem,byte_read);
       for(i=(block_start)+1;i<(block_end);i++){
          blocknum = DfsInodeTranslateVirtualToFilesys(handle,i);
    //      printf("read from block(middle) %d\n",blocknum);
          DfsReadBlock(blocknum,&block);
          bcopy((char *)(&block),(char *)((int)mem+byte_read),sb.filesys_blocksize);
          byte_read +=sb.filesys_blocksize;
          } 
       blocknum = DfsInodeTranslateVirtualToFilesys(handle,block_end);
    //   printf("read from block(last) %d\n",blocknum);
       DfsReadBlock(blocknum,&block);
       bcopy((char *)(&block),(char *)((int)mem+byte_read),(byte_end)+1);
       byte_read +=(byte_end)+1;
        return (byte_read);
     }
       
}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------
int DfsAllocateCheck(int handle,int blocknum){
    uint32 *table;
    dfs_block indirect_table;
 
    if(blocknum<10){
       if(inodes[handle].direct[blocknum]==0){
          return DFS_FAIL;
       }
    }
    else{
       if(inodes[handle].indirect==0){
         return DFS_FAIL;
        }
       else{
          table =(uint32 *)(&indirect_table);
          DfsReadBlock(inodes[handle].indirect,&indirect_table);
          if(table[blocknum-10]==0){
             return DFS_FAIL;
          }
        }
    }
    return DFS_SUCCESS;
}    

int DfsInodeWriteBytes(int handle, void *mem, int start_byte, int num_bytes) {
    int i;
    int end_byte = start_byte+num_bytes-1;
    int byte_start,byte_end;
    int block_start,block_end;
    dfs_block block;
    int blocknum;
    int byte_write;

  //  printf("In DfsInodeWriteBytes\n");
    if(sb.valid == 0){
      printf("File system not opened!\n");
      return DFS_FAIL;
    }
    if((handle<0)||(handle>DFS_INODE_MAX_NUM)){
      printf("Invalid inode handle = %d\n",handle);
      return DFS_FAIL;
      }
    if(inodes[handle].inuse == 0){
      printf("Inode not allocated!\n");
      return DFS_FAIL;
     } 
    if((start_byte<0)||(num_bytes<0)||((end_byte)>(sb.filesys_blocknum*sb.filesys_blocksize))){
      printf("Invalid start_byte = %d  or num_byte= %d\n",start_byte,num_bytes);
      return DFS_FAIL;
    }
    
    block_start = (start_byte/sb.filesys_blocksize);
    byte_start = (start_byte%sb.filesys_blocksize);
    block_end = (end_byte/sb.filesys_blocksize);
    byte_end = (end_byte%sb.filesys_blocksize);

    if(block_start == block_end){
        //check if the virtual block table is valid
        if(DfsAllocateCheck(handle,block_start)!= DFS_FAIL){
           blocknum = DfsInodeTranslateVirtualToFilesys(handle,block_start);
         //  printf("write to block(start=end,noalloc) %d\n",blocknum);
           DfsReadBlock(blocknum,&block);
           bcopy((char *)mem,(char *)((int)&block+byte_start),num_bytes);
           DfsWriteBlock(blocknum,&block);
           }
        else{
           if((blocknum = DfsInodeAllocateVirtualBlock(handle,block_start))== DFS_FAIL){
               printf("Cannot allocate a virtual block\n");
               return DFS_FAIL;
              }
         //  printf("write to block(start=end alloc) %d\n",blocknum);
           DfsReadBlock(blocknum,&block);
           bzero((char *)&block,sizeof(dfs_block));
           bcopy((char *)mem,(char *)((int)&block+byte_start),num_bytes);
           DfsWriteBlock(blocknum,&block);
          }
       inodes[handle].filesize+=num_bytes;
       return (num_bytes);
     }
    else{
       if(DfsAllocateCheck(handle,block_start)!=DFS_FAIL){
          blocknum = DfsInodeTranslateVirtualToFilesys(handle,block_start);
        //  printf("write to block(1st,noalloc) %d\n",blocknum);
          DfsReadBlock(blocknum,&block);
          byte_write = sb.filesys_blocksize-(byte_start);
          bcopy((char *)mem,(char *)((int)&block+byte_start),byte_write);
          DfsWriteBlock(blocknum,&block);
         }
       else{
          if((blocknum = DfsInodeAllocateVirtualBlock(handle,block_start)) == DFS_FAIL){
              printf("Cannot allocate a virtual block!\n");
              return DFS_FAIL;
            }
        //  printf("write to block(1st,alloc) %d\n",blocknum);
          DfsReadBlock(blocknum,&block);
          bzero((char *)&block,sizeof(dfs_block));
          byte_write = sb.filesys_blocksize-(byte_start);
          bcopy((char *)mem,(char *)((int)&block+byte_start),byte_write);
          DfsWriteBlock(blocknum,&block);
         }
       for(i=(block_start)+1;i<(block_end);i++){
          if((DfsAllocateCheck(handle,i)!=DFS_FAIL)){
             blocknum = DfsInodeTranslateVirtualToFilesys(handle,i);
        //     printf("noalloc  ");
          }
          else{
             if((blocknum = DfsInodeAllocateVirtualBlock(handle,i)) == DFS_FAIL){
        //      printf("Cannot allocate a virtual block!\n");
              return DFS_FAIL;
              }
             }
       //      printf("write to block(middle) %d\n",blocknum);
             bzero((char *)&block,sizeof(dfs_block));
             bcopy((char *)((int)mem+byte_write),(char *)(&block),sb.filesys_blocksize);
             DfsWriteBlock(blocknum,&block);
             byte_write +=sb.filesys_blocksize;
         } 
       if(DfsAllocateCheck(handle,block_end)!=DFS_FAIL){  
          blocknum = DfsInodeTranslateVirtualToFilesys(handle,block_end);
       //   printf("write to block(last,noalloc) %d\n",blocknum);
          DfsReadBlock(blocknum,&block);
          bcopy((char *)((int)mem+byte_write),(char *)(&block),(byte_end)+1);
          DfsWriteBlock(blocknum,&block); 
          byte_write +=(byte_end)+1;
         }
       else{
          if((blocknum = DfsInodeAllocateVirtualBlock(handle,block_end)) == DFS_FAIL){
             printf("Cannot allocate a virtual block!\n");
             return DFS_FAIL;
            }
      //    printf("write to block(last alloc) %d\n",blocknum);
          DfsReadBlock(blocknum,&block);
          bzero((char *)&block,sizeof(dfs_block));
          bcopy((char *)((int)mem+byte_write),(char *)(&block),(byte_end)+1);
          DfsWriteBlock(blocknum,&block); 
          byte_write +=(byte_end)+1;
         }
          inodes[handle].filesize+=byte_write;
          return (byte_write);
     }
}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeFilesize(int handle) {
   //  printf("In DfsInodeFilesize\n");
     if (sb.valid == 0){
        printf("File system not opened\n");
        return DFS_FAIL;
        }
     if((handle<0)||(handle>DFS_INODE_MAX_NUM)){
        printf("Invalid inode handle = %d\n",handle);
        return DFS_FAIL;
        }
     if(inodes[handle].inuse == 0){
        printf("Inode not allocated!\n");
        return DFS_FAIL;
        }
     return (inodes[handle].filesize);
}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

int  DfsInodeAllocateVirtualBlock(int handle, int virtual_blocknum) {
     dfs_block indirect_table;
     int blocknum;
     uint32 *table;
     
   //  printf("In DfsInodeAllocateVirtualBlock\n");  
     if (sb.valid == 0){
        printf("File system not opened\n");
        return DFS_FAIL;
        }
     if((handle<0)||(handle>DFS_INODE_MAX_NUM)){
        printf("Invalid inode handle = %d\n",handle);
        return DFS_FAIL;
        }
     if(inodes[handle].inuse == 0){
        printf("Inode not allocated!\n");
        return DFS_FAIL;
        }
     if((virtual_blocknum<0)||(virtual_blocknum>(sb.filesys_blocksize/sizeof(uint32)+10))){
        printf("Invalid virtural_blocknum = %d\n",virtual_blocknum);
        return DFS_FAIL;
       }
     //check direct or indirect
     if(virtual_blocknum < 10){
       if((blocknum = DfsAllocateBlock()) == DFS_FAIL){
          printf("Cannot allocate block for direct table\n");
          return DFS_FAIL;
         }
        inodes[handle].direct[virtual_blocknum] =(uint32)blocknum;
        return blocknum;
       }
     else{
        table =(uint32 *)(&indirect_table);
        if((inodes[handle].indirect)!=0){
           DfsReadBlock(inodes[handle].indirect,(dfs_block *)table); 
          if((blocknum = DfsAllocateBlock()) == DFS_FAIL){
            printf("Cannot allocate block for direct table\n");
            return DFS_FAIL;
          }
          table[virtual_blocknum-10] =(uint32)blocknum;
          DfsWriteBlock(inodes[handle].indirect,(dfs_block *)table);
          return blocknum;
         }
         else{   
         if((blocknum = DfsAllocateBlock()) == DFS_FAIL){
            printf("Cannot allocate block for indirect table\n");
            return DFS_FAIL;
          }
          inodes[handle].indirect = (uint32)blocknum;
          bzero((char *)table,sb.filesys_blocksize); 
          if((blocknum = DfsAllocateBlock()) == DFS_FAIL){
             printf("Cannot allocate block for indirect address\n");
             return DFS_FAIL;
            }
          table[virtual_blocknum-10] = (uint32)blocknum;
          DfsWriteBlock(inodes[handle].indirect,(dfs_block *)table);
          return blocknum;
          }
      }
}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeTranslateVirtualToFilesys(int handle, int virtual_blocknum) {
       dfs_block indirect_table;
       uint32 *table;
       uint32 blocknum;

    //   printf("In DfsInodeTranslateVirtualToFilesys\n");  
       if (sb.valid == 0){
        printf("File system not opened\n");
        return DFS_FAIL;
        }
       if((handle<0)||(handle>DFS_INODE_MAX_NUM)){
        printf("Invalid inode handle = %d\n",handle);
        return DFS_FAIL;
        }
       if(inodes[handle].inuse == 0){
        printf("Inode not allocated!\n");
        return DFS_FAIL;
        }
       if((virtual_blocknum<0)||(virtual_blocknum>(sb.filesys_blocksize/sizeof(uint32)+10))){
        printf("Invalid virtural_blocknum = %d\n",virtual_blocknum);
        return DFS_FAIL;
       }
       //check direct or indirect
       if(virtual_blocknum<10){
        return (inodes[handle].direct[virtual_blocknum]);
        }
       else{
        if(inodes[handle].indirect == 0){
          printf("Indirect address table not exist!\n");
          return DFS_FAIL;
        }
        else{
          table = (uint32 *)(&indirect_table);
          DfsReadBlock(inodes[handle].indirect,&indirect_table);
          blocknum = table[virtual_blocknum-10];
          return (int)blocknum;
        }
      }
      return DFS_FAIL;
}
