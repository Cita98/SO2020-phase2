#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "const.h"
#include "types_bikaya.h"
#include "handler.h"

//Interrup line device
#define INTERVAL_TIMER 		2

//PER PROVE
#define INT_DISK 3
#define INT_TAPE 4
#define INT_UNUSED 5   
#define INT_PRINTER 6
#define INT_TERMINAL 7

//Funzioni
void init_interrupt();
int getINT_LINE();

#endif