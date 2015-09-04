#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"

#define FILE_MAX_OPEN_FILES 15

#define R 1
#define W 2
#define RW 3
#define OR 4
#define OW 2
#define OX 1
#define UR 32
#define UW 16
#define UX 8
#define OK 1
#define FILE 1
#define DIR 2

#define READ 0x4
#define WRITE 0X2
#define EXECUTE 0X1

int FileOpen(char *filename, char *mode);

int FileClose(int handle);

int FileWrite(int handle,void *mem, int num_bytes);

int FileRead(int handle, void *mem, int num_bytes);

int FileSeek(int handle, int num_bytes, int from_where);

int FileDelete(char *path);

int FileLink(char *srcpath,char *dstpath);

int MkDir(char *path, int permissions);

int RmDir(char *path);

#endif
