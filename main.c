#include "utilis.h"
#include "scheduler.h"
#include "interrupt.h"
#include "tlb.h"
#include "trap.h"
#include "syscall.h"
#include "const.h"

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

	//debug
	mStr("init pcb... OK");
	mStr("init ASL... OK");
	mStr("init scheduler... OK");

	//Aggiunta processo di test nella ready queue
	initProcess(1,(memaddr)test);

	//debug
	mStr("init proc... OK");

	scheduler();
	HALT();

	return(0);
}
