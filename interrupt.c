#include "interrupt.h"

void init_interrupt(){ //Inizializzazione new area interrupt
	
	state_t* int_na = ((state_t *)INT_NEWAREA);
	
	int_na->prog_counter = (memaddr)int_handler; 	
	int_na->stack_pointer= RAMTOP;
	
	#ifdef TARGET_UMPS
	
		//interrup disabilitati, Kernel mode ON, Virtual memory OFF
		int_na->status &= (~STATUS_KUc & ~STATUS_VMc & ~STATUS_IM_MASK);
		int_na->status |= STATUS_IEp;
		int_na->status |= STATUS_IEc;
	
	#endif
	
	#ifdef TARGET_UARM
	
		//System mode abilitato (sarebbe il kernel mode di UARM)
		int_na->cpsr |= STATUS_SYS_MODE;
		//Virtual memory OFF 
		int_na->CP15_Control = CP15_DISABLE_VM(int_na->CP15_Control);
		//interrup disabilitati ma timer abilitato
		int_na->cpsr = STATUS_ALL_INT_DISABLE(int_na->cpsr);

	
	#endif
}
