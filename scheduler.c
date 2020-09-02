#include "scheduler.h"
#include "pcb.h"
#include "utilis.h"

extern void termprint(char *str);

//Puntatore al processo corrente, variabile condivisa
pcb_t* current_proc = NULL;

//Lista dei processi pronti inizializzata qui per poterla richiamare in tutti gli altri file
LIST_HEAD(ready_queue);
struct list_head* head_rd = &(ready_queue);

struct list_head* getHeadRd()
{
	return head_rd;
}

//SCHEDULING

void scheduler()
{
	if(current_proc != NULL){
		/* Reinserisco il processo corrente (se esiste) nella lista dei processi pronti*/
		insertProcQ(head_rd, current_proc);
		current_proc = NULL;
	}

	if(current_proc == NULL && emptyProcQ(head_rd)){HALT();} //Se la ready queue è vuota metto il sistema in attesa
	else{//Altrimenti

				/* Prendo il processo con priorità maggiore */
			current_proc = removeProcQ(head_rd);
				/* Riporto la priorità del processo a quella originaria */
			current_proc->priority = current_proc->original_priority;
				/* Aumento la priorità di tutti gli altri processi che restano in coda */
			aging();
				/* Imposto l'interval timer */
			setIT_TIMER(TIME_SLICE);
				/* Time management: se è la prima attivazione del processo inizio a calcolarne il tempo totale di attivazione */
			if(!current_proc->wallclock_time) current_proc->wallclock_time = getTODLO();
				/* Se un processo si trovava nella lista dei processi pronti deve essere in user mode, se viene caricato esegue il proprio codice non quello di un'eccezione */
			current_proc->user_timeNEW = getTODLO();

				/* Carico lo stato del processo all'interno del processore */
			LDST(&(current_proc->p_s));
	}

	return;
}

void aging()
{
	/* Creo l'iteratore della lista ready queue */
	pcb_t* iter;
	/* Scorro la lista e aumento di 1 la priorità di ogni processo */
	list_for_each_entry(iter, head_rd, p_next)
	{ iter->priority = iter->priority+1; }
}

//FUNZIONI PER L'UTILIZZO DELLE VARIABILI CONDIVISE (current proc e ready queue)

pcb_t* runningProc() //Funzione che permette di accere al current proc da altri file
{
	return current_proc;
}

void setNULL() //Utilizzata per indicare che il current_proc è in uno stato inconsistente
{
	current_proc = NULL;
}

int initProcess(int priority, unsigned int pc) //Inizializzazione del processo da inserire nella ready queue
{
	int nRet = FALSE;
	pcb_t* newPcb;
	if( (newPcb = allocPcb()) != NULL)
	{
		nRet = TRUE;
		init_ProcState(newPcb);
		newPcb->p_s.stack_pointer = RAMTOP - FRAMESIZE * priority;
		newPcb->priority = priority;
		newPcb->original_priority = priority;
		newPcb->p_s.prog_counter = pc;
			///Inizializzazione variabili Spec_PassUp
		newPcb->spec_assigned[0] = FALSE; // Syscall/breakpoint
		newPcb->spec_assigned[1] = FALSE; // TLB
		newPcb->spec_assigned[2] = FALSE; // Program trap

		insertProcQ(head_rd,newPcb);
	}
	return nRet;
}

void initScheduler(){ //Inizializzazione della ready queue

		mkEmptyProcQ(head_rd);
}

void updateCurrentProc(state_t* src_state) //Copia lo stato di un processo nel processo corrente
{
	cp_state(src_state, &(current_proc->p_s));
}
