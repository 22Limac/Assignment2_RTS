/*
 * @file    KernelCall.c
 * @brief
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    9-Nov-2019 (edited)
 */
#include <stdio.h>
#include "KernelCall.h"
#include "Process.h"
#include "Messages.h"

void assignR7(volatile unsigned long data)
{
    __asm("     mov     r7,r0");
}

int procKernelCall(int code, void* messageStruct)
{
    volatile KernelArgs argList;
    argList.code = code;
    argList.arg1 = (unsigned long)&messageStruct;
    assignR7((unsigned long) &argList);

    SVC();

    return argList.rtnvalue;

}


int getid()
{
volatile KernelArgs getIdArg; /* Volatile to actually reserve space on stack */
getIdArg . code = GETID;

/* Assign address of getidarg to R7 */
assignR7((unsigned long) &getIdArg);

SVC();

return getIdArg.rtnvalue;
}

void terminate()
{
    volatile KernelArgs terminateArg; /* Volatile to actually reserve space on stack */
    terminateArg.code = TERMINATE;

    /* Assign address of terminateArg to R7 */
    assignR7((unsigned long) &terminateArg);

    SVC();
}

/* @brief   Kernel call used to change a process' priority
 * @param   [in] int newPriority: Desired new priority of running process
 * @return  int: New priority of calling process. If this value is the same
 *          as the process' priority from before this call, then the priority
 *          change has failed.
 */
int nice(int newPriority)
{
    volatile KernelArgs niceArgs; /* Volatile to actually reserve space on stack */

    /* First check whether requested priority is valid */
    if((newPriority < 0) || (newPriority >= PRIORITY_LEVELS))
    {
        return -1;
    }

    niceArgs.code = NICE;
    niceArgs.arg1 = newPriority;

    /* Assign address of niceArgs to R7 */
    assignR7((unsigned long) &niceArgs);

    SVC();
    return niceArgs.rtnvalue;
}


/*
 * @brief   Invokes the kernel to send a message to a desired queue
 * @param   [in] unsigned char sourceQueue:
 *          index of queue from which message was sent
 * @param   [in] unsigned char destQueue:
 *          index of queue designated as the message's destination
 * @param   [in] void * msgptr:
 *          address of message in memory
 * @param   [in] unsigned int size:
 *          size of message to send in bytes
 */
int sendMessage(int destinationMB, int fromMB, void * contents, int size)
{
    SendMessage sendArgs;
    sendArgs.destinationMB = destinationMB;
    sendArgs.fromMB = fromMB;
    sendArgs.contents =contents;
    sendArgs.size =size;

    return procKernelCall( SENDMSG, &sendArgs);
}


/*
 * @brief   Invokes the kernel to receive a message from a desired queue
 * @param   [in] unsigned char sourceQueue:
 *          index of queue from which message was sent
 * @param   [in] unsigned char destQueue:
 *          index of queue designated as the message's destination
 * @param   [in] void * msgptr:
 *          address of message in memory
 * @param   [in] unsigned int size:
 *          size of message to receive in bytes
 */
unsigned int recvMessage(int bindedMB, int * returnMB, void * contents, int maxSize)
{
    ReceiveMessage recvArgs;
    recvArgs.bindedMB = bindedMB;
    recvArgs.returnMB = returnMB;
    recvArgs.contents =contents;
    recvArgs.maxSize = maxSize;

    return procKernelCall( RECEIVEMSG, &recvArgs);
}



