#include "lab4-api.h"
#include "usertraps.h"
#include "misc.h"

#include "../include/q5.h"

void rx1(void);
void rx2(void);
void rx3(void);
void water(void);
void sulfate(void);

atmosphere *mem;        // Used to access missile codes in shared memory page

void main (int argc, char *argv[])
{

	uint32 h_mem;            // Handle to the shared memory page
	sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int process_type = -1;


	if (argc != 4) {
		Printf("Usage: ");
		Printf(argv[0]);
		Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore> <RX_type>\n");
		Exit();
	}

	// Convert the command-line strings into integers for use as handles
	h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
	s_procs_completed = dstrtol(argv[2], NULL, 10);
	process_type = dstrtol(argv[3], NULL, 10);

	// Map shared memory page into this process's memory space
	if ((mem = (atmosphere *)shmat(h_mem)) == NULL) {
		Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}

	if (process_type == RX1){
		rx1();
	}
	else if (process_type ==  RX2) {
		rx2();
	}
	else if (process_type == RX3) {
		rx3();
	}
	else if (process_type == WATER_RX) {
		water();
	}
	else if (process_type == SULFATE_RX) {
		sulfate();
	}else {
		Printf("q5: Invalid process type\n");
	}


	// Signal the semaphore to tell the original process that we're done
	Printf("q5: PID %d is complete.\n", getpid());
	if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}
}

void rx1(void){
	int i;
	for (i = 0; i < mem->original_water_num; i += 2){
		sem_wait(mem->water);
		sem_wait(mem->water);
		sem_signal(mem->hydrogen);
		sem_signal(mem->hydrogen);
		sem_signal(mem->oxygen);
		Printf("Consume 2 water molecules\n");
	}

}
void rx2(void){
	int i;
	for (i = 0; i < mem->original_sulfate_num; i++){
		sem_wait(mem->sulfate);
		sem_signal(mem->oxygen);
		sem_signal(mem->sulfer_dioxide);
		Printf("Consuming a sulfate molecule\n");
	}
}
void rx3(void){
	int i;
	int limit = 0;

	if(mem->original_water_num < mem->original_sulfate_num){
		limit = mem->original_water_num;
	} else {
		limit = mem->original_sulfate_num;
	}

	for (i = 0; i < limit; i++){
		sem_wait(mem->hydrogen);
		sem_wait(mem->oxygen);
		sem_wait(mem->sulfer_dioxide);
		Printf("Created H2SO4 %d\n", i+1);
	}


}
void water(void){
	int i;
	for (i = 0; i < mem->original_water_num; i += 2){
		sem_signal(mem->water);
		sem_signal(mem->water);
		Printf("Injected a total of %d water molecules\n", i+2);
	}
}

void sulfate(void){
	int i;
	for (i = 0; i < mem->original_sulfate_num; i ++){
		sem_signal(mem->sulfate);
		Printf("Injected a total of %d sulfate molecules\n", i+1);
	}

}