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


	 if(CAUSE_IP_GET(getCAUSE(),INTERVAL_TIMER))
	{
		//Ack dell'interrupt, riavvio il timer
		setIT_TIMER(TIME_SLICE);
		// Richiamo lo scheduler per far partire il prossimo processo
		scheduler();

	} //In tutti gli altri casi, errore
	else PANIC();

}


void syscall_handler()
{

	/* Prendo il puntatore allo stato del processo interrotto, nella old area */
	state_t* old_proc =((state_t*) SYSCALL_OLDAREA);

	#ifdef TARGET_UMPS
		//Incremento program counter del processo
		old_proc->prog_counter += 1;
	#endif

	/* Copio lo stato del processo interrotto nel processo corrente */
	updateCurrentProc(old_proc);

	unsigned int param[3];
	unsigned int result;

	pcb_t* cur_proc = runningProc(); //Prendo il puntatore al processo corrente

	int SysNumb = get_SysNumb(cur_proc); //Recupero il numero della Syscall in maniera differente per le due architetture
	get_param(param, cur_proc);


	switch(SysNumb)
	{
			case(TERMINATEPROCESS):
					/* Termino il processo corrente e tutta la sua progenie */
				terminate_process(cur_proc);
					/* Richiamo lo scheduler per passare al prossimo processo */
				setNULL();
				scheduler();
			break;

			case(WAITIO):

				result = do_io(param[0],(unsigned int*)param[1],(int)param[2], cur_proc);
				if(result != DEV_NOT_INSTALLED)
				{
					setNULL();
					scheduler();
				}

			break;

			default: /* In tutti gli altri casi errore */
				PANIC();
			break;
	}

}

void tlb_handler(){return;} //Da implementare nelle fasi successive

void pgmtrap_handler(){ return; } //Da implementare nelle fasi successive
