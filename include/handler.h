#ifndef _HANDLER_H
#define _HANDLER_H

#include "interrupt.h"
#include "syscall.h"
#include "const.h"

void int_handler(); // Handler per gli interrupt

void syscall_handler(); // Handler per le systemcall

void tlb_handler();

void pgmtrap_handler();

#endif