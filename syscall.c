#include "syscall.h"
#include "uARMtypes.h"
#include "const.h"
#include "types_bikaya.h"


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

void get_param(unsigned int* param, pcb_t* curr_proc){
	#ifdef TARGET_UMPS
		param[0] = curr_proc->p_s.reg_a1;
		param[1] = curr_proc->p_s.reg_a2;
		param[2] = curr_proc->p_s.reg_a3;
	#endif
	#ifdef TARGET_UARM
		param[0] = curr_proc->p_s.a2;
		param[1] = curr_proc->p_s.a3;
		param[2] = curr_proc->p_s.a4;
	#endif
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



	


























=======
//SYSCALL 5
void passeren(int *semaddr)
{

}

//SYSCALL 6
int do_io(unsigned int command, unsigned int* devRegister, int subdevice, pcb_t* proc)
{
	devreg_t* reg = (devreg_t*)devRegister;
	int type;	//Tipo di device
	int line; //Linea del device
	unsigned int* reg_status;
	unsigned int* reg_command;

	fintTYPEandLINE(&type, &line, devRegister);

	if(type == 7) //Terminale
	{
		if(subdevice)
		{
			reg_status = &(reg->term.recv_status);
			reg_command = &(reg->term.recv_command);
		}else
		{
			reg_status = &(reg->term.transm_status);
			reg_command = &(reg->term.transm_command);
		}
	}else
	{
		reg_status = &(reg->dtp.status);
		reg_command = &(reg->dtp.command);
	}

	switch (*reg_status) {
		case DEV_NOT_INSTALLED:
			return(*reg_status);
		break;
		case DEV_S_READY:
			*reg_command = command;
		break;
	}

	blockProcAtDev(type,line,subdevice,proc);

	return(*reg_status);
}

//DEV_REGS_BASE indirizzo base dei registri per i device esterni
//DEV_REGS_SIZE = sizeof devreg_t

//Trova la linea del device (pline) e il tipo di device ptype (interrupt) dato un registro
void fintTYPEandLINE(int* ptype, int* pline, unsigned int* preg )
{
	int line = 0;
	int type = 0;

	//Controllo il tipo (tenendo conto che ogni tipo puo' avere in totale 8 device)
	for(; (!((void*)preg == (void*)DEV_REGS_BASE + (type * DEV_PER_INT * DEV_REGS_SIZE) + (line * DEV_REGS_SIZE)) && type < DEV_USED_INTS) ;type++)
	{
		for(; (!((void*)preg == (void*)DEV_REGS_BASE + (type * DEV_PER_INT * DEV_REGS_SIZE) + (line * DEV_REGS_SIZE)) && (line < DEV_PER_INT)) ; line++){}
		if(line == DEV_PER_INT)
			line = 0;
	}

	*ptype = type + 3;
	*pline = line;
}

/*				!!!!!!!!!!		ATTENZIONME INCOMPLETA E/O ERRATA		!!!!!!!!!!					*/
void blockProcAtDev(int type, int line, int subdevice, pcb_t* proc){

		//insertProcQ(&(Semd_key->s_procQ), proc); /*Aggiungo p alla coda dei processi bloccati associati a Semd_key */

		switch(type){

			case 3: //DISK
			insertProcQ(&(devSem.disk[line].s_procQ), proc);
			break;

			case 4: //TAPE
			insertProcQ(&(devSem.tape[line].s_procQ), proc);
			break;

			case 5: //UNUSED OR NETWORK
			insertProcQ(&(devSem.network[line].s_procQ), proc);
			break;

			case 6: //PRINTER
			insertProcQ(&(devSem.printer[line].s_procQ), proc);
			break;

			case 7: //TERMINAL
				if(subdevice){ //se è true il terminale è in ricezione
					insertProcQ(&(devSem.terminalR[line].s_procQ), proc);
				} //altrimenti in trasmissione
				else{insertProcQ(&(devSem.terminalT[line].s_procQ), proc);}
			break;
		}

	}
>>>>>>> ccade35ca2688723f33035ebbd1d6c9b637479c2
