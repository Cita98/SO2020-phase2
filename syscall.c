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


//SYSCALL 3
void terminate_process(pcb_t* curr_proc) 	// Rimuovo il processo da terminare e tutti i suoi figli dalla ready_queue 
{
	outChildBlocked(curr_proc); // Funzione definita in asl.c, scelta progettuale spiegata nella documentazione 
	// NOTA: Il processo terminato non si troverà sicuramente nella ready_queue poichè era il processo corrente 
	// Libero il processo corrente rimettendolo nella lista dei pcb liberi 
	freePcb(curr_proc);

	return;
}
	
