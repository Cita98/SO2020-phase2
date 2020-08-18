#ifndef _UTILIS_H
#define _UTILIS_H

#include "const.h"
#include "types_bikaya.h"
#include "pcb.h"
#include "scheduler.h"

//Funzioni ausiliarie
	void cp_state(state_t* scr_state, state_t* dst_state);

//Funzioni di inizializzazione
	void init_ProcState(pcb_t* new_Pcb);

//Gestione timer
	unsigned int get_TimeScale(int ms);
	void setIT_TIMER(int ms);
		
		
#endif