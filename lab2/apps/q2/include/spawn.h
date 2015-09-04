#ifndef __USERPROG__
#define __USERPROG__
#define BUFFER_SIZE 11
typedef struct circular_buffer {
  char buffer[BUFFER_SIZE];
  int head;
  int tail;
} circular_buffer;

#define PRODUCER_CONSUMER "Producer_Consumer.dlx.obj"
#define PRODUCER 1
#define CONSUMER 0
#endif
