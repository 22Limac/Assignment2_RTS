/*
 * @file    KernelCall.c
 * @brief
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date
 */
#include <stdio.h>
#include "KernelCall.h"
#include "Process.h"

void assignR7(volatile unsigned long data)
{
    __asm("     mov     r7,r0");
}

int getid()
{
volatile struct kcallargs getIdArg; /* Volatile to actually reserve space on stack */
getIdArg . code = GETID;

/* Assign address if getidarg to R7 */
assignR7((unsigned long) &getIdArg);

SVC();

return getIdArg . rtnvalue;
}

void terminate()
{
    volatile struct kcallargs terminateArg; /* Volatile to actually reserve space on stack */
    terminateArg.code = TERMINATE;

    /* Assign address if getidarg to R7 */
    assignR7((unsigned long) &terminateArg);

    SVC();
}



