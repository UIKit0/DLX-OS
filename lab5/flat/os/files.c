#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.
lock_t lock_file;
static file_descriptor files[FILE_MAX_OPEN_FILES];
// STUDENT: put your file-level functions here
void FileModuleInit(){
     int i,j;
     lock_file = LockCreate();
  // printf("In FileModuleInit\n"); 
   for(i=0;i<FILE_MAX_OPEN_FILES;i++){
       for(j=0;j<FILENAME_LENGTH;j++){
          files[i].filename[j]='\0';
          }
       files[i].inode = -1;
       files[i].position = -1;
       files[i].endflag = -1;
       files[i].mode = 0;
       files[i].pid = -1;
      files[i].inuse = 0;
   }
}
     

int FilenameExists(char *filename){
    int i;
   // printf("In FilenameExists\n");
    
    for(i=0;i<FILE_MAX_OPEN_FILES;i++){
       if(files[i].inuse != 0){
          if(dstrncmp(files[i].filename,filename,FILENAME_LENGTH)==0){
         //    printf("find the file!\n");
             return i;
           }
       }
    }
  return DFS_FAIL;
}
int convertModeToInt(char *mode)
{
  int i;
  int md =0;

  for(i=0;mode[i]!='\0';i++)
  {
    switch(mode[i])
    {
      case 'r':
      case 'R':
        md |=R;
      break;
      case 'w':
      case 'W':
        md |=W;
      break;
        default:return 0;
   }
  }
  return md;
} 
  
int FileOpen(char *filename,char *mode_file){
    int handle;
    int i,j;
    int mode;
 // printf("InFileOpen, mode = %s\n",mode);
    if(*filename == '\0'){
       printf("Error: Invalid name\n");
       return FILE_FAIL;
    }
    mode = convertModeToInt(mode_file)+convertModeToInt(mode_file+1);
  //  printf("mode is %d\n",mode);
    if((mode!=R)&&(mode!=W)&&(mode!=RW)){
  //     printf("mode is %d\n",mode);
       printf("Error : Invalid mode\n");
       return FILE_FAIL;
      }
    LockHandleAcquire(lock_file);
    if((handle = FilenameExists(filename))!= DFS_FAIL){//filename found
        if(files[handle].inuse == 0){
          printf("Fatal Error: System Mistake");
          LockHandleRelease(lock_file);
          return FILE_FAIL;
        }
        if(files[handle].pid != GetCurrentPid()){//not the current process
            if(files[handle].pid != -1){// not closed file
              printf("Fail to Open, Opened by another process!\n");
              LockHandleRelease(lock_file);
              return FILE_FAIL;
            }
            else{// closed file
              files[handle].pid = GetCurrentPid();
              files[handle].mode = mode;
              LockHandleRelease(lock_file);
              return handle;
            }
        }
        else{ // current process
        printf("File open again!\n");
        files[handle].mode = mode;
        LockHandleRelease(lock_file);
        return handle;
     }
    }
    else{// filename not found
      if((handle=DfsInodeOpen(filename))== DFS_FAIL){
          printf("Fail to allocate an Inode!\n");
          LockHandleRelease(lock_file);
          return FILE_FAIL;
      }
      for(i=0;i<FILE_MAX_OPEN_FILES;i++){
          if(files[i].inuse == 0){
             files[i].inuse =1;
             files[i].pid = GetCurrentPid();
             files[i].mode = mode;
             files[i].position = 0;
             files[i].endflag = 0;
             files[i].inode = handle;
             for(j=0;j<FILENAME_LENGTH;j++){
                 files[i].filename[j]=filename[j];
             }
             LockHandleRelease(lock_file);
             return i;
         }
    }
  } 
    printf("Error: All file_descriptors taken up!\n");
    LockHandleRelease(lock_file);  
    return FILE_FAIL;
}
int FileClose(int handle){
// printf("In Fileclose\n");
    if((handle<0)||(handle>=FILE_MAX_OPEN_FILES)){
        printf("Error: Invalid file handler!\n");
        return FILE_FAIL;
    }
    files[handle].pid = -1;
    return FILE_SUCCESS;
}

int FileRead(int handle, void *mem, int num_bytes){
    int eof;
    int inode;
    int start_byte;
    int bytes_num;
    int mode;

// printf("In FileRead\n");
    if((handle<0)||(handle>=FILE_MAX_OPEN_FILES)){
       printf("Error: Invalid file handler!\n");
       return FILE_FAIL;
     }
    if(files[handle].pid != GetCurrentPid()){
       printf("Error:Please Open before read\n");
       return FILE_FAIL;
     }
    mode = files[handle].mode;
  //  printf("mode is %d\n",mode);
    if((mode!=R)&&(mode!=RW)){
       printf("Error:No permission to read\n");
       return FILE_FAIL;
     }
    if(num_bytes>FILE_MAX_READWRITE_BYTES){
       printf("Error:Reading more than 4096 bytes\n");
       return FILE_FAIL;
    }
    start_byte = files[handle].position;
    inode = files[handle].inode;
    if((eof=DfsInodeFilesize(inode))==DFS_FAIL){
        printf("Error: Fail to get the filesize from file %d\n",handle);
        return FILE_FAIL;
    }
//  printf("eof = %d\n",eof);
//  printf("files[handle].position = %d\n",files[handle].position); 
    if((files[handle].position+num_bytes)> eof){
       printf("Error: Read beyound end!\n");
       return FILE_FAIL;
    }
    if((bytes_num=DfsInodeReadBytes(inode,mem,start_byte,num_bytes))==DFS_FAIL){
      printf("Error:Fail to read from file %d\n",handle);
      return FILE_FAIL;
    }
    files[handle].position+=bytes_num;
    if(files[handle].position>=(eof)){
       files[handle].endflag = 1;
    }
    return (bytes_num);
}

int FileWrite(int handle, void *mem, int num_bytes){
    int start_byte;
    int bytes_num;
    int inode;
    int mode;
    

//printf("In FileWrite\n");
    if((handle<0)||(handle>=FILE_MAX_OPEN_FILES)){
       printf("Error: Invalid file handler!\n");
       return FILE_FAIL;
    }
    if(files[handle].pid != GetCurrentPid()){
       printf("Error:Please Open before write\n");
       return FILE_FAIL;
    }
    mode = files[handle].mode;
   // printf("mode is %d\n",mode);
    if((mode!=W)&&(mode!=RW)){
     // printf("pid = %d, inode =%d, position = %d,endflag = %d,inuse=%d,filename=%s\n",files[handle].pid,files[handle].inode,files[handle].position,files[handle].endflag,files[handle].inuse,files[handle].filename); 
      printf("Error: No permission to write\n");
      return FILE_FAIL;
    }
    start_byte = files[handle].position;
    inode = files[handle].inode;
    if((bytes_num=DfsInodeWriteBytes(inode,mem,start_byte,num_bytes))==DFS_FAIL){
      printf("Error:Fail to write to file %d\n",handle);
      return FILE_FAIL;
    }
    files[handle].position+=bytes_num;
    return (bytes_num);
}

int FileSeek(int handle, int num_bytes, int from_where){
    int eof;

//printf("In FileSeek\n");
    if((handle<0)||(handle>=FILE_MAX_OPEN_FILES)){
       printf("Error: Invalid file handler!\n");
       return FILE_FAIL;
    }
    if(files[handle].pid != GetCurrentPid()){
       printf("Error:Please Open before seek\n");
       return FILE_FAIL;
    }
    if(from_where == FILE_SEEK_CUR){
        files[handle].position+=num_bytes;
        files[handle].endflag = 0;
        return FILE_SUCCESS;
    }
    else if(from_where == FILE_SEEK_SET){
        files[handle].position=num_bytes;
        files[handle].endflag = 0;
        return FILE_SUCCESS;
    }  
    else if(from_where == FILE_SEEK_END){
        if((eof=DfsInodeFilesize(files[handle].inode))==DFS_FAIL){
           printf("Error: Cannot return filesize for file %d\n",handle);
           return FILE_FAIL;
        }
        files[handle].position+=eof;
        files[handle].endflag = 0;
        return FILE_SUCCESS;
    }
    return FILE_FAIL;
}

int FileDelete(char *filename){
    int handle;
    int i;
    if((handle = FilenameExists(filename))== DFS_FAIL){
       printf("No files found!\n");
       return FILE_FAIL;
    }
    for(i=0;i<FILENAME_LENGTH;i++){
        files[handle].filename[i] = '\0';
        }
    if(DfsInodeDelete(files[handle].inode)== DFS_FAIL){
        printf("Error:Cannot Delete the inode %d for handle %d!\n",files[handle].inode,handle);
      return FILE_FAIL;
      }
       files[handle].position = -1;
       files[handle].endflag = -1;
       files[handle].mode = 0;
       files[handle].pid = -1;
       files[handle].inode = -1;
       files[handle].inuse = 0;
    return FILE_SUCCESS;
}
