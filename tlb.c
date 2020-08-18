#include "tlb.h"

void init_tlb(){
	
	state_t* tlb_na = ((state_t *)TLB_NEWAREA);
	
	tlb_na->prog_counter = (memaddr)tlb_handler;
	tlb_na->stack_pointer = RAMTOP;
	
	#ifdef TARGET_UMPS
	
		//interrup disabilitati, Kernel mode ON, Virtual memory OFF
		tlb_na->status &= (~STATUS_KUc & ~STATUS_VMc & ~STATUS_IM_MASK);
		tlb_na->status |= STATUS_IEp;
		tlb_na->status |= STATUS_IEc;
	
	#endif
	
	#ifdef TARGET_UARM
	
		//System mode abilitato (sarebbe il kernel mode di UARM)
		tlb_na->cpsr |= STATUS_SYS_MODE;
		//Virtual memory OFF 
		tlb_na->CP15_Control = CP15_DISABLE_VM(tlb_na->CP15_Control);
		//Interval Timer abilitato
		tlb_na->cpsr = STATUS_ENABLE_TIMER(tlb_na->cpsr);
		//interrup disabilitati
		tlb_na->cpsr = STATUS_DISABLE_INT(tlb_na->cpsr);
	
	#endif
	
}