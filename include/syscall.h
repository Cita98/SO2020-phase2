#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "const.h"
#include "types_bikaya.h"
#include "handler.h"
#include "scheduler.h"
#include "pcb.h"
#include "asl.h"

//Utilis
void init_syscall();
int get_SysNumb(pcb_t* curr_proc);
//int get_SysNumb(state_t* curr_proc);

//SYS 3
void terminate_process(pcb_t* curr_proc);

#endif