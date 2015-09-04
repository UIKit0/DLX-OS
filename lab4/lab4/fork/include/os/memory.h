#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);
int RopAccessHandler(PCB *pcb);
//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree functions go here
uint32 MemorySetupPte (uint32 page);
uint32 MemoryInitPte( uint32 page);
uint32 MemoryAllocPage();
void MemoryFreePage (uint32 page);
void MemoryFreePte (uint32 pte);
uint32 MemoryPteToPage();
void  MemorySetFreemap(int,int);
void *malloc(PCB *currentPCB, int memsize);
int mfree(PCB *currentPCB, void *ptr);
void SetPara();
#endif	// _memory_h_
