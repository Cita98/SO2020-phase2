#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "const.h"
#include "types_bikaya.h"
#include "handler.h"

// //Interrup line device
// #define INT_TIMER 		2
//
// //PER PROVE
// #define INT_DISK 3
// #define INT_TAPE 4
// #define INT_UNUSED 5
// #define INT_PRINTER 6
// #define INT_TERMINAL 7

typedef struct st_sem_dev {
    int disk[DEV_PER_INT];
    int tape[DEV_PER_INT];
    int network[DEV_PER_INT];
    int printer[DEV_PER_INT];
    int terminalR[DEV_PER_INT];
    int terminalT[DEV_PER_INT];
} st_sem_dev;

st_sem_dev sem_dev;

//Funzioni
void init_interrupt();
int checkDevLine(int intLine, int dev);
int setIOresult(int *key, unsigned int res);
//int getINT_LINE();

//Interrupt disk
void intDisk();

//Interrupt tape
void intTape();

//Interrupt network
void intNet();

//Interrupt printer
void intPrint();

//Interrupt terminal
void intTerm();

void aaadebugFc();

#endif
