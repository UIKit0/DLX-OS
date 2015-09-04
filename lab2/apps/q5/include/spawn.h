#ifndef __USERPROG__
#define __USERPROG__
#define BUFFER_SIZE 11
typedef struct circular_buffer {
  char buffer[BUFFER_SIZE];
  int head;
  int tail;
} circular_buffer;

#define PRODUCER_CONSUMER "Producer_Consumer.dlx.obj"
#define INJECTION_N2      "injection_n2.dlx.obj"
#define INJECTION_H2O     "injection_h2o.dlx.obj"
#define REACTION1         "reaction1.dlx.obj"
#define REACTION2         "reaction2.dlx.obj"
#define REACTION3         "reaction3.dlx.obj"
#endif
