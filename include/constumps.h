#ifndef _CONSTUMPS_H
#define _CONSTUMPS_H

#include "umps/libumps.h"
#include "umps/arch.h"
#include "umps/types.h"
#include "umps/cp0.h"

/* Utili */
#define CAUSE_IP_GET(cause, int_no) ((cause) & (1 << ((int_no) + 8)))

/* Costanti */
 #define FRAMESIZE 				4096

//Rinominazione per unificare i nomi nelle due architetture
#define prog_counter 		pc_epc
#define stack_pointer 		reg_sp

/* Indirizzi old e new areas */
#define INT_NEWAREA 		0x2000008c
#define INT_OLDAREA 		0x20000000
#define TLB_NEWAREA 		0x200001a4
#define TLB_OLDAREA 		0x20000118
#define PGMTRAP_NEWAREA 	0x200002bc
#define PGMTRAP_OLDAREA 	0x20000230
#define SYSCALL_NEWAREA 	0x200003d4
#define SYSCALL_OLDAREA 	0x20000348

//Phase 2

/* Values for CP0 Cause.ExcCode */
#define EXC_INTERRUPT      0
#define EXC_TLBMOD         1
#define EXC_TLBINVLOAD     2
#define EXC_TLBINVSTORE    3
#define EXC_ADDRINVLOAD    4
#define EXC_ADDRINVSTORE   5
#define EXC_BUSINVFETCH    6
#define EXC_BUSINVLDSTORE  7
#define EXC_SYSCALL        8
#define EXC_BREAKPOINT     9
#define EXC_RESERVEDINSTR  10
#define EXC_COPROCUNUSABLE 11
#define EXC_ARITHOVERFLOW  12
#define EXC_BADPTE         13
#define EXC_PTEMISS        14

/* Interrupt lines used by the devices */
#define INT_T_SLICE  1 /* time slice interrupt */
#define INT_TIMER    2 /* timer interrupt */
#define INT_LOWEST   3 /* minimum interrupt number used by real devices */
#define INT_DISK     3
#define INT_TAPE     4
#define INT_UNUSED   5 /* network? */
#define INT_PRINTER  6
#define INT_TERMINAL 7

#define FRAMESIZE 4096

#define DEV_USED_INTS 5 /* Number of ints reserved for devices: 3,4,5,6,7 */
#define DEV_PER_INT   8 /* Maximum number of devices per interrupt line */
#define DEV_REGS_BASE 0x10000050

// Possibili status dei device
#define DEV_NOT_INSTALLED 0
#define DEV_S_READY 1

#define DEV_C_ACK   1 /* command common to all devices */

//Interrupting device bitmap start address
#define PENDING_BITMAP_START 0x1000003c

//Macro che ritorna una word contenente il device che richiede l'interrupt nella linea LINENO
#define INTR_CURRENT_BITMAP(LINENO)	 (unsigned int *)(PENDING_BITMAP_START + (WORD_SIZE * (LINENO - 3)))

//Trova indirizzo base device
#define DEV_ADDR(INT_LINE_NO,DEV_NO) DEV_REGS_BASE + ((INT_LINE_NO - 3) * 0x80 ) + (DEV_NO * 0x10)

//Timing
#define BUS_TODLOW 	0x1000001c
#define TOD_LO          *((unsigned int*)0x1000001C)
#define getTOD_LO() (*((unsigned int*)BUS_TODLOW))


#endif
