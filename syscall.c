#include "syscall.h"
// #ifndef TARGET_UARM
// #include "uARMtypes.h"
// #endif
#include "const.h"
#include "types_bikaya.h"
#include "interrupt.h"

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

	//debug
	mStr("init syscall... OK");
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

void get_param(p_u_int *param, pcb_t* curr_proc){
	#ifdef TARGET_UMPS
		param[0] = &(curr_proc->p_s.reg_a1);
		param[1] = &(curr_proc->p_s.reg_a2);
		param[2] = &(curr_proc->p_s.reg_a3);
	#endif
	#ifdef TARGET_UARM
		param[0] = &(curr_proc->p_s.a2);
		param[1] = &(curr_proc->p_s.a3);
		param[2] = &(curr_proc->p_s.a4);
	#endif
}

//SYSCALL 1
void get_cpu_time(unsigned int* user, unsigned int* kernel, unsigned int* wallclock){

	pcb_t* cur_proc = runningProc();

		//Tempo passato dal processo in user mode
	if(user != NULL) *user = cur_proc->user_time;
		//Aggiorno il tempo passato in kernel mode prima di restituirlo in quanto anche questa syscall viene eseguita (ovviamente) in kernel mode
		/* (alla fine della syscall, nell'handler, il kernel time sarà aggiornato di nuovo, perciò basta leggere il TODLOW prima di restituire il valore del kernel time)*/
	cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
		//Tempo passato dal processo in kernel mode
	if(kernel != NULL)
		*kernel = cur_proc->kernel_time;
		//Tempo passato dalla prima attivazione del processo
	if(wallclock != NULL)
		*wallclock = getTOD_LO() - cur_proc->wallclock_time;

}

//SYSCALL 2
int create_process(state_t *state_p, int priority, void** cpid){

	pcb_t * new_proc = allocPcb();

	if(new_proc == NULL) return(-1); //Errore, non è stato creato un nuovo processo
	else{
			//Setto la priorità e lo stato del nuovo processo
		new_proc->original_priority = priority;
		cp_state(state_p, &(new_proc->p_s));

		pcb_t* cur_proc = runningProc();

		struct list_head* head_rd = getHeadRd();

			//Aggiungo il processo come figlio del corrente e lo iserisco nella lista dei processi pronti
		insertChild(cur_proc, new_proc);
		insertProcQ(head_rd, new_proc);

			//cpid contiene l'identificatore del processo figlio
		if(cpid!=NULL) *cpid = (void*)new_proc;
		//*((pcb_t **)cpid) = new_proc;

		//(pcb_t *)*cpid = new_proc;

			//Tutto è andato a buon fine, ritorno 0
		return(0);
	}

}


//SYSCALL 3 PROBABILMENTE DA CAMBIARE, LEGGI SPECIFICHE NUOVE

int terminate_process(void* pid) 	// Rimuovo il processo da terminare e tutti i suoi figli dalla ready_queue
{
	/*quando un processo viene terminato:
			-il processo da eliminare va tolto dalla ready_queue_h
			-la lista dei processi liberi (pcbFree) riceve un nuovo pcb_t allocabile(si usa freePcb)
			-va tolto dalle code dei semafori in cui il processo potrebbe essere bloccato(se non è il curr_proc)
		Inoltre se il processo terminato è il curr_proc, allora va tolto dal processore il processo corrente
		e richiamato lo scheduler*/
	pcb_t* p_daTerminare;
	pcb_t* cur_proc = runningProc();

	if(pid == NULL || pid == 0 || cur_proc == pid)	//Il processo da terminare è il processo corrente
		p_daTerminare = cur_proc;
	else{

		p_daTerminare = (pcb_t *)pid;
			//Controllo che appartenga alla progenie del processo corrente, se non è così termino con errore, vale anche se non ha un padre
		if(!isChildOf(cur_proc, p_daTerminare))
			return(-1);
	}

	//Termino i processi figli
	 //Caso base: p non ha figli, quindi controllo che ne abbia e in caso ricorro su di essi
    if( !emptyChild(p_daTerminare) ){
        	//Prendo il primo figlio
        pcb_t* child = returnChild(p_daTerminare);

        while(child != NULL){

			//Termino i processi figli
            terminate_process(child);
			//Rimuovo il primo figlio e scorro la lista dei figli prendendo il prossimo
			//removeChild(p_daTerminare);
			child = returnChild(p_daTerminare);
        }
    }
	//else //Se il pcb non ha figli lo rimuovo dalla lista del padre
  outChild(p_daTerminare);

	//Gestisco la terminazione del processo corrente, non può essere bloccato ad un semaforo nè presente nella ready queue
	if(p_daTerminare == cur_proc){

		freePcb(p_daTerminare);
		setNULL(); //Setto il processo corrente a NULL
		scheduler(); //Richiamo lo scheduler per passare al prossimo processo
	}
	else{
			//Controllo se il processo da terminare sia bloccato ad un semaforo
		if(p_daTerminare->p_semkey!=NULL){
				//Rimuovo il processo dal semaforo
			outBlocked(p_daTerminare);
				//Rilascio il semaforo
			*(p_daTerminare->p_semkey) +=1;
		}
		else{
				//Recupero la lista dei processi pronti
			struct list_head* head_rd = getHeadRd();
				//Rimuovo il processo dalla lista dei processi pronti
			outProcQ(head_rd, p_daTerminare);
		}
	}

	    //Libero il processo
    freePcb(p_daTerminare);
		//Tutto è andato a buon fine
    return 0;

	/* Caso base: p non ha figli, quindi controllo che ne abbia e in caso ricorro su di essi
	if (!list_empty(&(p_daTerminare->p_child))){
		//Inizializzo l'iteratore
		struct list_head* childIt = &(p_daTerminare->p_child);
		// scorro su ogni figlio
		list_for_each(childIt, &(p_daTerminare->p_child))
		{
			pcb_t** term_Pcb = &container_of(childIt, pcb_t, p_sib);
			// termino i processi figli
			terminate_process(term_Pcb);
		}
	} */
}


//SYSCALL 4
void verhogen(int* semaddr){

	pcb_t *proc = NULL;
		//incremento il valore del semaforo, operazione di rilascio
	*semaddr += 1;
		//sveglio un processo in attesa sul semaforo
	if(*semaddr <= 0) proc = removeBlocked(semaddr);

		//se ho svegliato un processo lo inserisco nella nella ready queue
	if(proc!=NULL){
		struct list_head* head_rd = getHeadRd();
		insertProcQ(head_rd, proc);
	}

}

//Valutare una vPasseren che prenda in ingresso anche il processo da bloccare, potrebbe servire nella Do_Io
//SYSCALL 5
void passeren(int* semaddr){

		//Decremento il valore del semaforo, richiesta della risorsa
	*semaddr -= 1;
		//Blocco il processo al semaforo in caso ce ne fosse bisogno
	if(*semaddr < 0){
		pcb_t* cur_proc = runningProc();
			//Blocco il processo al semaforo
		insertBlocked(semaddr, cur_proc);

//TEMPO DELLA GET CPU TIME DA GESTIRE

		cur_proc = NULL;
		setNULL();
			//Richiamo lo scheduler per passare alla gestione di un altro processo

		scheduler();
	}
}


int do_io(unsigned int command, unsigned int* devRegister, int subdevice)
{
	devreg_t* reg = (devreg_t*)devRegister;
	int type;	//Tipo di device
	int line; //Linea del device
	unsigned int* reg_status;
	unsigned int* reg_command;
	int result;

	//debug
	mStr("Entering do_io");

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

	//debug
	termreg_t* tmpReg = (termreg_t*)devRegister;
	reg_status = &(tmpReg->transm_status);

	switch (*reg_status) {
		case DEV_NOT_INSTALLED:
	 		return(*reg_status);
	 	break;
	 	case DEV_S_READY:
	 		*reg_command = command;
	 	break;
	 }

	if (*reg_status == DEV_S_READY)
	{
		//debug
		mStr("writing command");
		//Eseguo il comando
		*reg_command = command;
		//Blocco il processo
		blockProcAtDev(type,line,subdevice);
		//Restituisco lo status register
		result = DEV_S_READY;
	}else
	{
		//debug
		mStr("error");
		result = *(int*)reg_status;
	}

	*reg_command = command;
	//Blocco il processo
	blockProcAtDev(type,line,subdevice);
	result = DEV_S_READY;

	return(result);
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

void blockProcAtDev(int type, int line, int subdevice){

		switch(type){

			case 3: //DISK
			passeren(&(sem_dev.disk[line]));
			break;

			case 4: //TAPE
			passeren(&(sem_dev.tape[line]));
			break;

			case 5: //UNUSED OR NETWORK
			passeren(&(sem_dev.network[line]));
			break;

			case 6: //PRINTER
			passeren(&(sem_dev.printer[line]));
			break;

			case 7: //TERMINAL
				if(subdevice){ //se è true il terminale è in ricezione
					passeren(&(sem_dev.terminalR[line]));
				}
				else //altrimenti in trasmissione
				{
					passeren(&(sem_dev.terminalT[line]));
				}
			break;
		}

	}

//SYSCALL 7
int spec_passup(int type, state_t* old, state_t* new){

	pcb_t* cur_proc = runningProc();

		//Se la syscall è già stata richiamata, c'è già un assegnamento per quel tipo
	if(cur_proc->spec_assigned[type]){
			//Terminazione con "errore"
		return(-1);
	}
	else{
			//Assegnazione dell'handler di livello superiore per quel tipo
		cur_proc->spec_oarea[type] = old;
		cur_proc->spec_narea[type] = new;
			//Indico che è già stato effettuato un assegnamento per quel tipo
		cur_proc->spec_assigned[type] = TRUE;
			//Terminazione con successo
		return(0);
	}
}
