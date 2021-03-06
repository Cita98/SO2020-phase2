#include "handler.h"

void int_handler(){


	pcb_t* curr_proc = runningProc();

		//Time management del tempo passato in user mode
	if(curr_proc != NULL){
		if(curr_proc->user_timeNEW > 0){
			curr_proc->user_time += getTOD_LO() - curr_proc->user_timeNEW;
			curr_proc->user_timeNEW = 0;
		}
	}

	// Prendo il puntatore allo stato del processo interrotto, nella old area
	state_t* old_proc = ((state_t*)INT_OLDAREA);

	#ifdef TARGET_UARM
	// 		//Program counter della old area decrementato di 4 byte
	 	old_proc->prog_counter -= 4;
	#endif

	if(curr_proc != NULL){
		// Copio lo stato del processo interrotto nel processo corrente
		updateCurrentProc(old_proc);
	}

	//aaadebugFc();

	unsigned int cause = getCAUSE();

	if(CAUSE_IP_GET(cause,INT_TIMER))
	{
		mStr("Time");
		//Ack dell'interrupt, riavvio il timer
		setIT_TIMER(TIME_SLICE);
		// Richiamo lo scheduler per far partire il prossimo processo
		scheduler();

	}

	if(CAUSE_IP_GET(cause,INT_DISK))
	{
		mStr("Disk");
		intDisk();
	}

	if(CAUSE_IP_GET(cause,INT_TAPE))
	{
		mStr("Tape");
		intTape();
	}

	if(CAUSE_IP_GET(cause,INT_UNUSED))
	{
		mStr("Net");
		intNet();
	}

	if(CAUSE_IP_GET(cause,INT_PRINTER))
	{
		mStr("Print");
		intPrint();
	}

	if(CAUSE_IP_GET(cause,INT_TERMINAL))
	{
		//mStr("Term");
		intTerm();
	}



	//Quando si gestisce un interrupt tutte le linee di interrupt sono disabilitate quindi per forza alla fine si torna in user mode
	if(curr_proc != NULL){
		curr_proc->user_timeNEW = getTOD_LO();
		LDST(old_proc);
	}
	else //Per gestire il ritorno da un interrupt sollevato in stato di wait
		scheduler();

}

//NOTA: i parametri delle syscall si trovano nei registri a0-a3 su umps e a1-a4 su uarm
//		al termine della syscall il valore di ritorno va messo nel registro v0 per umps a1 per uarm

void syscall_handler()
{
	pcb_t* cur_proc = runningProc(); //Prendo il puntatore al processo corrente

			//Time management del tempo passato in user mode
	if(cur_proc->user_timeNEW > 0){
			cur_proc->user_time += getTOD_LO() - cur_proc->user_timeNEW;
			cur_proc->user_timeNEW = 0;
	}

	//time management, inizio a contare il tempo passato in kernel mode
		cur_proc->kernel_timeNEW = getTOD_LO();

	/* Prendo il puntatore allo stato del processo interrotto, nella old area */
	state_t* old_proc =((state_t*) SYSCALL_OLDAREA);

	#ifdef TARGET_UMPS
		//Incremento program counter del processo di 4 byte
		old_proc->prog_counter += 4;
	#endif

	/* Copio lo stato del processo interrotto nel processo corrente dello scheduler*/
	updateCurrentProc(old_proc);

	p_u_int param[3];
	unsigned int result;

	int SysNumb = get_SysNumb(cur_proc); //Recupero il numero della Syscall in maniera differente per le due architetture
	get_param(param, cur_proc);

	switch(SysNumb)
	{
			case(GETCPUTIME):
					//Restituisce il tempo trascorso dalla prima esecuzione del processo
					//Quanto tempo passato come utente, kernel, tempo totale trascorso
				get_cpu_time((unsigned int*)*param[0],(unsigned int*)*param[1],(unsigned int*)*param[2]);
			break;

			case(CREATEPROCESS):

				result = create_process((state_t *)*param[0], (int) *param[1], (void**) *param[2]);

			break;

			case(TERMINATEPROCESS):
					/* Termino il processo corrente e tutta la sua progenie */
				result = terminate_process((void*)*param[0]);
			break;

			case(VERHOGEN):
					//Operazione di rilascio sul semaforo
				verhogen((int*)*param[0]);

			break;

			case(PASSEREN):
					//Operazione di richiesta di un semaforo
				passeren((int*)*param[0]);
			break;

			case(WAITIO):

				result = do_io((unsigned int)*param[0],(unsigned int*)*param[1],(int)*param[2]);
				/*if(result != DEV_S_READY)
				{
					#ifdef TARGET_UMPS
						cur_proc->p_s.reg_v0 = result;
					#endif
					#ifdef TARGET_UARM
						cur_proc->p_s.a1 = result;
					#endif
				}*/

			break;

			case(SPECPASSUP):
					//Assegnamento gestore di livello superiore
				result = spec_passup((int)*param[0], (state_t*)*param[1], (state_t*)*param[2]);

			break;

			case(GETPID):
				get_pid_ppid((void**)*param[0], (void**)*param[1]);
			break;

			default: //sysbp

				//Controllo se ho un gestore di livello superiore
				if(cur_proc->SysBp_Assigned){

					//Copio nell'old area del processo l'old area della sys
					cp_state(old_proc,  cur_proc->SysBp_Old);

					//Aggiornamento del tempo passato in kernel mode
    			if(cur_proc->kernel_timeNEW > 0){
                    cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
                    cur_proc->kernel_timeNEW = 0;
                }
    			//Ricomincio a contare il tempo passato in user mode
    			cur_proc->user_timeNEW = getTOD_LO();

					//Carico la new area del gestore nel processore
					LDST(cur_proc->SysBp_New);

				}
				else	//Non ho un gestore di livello superiore
				{
					//Gestione del tempo
					if(cur_proc->kernel_timeNEW > 0){
                    cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
                    cur_proc->kernel_timeNEW = 0;
					}

					//Il processo va terminato
					terminate_process(0);

				}
			break;
	}

	#ifdef TARGET_UMPS
		cur_proc->p_s.reg_v0 = result;
	#endif
	#ifdef TARGET_UARM
		cur_proc->p_s.a1 = result;
	#endif

	//Aggiornamento del tempo passato in kernel mode
	if(cur_proc->kernel_timeNEW > 0){
					cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
					cur_proc->kernel_timeNEW = 0;
				}
	//Ricomincio a contare il tempo passato in user mode
	cur_proc->user_timeNEW = getTOD_LO();
	//Ricarico lo stato del processo
	LDST(&cur_proc->p_s);

}

void tlb_handler(){

	pcb_t* cur_proc = runningProc(); //Prendo il puntatore al processo corrente
	state_t *old_proc = (state_t *) TLB_OLDAREA;

	//Time management del tempo passato in user mode
		if(cur_proc->user_timeNEW > 0){
				cur_proc->user_time += getTOD_LO() - cur_proc->user_timeNEW;
				cur_proc->user_timeNEW = 0;
		}
		//time management, inizio a contare il tempo passato in kernel mode
		cur_proc->kernel_timeNEW = getTOD_LO();


	//Controllo se ho un gestore di livello superiore
	if(cur_proc->Tlb_Assigned){

		//Salvo lo stato della old area nel processo attuale
		updateCurrentProc(old_proc);


		//Copio nell'old area del processo l'old area della tlb
		cp_state(old_proc,  (cur_proc->Tlb_Old));

		//Aggiornamento del tempo passato in kernel mode
		if(cur_proc->kernel_timeNEW > 0){
							cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
							cur_proc->kernel_timeNEW = 0;
					}
		//Ricomincio a contare il tempo passato in user mode
		cur_proc->user_timeNEW = getTOD_LO();

		//Cario la new area nel processore
		LDST(cur_proc->Tlb_New);

	}

	//Non ho un gestore al livello superiore
	else{

		//Gestione del tempo
		if(cur_proc->kernel_timeNEW > 0){
							cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
							cur_proc->kernel_timeNEW = 0;
		}
		//Il processo va terminato
		terminate_process(0);

	}

	return;
}

void pgmtrap_handler()
{
	pcb_t* cur_proc = runningProc(); //Prendo il puntatore al processo corrente
	state_t *old_proc = (state_t *) PGMTRAP_OLDAREA;

	//Time management del tempo passato in user mode
		if(cur_proc->user_timeNEW > 0){
				cur_proc->user_time += getTOD_LO() - cur_proc->user_timeNEW;
				cur_proc->user_timeNEW = 0;
		}
		//time management, inizio a contare il tempo passato in kernel mode
		cur_proc->kernel_timeNEW = getTOD_LO();


	//Controllo se ho un gestore di livello superiore
	if(cur_proc->PrgTrap_Assigned){

		//Salvo lo stato della old area nel processo attuale
		updateCurrentProc(old_proc);


		//Copio nell'old area del processo l'old area della tlb
		cp_state(old_proc,  (cur_proc->PrgTrap_Old));

		//Aggiornamento del tempo passato in kernel mode
		if(cur_proc->kernel_timeNEW > 0){
							cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
							cur_proc->kernel_timeNEW = 0;
					}
		//Ricomincio a contare il tempo passato in user mode
		cur_proc->user_timeNEW = getTOD_LO();

		//Cario la new area nel processore
		LDST(cur_proc->PrgTrap_New);

	}

	//Non ho un gestore al livello superiore
	else{

		//Gestione del tempo
		if(cur_proc->kernel_timeNEW > 0){
							cur_proc->kernel_time += getTOD_LO() - cur_proc->kernel_timeNEW;
							cur_proc->kernel_timeNEW = 0;
		}
		//Il processo va terminato
		terminate_process(0);

	}

	return;
}
