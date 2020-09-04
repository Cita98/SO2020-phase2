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

//Riconosco la system call
int get_SysNumb(pcb_t* curr_proc);

//Riconosco i 3 parametri di input
void get_param(p_u_int *param, pcb_t* curr_proc);


//SYS 1
void get_cpu_time(unsigned int* user, unsigned int* kernel, unsigned int* wallclock);

//SYS 2
int create_process(state_t *state_p, int priority, void** cpid);

//SYS 3
int terminate_process(void* pid);

//SYS 4
void verhogen(int *semaddr);

//SYS 5
void passeren(int *semaddr);

//SYS 6
int do_io(unsigned int command, unsigned int* devRegister, int subdevice);
//Trova la linea di interrupt (pline) e il tipo di device ptype dato un registro
void fintTYPEandLINE(int* ptype, int* pline, unsigned int* preg );
//Blocca il processo in attesa della risposta dal device
void blockProcAtDev(int type, int line, int subdevice);

//SYSCALL 7
int spec_passup(int type, state_t* old, state_t* new);

//SYSCALL 8 
void get_pid_ppid(void** pid, void** ppid);


#endif
