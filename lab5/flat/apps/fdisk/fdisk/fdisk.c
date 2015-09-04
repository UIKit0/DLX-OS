#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
//dfs_inode inodes[DFS_INODE_MAX_NUM];
//uint32 fbv[DFS_FBV_MAX_NUM];

int diskblocksize;  // These are global in order to speed things up
int disksize;      // (i.e. fewer traps to OS to get the same number)
int num_filesystem_blocks;  

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory
  int i,j;
  dfs_block block;
  dfs_inode inode;
  uint32 fbv = 0xffffffff;
//  char b;

  //Initializations and argc check
    disksize = disk_size();
    diskblocksize = disk_blocksize();
    num_filesystem_blocks = (disksize)/(DFS_BLOCKSIZE);

    dfs_invalidate();
    sb.filesys_blocksize = DFS_BLOCKSIZE;
    sb.filesys_blocknum = num_filesystem_blocks;
    sb.filesys_inodestart = FDISK_INODE_BLOCK_START;
    sb.inodenum = FDISK_NUM_INODES;
    sb.filesys_fbvstart = FDISK_FBV_BLOCK_START;
    sb.fbvnum = FDISK_NUM_FBV;
    sb.filesys_datastart = FDISK_DATA_BLOCK_START;
        
    bzero((char *)(&inode),sizeof(dfs_inode));
    bzero((char *)(&block),sizeof(dfs_block));
  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  // sb.valid = 0

  // Make sure the disk exists before doing anything else
  if (disk_create() == DISK_FAIL){
      Printf("Error:The disk does not exist!\n");
      Exit();
     }

  // Write all inodes as not in use and empty (all zeros)
   for(j=0;j<FDISK_INODE_NUM_BLOCKS;j++){
     for(i=0;i<(DFS_BLOCKSIZE)/(sizeof(dfs_inode));i++){
         bcopy((char *)(&inode),(char *)((int)(&block)+(i*sizeof(dfs_inode))),sizeof(dfs_inode));
        }
        FdiskWriteBlock((uint32)(FDISK_INODE_BLOCK_START+j),&block);
        }
  // Next, setup free block vector (fbv) and write free block vector to the disk
   for(j=0;j<FDISK_FBV_NUM_BLOCKS;j++){
     for(i=0;i<(DFS_BLOCKSIZE)/(sizeof(uint32));i++){
         bcopy((char *)(&fbv),(char *)((int)(&block)+i*sizeof(uint32)),sizeof(uint32));
     }
        FdiskWriteBlock((uint32)(FDISK_FBV_BLOCK_START+j),&block);
     } 

     bzero((char *)(&block),sizeof(dfs_block));
     fbv = 0x00000000;//used to present 32 blocks has been taken up
       bcopy((char *)(&fbv),(char *)(&block),sizeof(uint32));
     fbv = 0xffffe000; //the last 12 blocks taken up
       bcopy((char *)(&fbv),(char *)((int)(&block)+sizeof(uint32)),sizeof(uint32));
     fbv = 0xffffffff;
     for(i=2;i<(DFS_BLOCKSIZE)/(sizeof(uint32));i++){
         bcopy((char *)(&fbv),(char *)((int)(&block)+i*sizeof(uint32)),sizeof(uint32));
     }
     FdiskWriteBlock((uint32)(FDISK_FBV_BLOCK_START),&block);     
     
  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
     sb.valid = 1;
     bzero((char *)(&block),sizeof(dfs_block));
     bcopy((char *)(&sb),(char *)((int)(&block)+diskblocksize),sizeof(dfs_superblock));
     FdiskWriteBlock((uint32)(FDISK_BOOT_FILESYSTEM_BLOCKNUM),&block);
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
  // STUDENT: put your code here
  int i;
  int ratio;
  uint32 diskblocknum;
  char diskdata[diskblocksize];
  
  ratio = (DFS_BLOCKSIZE)/(diskblocksize);
  diskblocknum = ratio*blocknum;
 // Printf("In FdiskWriteBlock,diskblocknum= %d\n",diskblocknum);
  for(i=0;i<ratio;i++){
      bcopy((char *)((int)b+i*diskblocksize),diskdata,diskblocksize);
      if (disk_write_block(diskblocknum+i,diskdata)== DISK_FAIL){
          Printf("Cannot wite to disk\n");
          return DISK_FAIL;
         }
  }
  return DISK_SUCCESS;
}

