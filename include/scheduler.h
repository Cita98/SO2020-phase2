#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "listx.h"
#include "pcb.h"
#include "utilis.h"
#include "const.h"

//SCHEDULING

	void scheduler();
	void aging(); //Aumenta la priorità dei processi in coda

//FUNZIONI PER VARIABILI CONDIVISE

	void initScheduler(); //Inizializza la ready queue
	void updateCurrentProc(state_t* src_state); //Copia lo stato del processo nel processo corrente
	pcb_t* runningProc(); //Ritorna il puntatore al processo corrente
	void setNULL(); //Setta il processo corrente a NULL
	struct list_head* getHeadRd(); //Restituisce il puntatore al puntatore della head_rd

	int initProcess(int priority, unsigned int pc); //Inizializza i processi e li inserisce nella ready queue

#endif
