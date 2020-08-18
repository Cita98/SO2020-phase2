#include "utilis.h"
#include "scheduler.h"
#include "interrupt.h"
#include "tlb.h"
#include "trap.h"
#include "syscall.h"

//Processi di test
extern void test();

int main(){
	
	//Inizializzazione delle new area
	init_interrupt();
	init_syscall();
	init_tlb();
	init_pgmtrap();
	
	
	//Inizializzazione processi
	initPcbs();
	//Inizializzazione dei semafori
	initASL();
	//Inizializzazione ready queue
	initScheduler();
	
	//Aggiunta processo di test nella ready queue
	initProcess(1,(memaddr)test);
	
	scheduler();
	HALT();
	
	return(0);
}
