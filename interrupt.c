#include "interrupt.h"



void init_interrupt(){ //Inizializzazione new area interrupt

	state_t* int_na = ((state_t *)INT_NEWAREA);

	int_na->prog_counter = (memaddr)int_handler;
	int_na->stack_pointer= RAMTOP;

	//memset(sem_dev, 0, sizeof(sem_dev));
	for (int i = 0; i < DEV_PER_INT; i++)
	{
		sem_dev.disk[i] = 0;
    sem_dev.tape[i] = 0;
    sem_dev.network[i] = 0;
    sem_dev.printer[i] = 0;
    sem_dev.terminalR[i] = 0;
    sem_dev.terminalT[i] = 0;
	}

	/*INTERRUPT->pc_epc = (unsigned int)interruptHandler;
		INTERRUPT->reg_sp = RAMTOP;
		INTERRUPT->status = INTERRUPT->status & ~STATUS_IEc;    	//Tutti gli interrupt disabilitati
		INTERRUPT->status = INTERRUPT->status & ~STATUS_IEp;		//Disabilito IEp
		INTERRUPT->status = INTERRUPT->status & ~STATUS_IM_MASK; 	//Disabilito maschera interrupt
		INTERRUPT->status = INTERRUPT->status & ~STATUS_KUc;		//Abilito kernel mode KUc=0 OK
		INTERRUPT->status = INTERRUPT->status & ~STATUS_VMc;		//Disabilito virtual memory OK
		INTERRUPT->status = INTERRUPT->status | STATUS_CU0;*/

	#ifdef TARGET_UMPS


	int_na->status = int_na->status & ~STATUS_IEc;    	//Tutti gli interrupt disabilitati
	int_na->status = int_na->status & ~STATUS_IEp;		//Disabilito IEp
	int_na->status = int_na->status & ~STATUS_IM_MASK; 	//Disabilito maschera interrupt
	int_na->status = int_na->status & ~STATUS_KUc;		//Abilito kernel mode KUc=0 OK
	int_na->status = int_na->status & ~STATUS_VMc;		//Disabilito virtual memory OK
	int_na->status = int_na->status | STATUS_CU0;
		//interrup disabilitati, Kernel mode ON, Virtual memory OFF
		/*int_na->status &= (~STATUS_KUc & ~STATUS_VMc & ~STATUS_IM_MASK);
		int_na->status |= STATUS_IEp;
		int_na->status |= STATUS_IEc;*/

	#endif

	#ifdef TARGET_UARM

	/*state_t *INTERRUPT = (state_t*) INT_NEWAREA;
		INTERRUPT->pc = (unsigned int)interruptHandler;
		INTERRUPT->sp = RAM_TOP;
		INTERRUPT->cpsr = STATUS_DISABLE_INT(INTERRUPT->cpsr);
		INTERRUPT->cpsr = STATUS_DISABLE_TIMER(INTERRUPT->cpsr);
		INTERRUPT->cpsr = INTERRUPT->cpsr | STATUS_SYS_MODE;
		INTERRUPT->CP15_Control = CP15_DISABLE_VM(INTERRUPT->CP15_Control);*/


		//interrup disabilitati ma timer abilitato
		//int_na->cpsr = STATUS_ALL_INT_DISABLE(int_na->cpsr); NON E' CAMBIATO NULLA NEMMENO CON LE RIGHE SOTTO
		int_na->cpsr = STATUS_DISABLE_INT(int_na->cpsr);
		int_na->cpsr = STATUS_DISABLE_TIMER(int_na->cpsr);
		//System mode abilitato (sarebbe il kernel mode di UARM)
		int_na->cpsr |= STATUS_SYS_MODE;
		//Virtual memory OFF
		int_na->CP15_Control = CP15_DISABLE_VM(int_na->CP15_Control);


	#endif

}

//Ritorna vero se il device $i nella linea interrupt $intLine sta richiedendo un interrupt
int checkDevLine(int intLine, int dev)
{
	return *INTR_CURRENT_BITMAP(intLine) & (1 << dev);
}

//Setta il risultato di una operazione IO nel primo processo bloccato al semaforo $key
//Ritorna 0 se e' andato a buon fine
int setIOresult(int *key, unsigned int res)
{
	int nRet = 1;
	pcb_t *temp = headBlocked(key);
	if(temp != NULL)
	{
		/*#ifdef TARGET_UARM
				//Program counter della old area decrementato di 4 byte
			temp->p_s.prog_counter -= 1;
		#endif*/
		#ifdef TARGET_UMPS
			temp->p_s.reg_v0 = res;
		#endif
		#ifdef TARGET_UARM
			temp->p_s.a1 = res;
		#endif
		nRet = 0;
	}
	return nRet;
}

void intDisk()
{
	for(int i = 0; i < DEV_PER_INT; i++)
	{
		if(checkDevLine(INT_DISK,i))
		{
			//Il device i sulla linea INT_DISK ha alzato un interrupt
			dtpreg_t *dev = (dtpreg_t*)DEV_ADDR(INT_DISK, i);

			//Se non si riesce a scrivere il risultato della io BOH... vado in panic?
			if(!setIOresult(&sem_dev.disk[i],dev->status))
				verhogen(&sem_dev.disk[i]);

			dev->command = DEV_C_ACK;
		}
	}
}

void intTape()
{
	for(int i = 0; i < DEV_PER_INT; i++)
	{
		if(checkDevLine(INT_TAPE,i))
		{
			//Il device i sulla linea INT_DISK ha alzato un interrupt
			dtpreg_t *dev = (dtpreg_t*)DEV_ADDR(INT_TAPE, i);

			//Se non si riesce a scrivere il risultato della io BOH... vado in panic?
			if(!setIOresult(&sem_dev.tape[i],dev->status))
				verhogen(&sem_dev.tape[i]);

			dev->command = DEV_C_ACK;
		}
	}
}

void intNet()
{
	for(int i = 0; i < DEV_PER_INT; i++)
	{
		if(checkDevLine(INT_UNUSED,i))
		{
			//Il device i sulla linea INT_DISK ha alzato un interrupt
			dtpreg_t *dev = (dtpreg_t*)DEV_ADDR(INT_UNUSED, i);

			//Se non si riesce a scrivere il risultato della io BOH... vado in panic?
			if(!setIOresult(&sem_dev.network[i],dev->status))
				verhogen(&sem_dev.network[i]);

			dev->command = DEV_C_ACK;
		}
	}
}

void intPrint()
{
	for(int i = 0; i < DEV_PER_INT; i++)
	{
		if(checkDevLine(INT_PRINTER,i))
		{
			//Il device i sulla linea INT_DISK ha alzato un interrupt
			dtpreg_t *dev = (dtpreg_t*)DEV_ADDR(INT_PRINTER, i);

			//Se non si riesce a scrivere il risultato della io BOH... vado in panic?
			if(!setIOresult(&sem_dev.printer[i],dev->status))
				verhogen(&sem_dev.printer[i]);

			dev->command = DEV_C_ACK;
		}
	}
}

void intTerm()
{
	for(int i = 0; i < DEV_PER_INT; i++)
	{
		if(checkDevLine(INT_TERMINAL,i))
		{
			//Il device i sulla linea INT_DISK ha alzato un interrupt
			termreg_t *dev = (termreg_t*)DEV_REG_ADDR(INT_TERMINAL, i);

			//mStr("debug prova");


			if((dev->recv_status & STATUSMASK) != DEV_S_READY)
			{

				//Se non si riesce a scrivere il risultato della io BOH... vado in panic?
				if(!setIOresult(&sem_dev.terminalR[i],dev->recv_status))
					verhogen(&sem_dev.terminalR[i]);

				dev->recv_command = DEV_C_ACK;

			}

			if((dev->transm_status & STATUSMASK) != DEV_S_READY)
			{
				//aaadebugFc();
				//Se non si riesce a scrivere il risultato della io BOH... vado in panic?
				if(!setIOresult(&sem_dev.terminalT[i],dev->transm_status))
				 	verhogen(&sem_dev.terminalT[i]);
				//
				dev->transm_command = DEV_C_ACK;

			}

		}
	}
}

void aaadebugFc()
{
	int i = 5;
	i = i +3;
	return;
}
