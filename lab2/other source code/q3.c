#include "lab4-api.h"
#include "usertraps.h"
#include "misc.h"

#include "../include/q3.h"
void wait(int n);
void eat(int *food);
void test(int i);
void pickup(int i);
void putdown(int i);
phil *mem;        // Used to access missile codes in shared memory page
char mouthstate[4];

void main (int argc, char *argv[])
{

	uint32 h_mem;            // Handle to the shared memory page
	sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int phil_num = -1;
	int food = 0;
	int left = -1;
	int right = -1;

	if (argc != 4) {
		Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n");
		Exit();
	}
//	dstrcpy(mouthstate[0], "HUNGRY");
	mouthstate[0] = 'H';
//	mouthstate[0][1] = '\0';
//	dstrcpy(mouthstate[1], "EATING");
	mouthstate[1] = 'E';
//	mouthstate[1][1] = '\0';
//	dstrcpy(mouthstate[2], "THINKING");
	mouthstate[2] = 'T';
//	mouthstate[2][1] = '\0';
	mouthstate[3] = '\0';
	// Convert the command-line strings into integers for use as handles
	h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
	s_procs_completed = dstrtol(argv[2], NULL, 10);
	phil_num = dstrtol(argv[3], NULL, 10);
	left = phil_num;
	right = (phil_num+1)%5;
	// Map shared memory page into this process's memory space
	if ((mem = (phil *)shmat(h_mem)) == NULL) {
		Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}


	food = 100;
	while (food > 0 ){

		//this needs to be synchronized
		// getchopsticks
		pickup(phil_num);
		eat(&food);
		mem->eaten[phil_num] =  1;
		Printf("%d is %c, right is %c, left is %c\n", phil_num, mouthstate[mem->state[phil_num]], mouthstate[mem->state[(phil_num + 1) % 5]], mouthstate[mem->state[(phil_num+4)%5]]);
		putdown(phil_num);
		wait(1000 * phil_num);
	}
	// Now print a message to show that everything worked


	// Signal the semaphore to tell the original process that we're done
	Printf("q3: PID %d is complete.\n", getpid());
	if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}
}
void test(int i){
	if ((mem->state[(i+4)%5] != EATING) &&
		(mem->state[i] == HUNGRY) &&
		(mem->state[(i + 1) % 5] != EATING))
	{
		lock_acquire(mem->wait_locks[i]);
		mem->state[i] = EATING;
		cond_signal(mem->self[i]);
		lock_release(mem->wait_locks[i]);
	}
}
void putdown(int i){
	lock_release(mem->wait_locks[i]);
	mem->state[i] = THINKING;
	test((i+4)%5);
	test((i+1)%5);
}
void pickup(int i){
	lock_acquire(mem->wait_locks[i]);
	mem->state[i] = HUNGRY;
	test(i);
	if (mem->state[i] != EATING){
		cond_wait(mem->self[i]);
	}
}
void wait(int n){
	float value = 2.0;
	float inter = 0;
	int i = 0;

	for (i = 0; i < n; i++){
		value = value * value;
		inter = value + value;
		value = inter + value;
	}
}

void eat(int *food){

	(*food)--;
}