#include "syscall.h"


void init_syscall(){ //Inizializzazione syscall new area
	
	state_t* sys_na = ((state_t *)SYSCALL_NEWAREA);
	
	sys_na->prog_counter = (memaddr)syscall_handler;
	sys_na->stack_pointer = RAMTOP;
	
	
	#ifdef TARGET_UMPS
	
		//interrup disabilitati, Kernel mode ON, Virtual memory OFF
		sys_na->status &= (~STATUS_KUc & ~STATUS_VMc & ~STATUS_IM_MASK);
		//Interrupt interval timer abilitato
		sys_na->status |= STATUS_IEc;
		sys_na->status |= STATUS_IEp;
	
	#endif
	
	#ifdef TARGET_UARM

		//System mode abilitato (sarebbe il kernel mode di UARM)
		sys_na->cpsr |= STATUS_SYS_MODE;
		//Virtual memory OFF 
		sys_na->CP15_Control = CP15_DISABLE_VM(sys_na->CP15_Control);
		//interrup disabilitati
		sys_na->cpsr = STATUS_DISABLE_INT(sys_na->cpsr);
		//Interval Timer abilitato
		sys_na->cpsr = STATUS_ENABLE_TIMER(sys_na->cpsr);
	
	#endif
	

}   

int get_SysNumb(pcb_t* curr_proc){
	
	int SysNumb; //Numero della System Call da gestire
	
	#ifdef TARGET_UMPS
		SysNumb = curr_proc->p_s.reg_a0;
	#endif
	
	#ifdef TARGET_UARM
		SysNumb = curr_proc->p_s.a1; 
	#endif
	
	return(SysNumb);
}


//SYSCALL 2
void create_process(state_t *state_p, int priority, void** cpid){
	
	pcb_t * new_proc = allocPcb();
	
	if(new_proc == NULL) return(-1); //Errore, non è stato creato un nuovo processo
	else{
			//Setto la priorità e lo stato del nuovo processo
		new_proc->original_priority = priority;
		cp_state(state_p, *(new_proc->p_s));
		
		pcb_t* cur_proc = runningProc();
		
			//Aggiungo il processo come figlio del corrente e lo iserisco nella lista dei processi pronti
		insertChild(cur_proc, new_proc);
		insertProcQ(head_rd, new_proc);
		
			//cpid contiene l'identificatore del processo figlio
		if(cpid!=NULL) *((pcb_t **)cpid) = new_proc;
		
			//Tutto è andato a buon fine, ritorno 0
		return(0);
	}
	
}


//SYSCALL 3 PROBABILMENTE DA CAMBIARE, LEGGI SPECIFICHE NUOVE
void terminate_process(pcb_t* curr_proc) 	// Rimuovo il processo da terminare e tutti i suoi figli dalla ready_queue 
{
	outChildBlocked(curr_proc); // Funzione definita in asl.c, scelta progettuale spiegata nella documentazione 
	// NOTA: Il processo terminato non si troverà sicuramente nella ready_queue poichè era il processo corrente 
	// Libero il processo corrente rimettendolo nella lista dei pcb liberi 
	freePcb(curr_proc);

	return;
}


//SYSCALL 4
void verhogen(int* semaddr){
	
	pcb_t *proc = NULL;
		//incremento il valore del semaforo, operazione di rilascio
	*semaddr += 1;
		//sveglio un processo in attesa sul semaforo
	if(*semaddr <= 0) proc = removeBlocked(semaddr);
	
		//se ho svegliato un processo lo inserisco nella nella ready queue
	if(proc!=NULL) insertProcQ(head_rd, proc);
	
}


//SYSCALL 5
void passeren(int* semaddr){
	
	

	
	
}



	


























