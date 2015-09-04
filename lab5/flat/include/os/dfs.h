#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

void DfsModuleInit();
void DfsInvalidate();
int DfsOpenFileSystem();
int DfsCloseFileSystem();
int DfsAllocateBlock();
int DfsFreeBlock(int blocknum);
int DfsReadBlock(int blocknum, dfs_block *b);
int DfsWriteBlock(int blocknum,dfs_block *b);
int DfsInodeFilenameExists(char *filename);
int DfsInodeOpen(char *filename);
int DfsInodeDelete(int handle);
int DfsInodeReadBytes(int handle,void *mem, int start_byte, int num_bytes);
int DfsInodeWriteBytes(int handle,void *mem, int start_byte, int num_bytes);
int DfsInodeFilesize(int handle);
int DfsInodeAllocateVirtualBlock(int handle, int virtual_blocknum);
int DfsInodeTranslateVirtualToFilesys(int handle,int virtual_blocknum);
int Dfscheck(int blocknum);
#endif
