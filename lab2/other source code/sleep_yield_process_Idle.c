//--------------------------------------------------------
// ProcessSleep assumes that it will be immediately
// followed by a call to ProcessSchedule (in traps.c).
//--------------------------------------------------------
void ProcessUserSleep(int seconds) {
	ProcessSetStatus(currentPCB, PROCESS_STATUS_SLEEPING);
	dbprintf('5', "Putting process %d to sleep for %d seconds\n", GetPidFromAddress(currentPCB), seconds);
	ProcessSuspend(currentPCB);

	currentPCB->sleep_start = ClkGetCurTime();
	currentPCB->sleep_time = seconds;

	/*puts the current process to sleep for the specified number of seconds.
	 * update a sleeping process's no of tickets properly when it is woken up.
	 * update ProcessSchedule to look for any "autowake" processes that should be woken up after a given number of jiffies.
	 * update the ProcessSchedule function to NOT update the no of tickets of a process marked as sleeping.
	 */
}

//-----------------------------------------------------
// ProcessYield simply marks the currentPCB as yielding.
// This should immediately be followed by a call to
// ProcessSchedule (in traps.c).
//-----------------------------------------------------
void ProcessYield() {
	//Set yielding
	//	printf("ProcessYield()\n");
	ProcessSetStatus(currentPCB, PROCESS_STATUS_YIELDING);
}

void ProcessIdle() {
	while(1);
}

void PrintRunQueue(){
	Link *l = NULL;
	PCB * pcb = NULL;
	if (!AQueueEmpty(&runQueue)) {
		dbprintf('4', "Contestantants: ");
		l = AQueueFirst(&runQueue);
		while (l != NULL) {
			pcb = AQueueObject(l);
			dbprintf('4', "%d ", GetPidFromAddress(pcb));
			l = AQueueNext(l);
		}
		dbprintf('4', "\n");
	}
}