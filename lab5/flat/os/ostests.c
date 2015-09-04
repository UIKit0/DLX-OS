#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"

void RunOSTests() {
  // STUDENT: run any os-level tests here

  char filename1[12] = "file1.c";
  char filename2[12] = "file3.c";
  int handle = -1;
  int result;
  int i = 0;
  dfs_block b1;
  dfs_block b2;
  int blocknum;
  char mem[30] = "XYJSDIGKENGIDSEGODHSKEOGODIUH";
  char data1[1000];
  char data2[10000];
 
/*  handle = DfsAllocateBlock();
  printf("Allocate file system block %d\n",handle);
 
  if((result=DfsFreeBlock(handle))== DFS_FAIL){
  printf("Fail to free file system block %d\n",handle);
  }

  handle = DfsAllocateBlock();
  printf("Allocate file system block %d\n",handle);

  bcopy((char *)mem,(char *)&b1,30);
  if((result=DfsWriteBlock(handle,&b1))== DFS_FAIL){
  printf("Fail in DfsWriteBlock to write to block %d\n",handle);
  }
  else{
  printf("Write to block %d %d bytes(should be 512)\n",handle,result);
  }
 
  if((result=DfsReadBlock(handle,&b2))== DFS_FAIL){
  printf("Fail in DfsReadBlock to read from block %d\n",handle);
  }
  else{
     printf("Reading back:");
     for(i=0;i<30;i++){
        printf("%c",b2.data[i]);
     }
     printf("\n");
  }
  
  handle = DfsInodeOpen(filename1);
  if (handle != DFS_FAIL){
        printf("%s successfully opened with inode number %d\n",filename1,handle);
  }
  handle = DfsInodeFilenameExists(filename1);
  printf("FilenameExists:the inode handle for %s is %d\n",filename1,handle);

  blocknum = DfsInodeAllocateVirtualBlock(handle, 5);
  printf("In DfsInodeAllocateVBLOCK:filesys blocknum for vblock 5 %d\n",blocknum);
  blocknum = DfsInodeAllocateVirtualBlock(handle,9);
  printf("In DfsInodeAllocateVBLOCK:filesys blocknum for vblock 9 %d\n",blocknum);
  blocknum = DfsInodeAllocateVirtualBlock(handle,11);
  printf("In DfsInodeAllocateVBLOCK:filesys blocknum for vblock 11 %d\n",blocknum);

  blocknum = DfsInodeTranslateVirtualToFilesys(handle,5);
  printf("In DfsInodeTranslate: filesysblocknum for vblock 5 %d\n",blocknum);
  blocknum = DfsInodeTranslateVirtualToFilesys(handle,11);
  printf("In DfsInodeTranslate: filesysblocknum for vblock 11 %d\n",blocknum);

  if(DfsInodeDelete(handle)!= DFS_FAIL){
        printf("Successfully delete inode %d\n",handle);
  }

  if(handle = DfsInodeFilenameExists(filename1)!= DFS_FAIL){
  printf("Error: File %s still exists and the handle is %d\n",filename1,handle);
  }
*/
  handle = DfsInodeOpen(filename2);
  if(handle != DFS_FAIL){
         printf("%s successfully opened with inode number %d\n",filename2,handle);
   }
  for(i=0;i<1000;i++){
    data1[i]='A';
  }
  for(i=0;i<10000;i++){
    data2[i]='H';
  }
  
  DfsInodeWriteBytes(handle,(void *)mem,20,30);
  DfsInodeWriteBytes(handle,(void *)data1, 96, 526);
  DfsInodeWriteBytes(handle,(void *)data2, 1020, 7000);

  for (i=0; i< 30;i++){
         mem[i] = 'C';
  }
  for(i=0;i<1000;i++){
        data1[i]='D';
  }
  for (i=0;i<10000;i++){
        data2[i]='F';
  }
  DfsInodeReadBytes(handle,(void *)mem, 20, 30);
  mem[29] = '\0';
  printf("Mem is now %s \n",mem);
  
  DfsInodeReadBytes(handle,(void *)data1,96,526);
  printf("data1 now:\n");
  for(i=0;i<1000;i++){
  printf("%c",data1[i]);
  }
  printf("\n");
  DfsInodeReadBytes(handle,(void *)data2,1020,7000);
  printf("data2 now:\n");
  for(i=0;i<10000;i++){
  printf("%c",data2[i]);
  }
  printf("\n");
 
}
