#ifndef _CONST_H
#define _CONST_H

/****************************************************************************
 *
 * This header file contains the global constant & macro definitions.
 *
 ****************************************************************************/

#ifdef TARGET_UMPS
	#include "constumps.h"
#endif

#ifdef TARGET_UARM
	#include "constuarm.h"
#endif

#include "const_bikaya.h"

#define RAMBASE    *((unsigned int *)BUS_REG_RAM_BASE)
#define RAMSIZE    *((unsigned int *)BUS_REG_RAM_SIZE)
#define RAMTOP     (RAMBASE + RAMSIZE)

/* Tempo */
#define TIME_SLICE 4

/* Maxi number of overall (eg, system, daemons, user) concurrent processes */
#define MAXPROC 20

#define UPROCMAX 3  /* number of usermode processes (not including master proc
		       and system daemons */

#define	HIDDEN static
#define	TRUE 	1
#define	FALSE	0
#define ON 		1
#define OFF 	0
#define EOS 	'\0'

//#define DEV_PER_INT 8 /* Maximum number of devices per interrupt line */

#define CR 0x0a   /* carriage return as returned by the terminal */

/* nucleus (phase2)-handled SYSCALL values */
#define GETCPUTIME       1
#define CREATEPROCESS    2
#define TERMINATEPROCESS 3
#define VERHOGEN         4
#define PASSEREN         5
#define WAITIO           6
#define SPECPASSUP       7
#define GETPID           8

#define DEFAULT_PRIORITY 1
#define TRUE             1
#define FALSE            0

// COSTATI PER LA GESTIONE DEI DEVICE ESTERNI
#define DEV_REGS_SIZE 0x10

typedef unsigned int * p_u_int;

#endif
