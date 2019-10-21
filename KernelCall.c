/*
 * @file    KernelCall.c
 * @brief   Contains functionality to identify command
 *          passed from holding buffer and call its respective
 *          function with arguments
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date
 */
#include <stdio.h>
#include "KernelCall.h"
#include "Process.h"

int getid()
{
volatile struct kcallargs getidarg; /* Volatile to actually reserve space on stack */
getidarg . code = GETID;

/* Assign address if getidarg to R7 */
//assignR7((unsigned long) &getidarg);

SVC();

return getidarg . rtnvalue;

}



