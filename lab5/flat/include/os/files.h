#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"

#define FILE_MAX_OPEN_FILES 15
#define R 1
#define W 2
#define RW 3
int FileOpen(char *filename, char *mode_file);

int FileClose(int handle);

int FileWrite(int handle,void *mem, int num_bytes);

int FileRead(int handle, void *mem, int num_bytes);

int FileSeek(int handle, int num_bytes, int from_where);

int FileDelete(char *filename);

int convertModeToInt(char *mode);
#endif
