#include "trap.h"

void init_pgmtrap(){
	
	state_t* pgmtrap_na = ((state_t *)PGMTRAP_NEWAREA);
	
	pgmtrap_na->prog_counter = (memaddr)pgmtrap_handler;
	pgmtrap_na->stack_pointer = RAMTOP;
	
	#ifdef TARGET_UMPS
	
		//interrup disabilitati, Kernel mode ON, Virtual memory OFF
		pgmtrap_na->status &= (~STATUS_KUc & ~STATUS_VMc & ~STATUS_IM_MASK);
		pgmtrap_na->status |= STATUS_IEp;
		pgmtrap_na->status |= STATUS_IEc;
	
	#endif
	
	#ifdef TARGET_UARM
	
		//System mode abilitato (sarebbe il kernel mode di UARM)
		pgmtrap_na->cpsr |= STATUS_SYS_MODE;
		//Virtual memory OFF 
		pgmtrap_na->CP15_Control = CP15_DISABLE_VM(pgmtrap_na->CP15_Control);
		//Interval Timer abilitato
		pgmtrap_na->cpsr = STATUS_ENABLE_TIMER(pgmtrap_na->cpsr);
		//interrup disabilitati
		pgmtrap_na->cpsr = STATUS_DISABLE_INT(pgmtrap_na->cpsr);
	
	#endif
        
    //debug
	mStr("init pgmtrap... OK");
	
}
