#include "utilis.h"

extern void termprint(char *str);

//Funzioni ausiliarie

void cp_state(state_t* src_state, state_t* dst_state){
	
	//Copio ogni campo dello stato sorgente nello stato di destinazione
	
	#ifdef TARGET_UMPS
	
	dst_state->entry_hi = src_state->entry_hi;
	dst_state->cause = src_state->cause;
	dst_state->status = src_state->status;
	dst_state->pc_epc = src_state->pc_epc;
	dst_state->hi = src_state->hi;
	dst_state->lo = src_state->lo;
	for(int i=0;i<29;i++){dst_state->gpr[i]=src_state->gpr[i];}
	
	#endif
	
	#ifdef TARGET_UARM
	
	dst_state->a1 = src_state->a1;
	dst_state->a2 = src_state->a2;
	dst_state->a3 = src_state->a3;
	dst_state->a4 = src_state->a4;
	dst_state->v1 = src_state->v1;
	dst_state->v2 = src_state->v2;
	dst_state->v3 = src_state->v3;
	dst_state->v4 = src_state->v4;
	dst_state->v5 = src_state->v5;
	dst_state->v6 = src_state->v6;
	dst_state->sl = src_state->sl;
	dst_state->fp = src_state->fp;
	dst_state->ip = src_state->ip;
	dst_state->sp = src_state->sp;
	dst_state->lr = src_state->lr;
	dst_state->pc = src_state->pc;
	dst_state->cpsr = src_state->cpsr;
	dst_state->CP15_Control = src_state->CP15_Control;
	dst_state->CP15_EntryHi = src_state->CP15_EntryHi;
	dst_state->CP15_Cause = src_state->CP15_Cause;
	dst_state->TOD_Hi = src_state->TOD_Hi;
	dst_state->TOD_Low = src_state->TOD_Low;
	
	#endif
	
}

//Funzioni di inizializzazione

void init_ProcState(pcb_t* new_Pcb){
	
	#ifdef TARGET_UMPS
		new_Pcb->p_s.status |= (STATUS_IEc | STATUS_IEp | STATUS_IM(2)); //Interrupt abilitati
		new_Pcb->p_s.status &= (~STATUS_KUc & ~STATUS_VMc); //Kernel mode ON, Virtual memory OFF
	#endif
	
	//Probabilmente da abilitare l'interval timer
	#ifdef TARGET_UARM
		new_Pcb->p_s.cpsr |= STATUS_SYS_MODE; //Kernel mode ON
		new_Pcb->p_s.cpsr = STATUS_ENABLE_INT(new_Pcb->p_s.cpsr); //Interrupt disabilitati - per questa fase serve solo il timer
		new_Pcb->p_s.CP15_Control = CP15_DISABLE_VM(new_Pcb->p_s.CP15_Control); //Virtual memory OFF
		new_Pcb->p_s.cpsr = STATUS_ENABLE_TIMER(new_Pcb->p_s.cpsr); //Interval Timer abilitato
	#endif
			
}


//Gestione del timer

unsigned int get_TimeScale(int ms){
	
		//La costante time_scale si chiama allo stesso modo nelle due architetture (arch.h)
		return ms*1000**((unsigned int *)BUS_REG_TIME_SCALE);
	
}

void setIT_TIMER(int ms){ //Funzione per settare l'Interval Timer
	
	unsigned int time;
	time = get_TimeScale(ms);
	*(unsigned int*)BUS_REG_TIMER = time; // In questo modo funziona correttamente per entrambe le architetture 

} 	 

