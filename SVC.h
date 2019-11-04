/*
 * @file    SVC.h
 * @brief   Contains functionality of kernel service calls
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    1-Nov-2019 (edited)
 */
#pragma once
#include "Process.h"

#ifndef GLOBAL_SVC
#define GLOBAL_SVC

extern int registerProcess(void (*)(void), unsigned int, unsigned int);
extern int addPCB(PCB *, unsigned int);
extern PCB * removePCB(void);
extern void initpendSV(void);

#else

void decrementPriority(void);
int addPCB(PCB *, unsigned int);
PCB * removePCB(void);
void initpendSV(void);
void SVCall(void);
void SVCHandler(StackFrame*);
void setRunning(void);

#endif /* GLOBAL_SVC */
