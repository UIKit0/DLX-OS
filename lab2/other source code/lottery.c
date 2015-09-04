//----------------------------------------------------------------------
//
//	ProcessSchedule
//
//	Schedule the next process to run.  If there are no processes to
//	run, exit.  This means that there should be an idle loop process
//	if you want to allow the system to "run" when there's no real
//	work to be done.
//
//	NOTE: the scheduler should only be called from a trap or interrupt
//	handler.  This way, interrupts are disabled.  Also, it must be
//	called consistently, and because it might be called from an interrupt
//	handler (the timer interrupt), it must ALWAYS be called from a trap
//	or interrupt handler.
//
//	Note that this procedure doesn't actually START the next process.
//	It only changes the currentPCB and other variables so the next
//	return from interrupt will restore a different context from that
//	which was saved.
//
//----------------------------------------------------------------------
void ProcessSchedule() {
	PCB *pcb = NULL;
	int i = 0;
	Link *l = NULL;
	float draw = 0;
	float lower = 0;
	float higher = 0;
	int sleeping = 0;
	int printed = -1;
	Link *l1 = NULL;
	PCB * pcb1 = NULL;
	int total_contestant_tickets = 0;

	srandom(ClkGetCurJiffies());


	dbprintf('p', "Now entering ProcessSchedule (cur=0x%x, %d ready)\n",
			(int)currentPCB, AQueueLength (&runQueue));


	//now always check the waitQueue
	if (!AQueueEmpty(&waitQueue)) {
		l = AQueueFirst(&waitQueue);
		while (l != NULL) {
			pcb = AQueueObject(l);
			if (pcb->flags & PROCESS_STATUS_SLEEPING){
				sleeping = 1;

				if (

						(ClkGetCurTime() - pcb->sleep_start) -
						(float) ((int)(ClkGetCurTime() - pcb->sleep_start)) < 10/(float)CLOCK_DEFAULT_RESOLUTION){
					dbprintf('5', "Process %d ", GetPidFromAddress(pcb));
					dbprintf('5', "has been asleep for %d seconds\n", (int)(ClkGetCurTime() - pcb->sleep_start));
				}
				if ((int)(ClkGetCurTime() - pcb->sleep_start) >= pcb->sleep_time){
					dbprintf('5', "\t\tNow waking up!!\n");
					ProcessWakeup(pcb);
				}
			}
			l = AQueueNext(l);
		}
	}



	// The OS exits if there's no runnable process.  This is a feature, not a
	// bug.  An easy solution to allowing no runnable "user" processes is to
	// have an "idle" process that's simply an infinite loop.
	if (AQueueEmpty(&runQueue) && !sleeping) {
		//		printf("Scheduler: empty queue and no sleeping\n");
		if (!AQueueEmpty(&waitQueue) ) {//wait queue not empty
			printf("FATAL ERROR: no runnable processes, but there are sleeping processes waiting!\n");
			l = AQueueFirst(&waitQueue);
			while (l != NULL) {
				pcb = AQueueObject(l);
				printf("Sleeping process %d: ", i++);
				printf("PID = %d\n", (int) (pcb - pcbs));
				l = AQueueNext(l);
			}
			exitsim();
		}
		printf("No runnable processes - exiting!\n");
		exitsim(); // NEVER RETURNS
	} else if(AQueueEmpty(&runQueue) && sleeping == 1){
		//dbprintf('4', "ProcessSchedule: switching in ProcessIdle\n");
		currentPCB = idle;
	} else {

		if (currentPCB != idle){//don't put the idle process on the run queue
//			dbprintf('4', "Moving front to end\n");
			AQueueMoveAfter(&runQueue, AQueueLast(&runQueue), AQueueFirst(&runQueue)); // Move front to end of queue
		}
		if (SCHEDULER == RR_SCHED){
			pcb = (PCB *)AQueueObject(AQueueFirst(&runQueue));// run process at head of queue
		}else if (SCHEDULER == LT_SCHED){

			if (AQueueLength(&runQueue) == 1){
				pcb = AQueueObject(AQueueFirst(&runQueue));

			} else if(AQueueLength(&runQueue) < 1){
				printf("\t\t************Shouldn't be running\n");
			} else{

				l1 = AQueueFirst(&runQueue);
				while (l1 != NULL) {
					pcb1 = AQueueObject(l1);
					total_contestant_tickets += pcb1->pnice;
					l1 = AQueueNext(l1);
				}



				{
					dbprintf('4', "\n----------------Lottery----------------\n");
					PrintRunQueue();

					dbprintf('4', "Lottery:\n\tTotal tickets = %d\n\tContestant tickets = %d\n\t", total_tickets,total_contestant_tickets);
					for (i = 0; i < PROCESS_MAX_PROCS; i++){

						if (!(pcbs[i].flags & PROCESS_STATUS_FREE)){
							if (printed % 3 == 2 ){
								dbprintf('4', "\n\t");
							}
							dbprintf('4', "(%d: %s - %d) ", GetPidFromAddress(&pcbs[i]),pcbs[i].name, pcbs[i].pnice);
							printed++;
						}
					}
					dbprintf('4', "\n");

				}

				draw = (float)random()/4294967295u;
				l = AQueueFirst(&runQueue);
				while (l != NULL) {
					if ( !(((PCB *)AQueueObject(l))->flags & PROCESS_STATUS_YIELDING)){
						pcb = AQueueObject(l);
//						dbprintf('4', "pcb = %d\n", GetPidFromAddress(pcb));
						higher += pcb->pnice/(float)total_contestant_tickets;
						if (draw >= lower && draw < higher){
							break;
						}
						lower = higher;
					} else{
//						dbprintf('4', "YIELD\n");
						ProcessSetStatus(((PCB *)AQueueObject(l)), PROCESS_STATUS_RUNNABLE);
					}
					l = AQueueNext(l);
				}
				dbprintf('4', "\t%.2f < ", lower);
				dbprintf('4', "%.2f ", draw);
				dbprintf('4', "< %.2f \n", higher);
				dbprintf('4', "\tSwitching to [%d] %s\n", GetPidFromAddress(pcb), pcb->name);
				dbprintf('4', "--------------------------------------\n\n");
			}

		}else {
			printf("ERROR: Must set SCHEDULER Macro in process.c\n");
		}

		//this is for pinfo
		currentPCB->jiffies += ClkGetCurJiffies() - currentPCB->start_time;
		if(currentPCB->pinfo == 1){
			printf(TIMESTRING1, GetCurrentPid());
			printf(TIMESTRING2, Jiffies2Seconds(currentPCB->jiffies));
			printf(TIMESTRING3, GetCurrentPid(), currentPCB->pnice );
		}

		//this replaces the currentPCB with the new process
		currentPCB = pcb;
		currentPCB->start_time = ClkGetCurJiffies();
		dbprintf('p', "About to switch to PCB 0x%x,flags=0x%x @ 0x%x\n",
				(int)pcb, pcb->flags, (int)(pcb->sysStackPtr[PROCESS_STACK_IAR]));
		// Clean up zombie processes here.  This is done at interrupt time
		// because it can't be done while the process might still be running
	}
	while (!AQueueEmpty(&zombieQueue)) {
		pcb = (PCB *) AQueueObject(AQueueFirst(&zombieQueue));
		dbprintf('p', "Freeing zombie PCB 0x%x.\n", (int)pcb);
		if (AQueueRemove(&(pcb->l)) != QUEUE_SUCCESS) {
			printf(
					"FATAL ERROR: could not remove zombie process from zombieQueue in ProcessSchedule!\n");
			exitsim();
		}
		ProcessFreeResources(pcb);
	}
	dbprintf('p', "Leaving ProcessSchedule (cur=0x%x)\n", (int)currentPCB);
}