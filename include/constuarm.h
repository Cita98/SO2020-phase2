#ifndef _CONSTUARM_H
#define _CONSTUARM_H

#include "uarm/libuarm.h"
#include "uarm/arch.h"
#include "uarm/uARMconst.h"

//Utili
#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 24)))

//Rinominazione per unificare i nomi nelle due architetture

	#define SYSCALL_NEWAREA 		SYSBK_NEWAREA
	#define SYSCALL_OLDAREA 		SYSBK_OLDAREA

	//Registri state_t
	#define stack_pointer 			sp
	#define prog_counter 			pc

//Gestione device esterni
#define DEV_REGS_BASE 0x00000040

#endif
