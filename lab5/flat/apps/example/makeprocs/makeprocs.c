#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
  int i;                          // Loop index variable
  int handle;
  char *filename ="file_name1";
  char *mode1 = "w";
  char *mode2 = "r";
  char *mode3 = "rw";
static  char mem[5000];

  if((handle = file_open(filename,"w"))== -1){
     Printf("In user Program, fail to open file\n");
    }
  for(i=0;i<5000;i++){
    mem[i]='B';
  }
  Printf("What is in memory\n");
  for(i=0;i<5000;i++){
    Printf("%c",mem[i]);
  }
  Printf("\n");
//  Printf("mem address 0x%x, mode address 0x%x\n",mem,mode1);
//  Printf("mode1 for w is %s\n",mode1);
  Printf("Write to file_name1 4096 'B'\n");
  if((file_write(handle,mem, 4096))== -1){
     Printf("In user Program, fail to write to file\n");
   }
  for(i=0;i<5000;i++){
     mem[i]='A';
   }
  Printf("Change 'B' to 'A' in memory\n");
  if(file_close(handle)== -1){
     Printf("In user Program, fail to close the file\n");
   }
  if(file_open(filename,mode2)==-1){
     Printf("In user Program, fail to open the file\n");
  }
  if(file_seek(handle,0,1)==-1){
    Printf("In user Program,fail to seek file\n");
  }
  if((file_read(handle,mem,4096))== -1){
     Printf("In user Program, fail to read file\n");
   }
  Printf("Read 4096 bytes from file_name1\n");
  Printf("What is in memory now?\n");
  for(i=0;i<5000;i++){
     Printf("%c",mem[i]);
   }
  Printf("\n");
 if(file_delete(filename)== -1){
     Printf("In user Program, fail to delete file\n");
 }

}
