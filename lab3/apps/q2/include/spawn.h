#ifndef __USERPROG__
#define __USERPROG__

typedef struct missile_code {
  int numprocs;
  char really_important_char;
} missile_code;

#define FILENAME_TO_RUN_0 "injection_n2.dlx.obj"
#define FILENAME_TO_RUN_1 "injection_h2o.dlx.obj"
#define FILENAME_TO_RUN_2 "reaction1.dlx.obj"
#define FILENAME_TO_RUN_3 "reaction2.dlx.obj"
#define FILENAME_TO_RUN_4 "reaction3.dlx.obj"

#ifndef NULL
#define NULL (void *)0x0
#endif

#endif
