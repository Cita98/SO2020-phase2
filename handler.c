#include "handler.h"

void int_handler(){

	/* Prendo il puntatore allo stato del processo interrotto, nella old area */
	state_t* old_proc = ((state_t*)INT_OLDAREA);

	#ifdef TARGET_UARM
			//Program counter della old area decrementato di 4 byte
		old_proc->prog_counter -= 4;
	#endif

	/* Copio lo stato del processo interrotto nel processo corrente */
	updateCurrentProc(old_proc);

	unsigned int cause = getCAUSE();

	if(CAUSE_IP_GET(cause,INT_TIMER))
	{
		//Ack dell'interrupt, riavvio il timer
		setIT_TIMER(TIME_SLICE);
		// Richiamo lo scheduler per far partire il prossimo processo
		scheduler();

	}

	if(CAUSE_IP_GET(cause,INT_DISK))
	{
		intDisk();
	}

	if(CAUSE_IP_GET(cause,INT_TAPE))
	{
		intTape();
	}

	if(CAUSE_IP_GET(cause,INT_UNUSED))
	{
		intNet();
	}

	if(CAUSE_IP_GET(cause,INT_PRINTER))
	{
		intPrint();
	}

	if(CAUSE_IP_GET(cause,INT_TERMINAL))
	{
		intTerm();
	}

}

//NOTA: i parametri delle syscall si trovano nei registri a0-a3 su umps e a1-a4 su uarm
//		al termine della syscall il valore di ritorno va messo nel registro v0 per umps a1 per uarm

void syscall_handler()
{

	/* Prendo il puntatore allo stato del processo interrotto, nella old area */
	state_t* old_proc =((state_t*) SYSCALL_OLDAREA);

	#ifdef TARGET_UMPS
		//Incremento program counter del processo di 4 byte
		old_proc->prog_counter += 4;
	#endif

	/* Copio lo stato del processo interrotto nel processo corrente dello scheduler*/
	updateCurrentProc(old_proc);

	unsigned int* param[3];
	unsigned int result;

	pcb_t* cur_proc = runningProc(); //Prendo il puntatore al processo corrente

	int SysNumb = get_SysNumb(cur_proc); //Recupero il numero della Syscall in maniera differente per le due architetture
	get_param(param, cur_proc);


	switch(SysNumb)
	{
			case(GETCPUTIME):
				//Restituisce il tempo trascorso dalla prima esecuzione del processo
				//Quanto tempo passato come utente, kernel (tempi di syscall e int), tempo totale trascorso

			break;

			case(CREATEPROCESS):


			break;

			case(TERMINATEPROCESS):
					/* Termino il processo corrente e tutta la sua progenie */
				terminate_process(cur_proc);
					/* Richiamo lo scheduler per passare al prossimo processo */
				setNULL();
				scheduler();
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

				result = do_io(*param[0],(unsigned int*)*param[1],(int)*param[2]);
				if(result != DEV_S_READY)
				{
					#ifdef TARGET_UMPS
						cur_proc->p_s.reg_vo = result;
					#endif
					#ifdef TARGET_UARM
						cur_proc->p_s.a1 = result;
					#endif
					//Se l'operazione di IO non ha impartito il comando riavvio il processo inserendo come risultato della syscall il registro status del device
					LDST(&(cur_proc->p_s));
				}

			break;

			case(SPECPASSUP):
					//Assegnamento gestore di livello superiore
				spec_passup((int)*param[0], (state_t*)*param[1], (state_t*)*param[2]);
			break;

			case(GETPID):


			break;

			default: /* In tutti gli altri casi errore */
				PANIC();
			break;
	}


}

void tlb_handler(){return;} //Da implementare nelle fasi successive

void pgmtrap_handler(){ return; } //Da implementare nelle fasi successive
