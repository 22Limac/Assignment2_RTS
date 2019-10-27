/*
 * SVC.h
 *
 *  Created on: Oct 20, 2019
 *      Author: LiamMacDonald
 */
#pragma once
#include "Process.h"

#ifndef GLOBAL_SVC
#define GLOBAL_SVC

extern int registerProcess(void (*)(void), unsigned int, unsigned int);
extern int addPCB(PCB *, unsigned int);
extern PCB * removePCB(void);

#else

void decrementPriority(void);
int addPCB(PCB *, unsigned int);
PCB * removePCB(void);
void SVCall(void);
void SVCHandler(StackFrame*);
void setRunning(void);

#endif /* GLOBAL_SVC */
