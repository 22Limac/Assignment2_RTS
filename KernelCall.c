/*
 * @file    KernelCall.c
 * @brief
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    4-Nov-2019 (edited)
 */
#include <stdio.h>
#include "KernelCall.h"
#include "Process.h"
#include "Messages.h"

void assignR7(volatile unsigned long data)
{
    __asm("     mov     r7,r0");
}

int getid()
{
volatile struct kCallArgs getIdArg; /* Volatile to actually reserve space on stack */
getIdArg . code = GETID;

/* Assign address of getidarg to R7 */
assignR7((unsigned long) &getIdArg);

SVC();

return getIdArg . rtnvalue;
}

void terminate()
{
    volatile struct kCallArgs terminateArg; /* Volatile to actually reserve space on stack */
    terminateArg.code = TERMINATE;

    /* Assign address of terminateArg to R7 */
    assignR7((unsigned long) &terminateArg);

    SVC();
}

int nice(int newPriority)
{
    volatile struct kCallArgs niceArgs; /* Volatile to actually reserve space on stack */

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
unsigned int sendMessage(unsigned char sourceQueue, unsigned char destQueue, void * msgptr, unsigned int size)
{
    volatile struct kCallArgs sendArgs; /* Volatile to actually reserve space on stack */
    volatile struct Message msg_tosend;

    /* Fill message structure */
    msg_tosend.source = sourceQueue;
    msg_tosend.dest = destQueue;
    msg_tosend.msgptr = msgptr;
    msg_tosend.size = size;

    /* Fill out kernel call arguments */
    sendArgs.code = SENDMSG;
    sendArgs.arg1 = (unsigned long)(&msg_tosend);

    /* Assign address of sendArgs to R7 */
    assignR7((unsigned long) &sendArgs);

    /* Trap to the kernel */
    SVC();

    /* Return size of message sent to caller process */
    return sendArgs.rtnvalue;
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
unsigned int recvMessage(unsigned char sourceQueue, unsigned char destQueue, void * msgptr, unsigned int size)
{
    volatile struct kCallArgs recvArgs; /* Volatile to actually reserve space on stack */
    volatile struct Message msg_torecv;

    /* Fill message structure */
    msg_torecv.source = sourceQueue;
    msg_torecv.dest = destQueue;
    msg_torecv.msgptr = msgptr;
    msg_torecv.size = size;

    /* Fill out kernel call arguments */
    recvArgs.code = RECEIVEMSG;
    recvArgs.arg1 = (unsigned long)(&msg_torecv);

    /* Assign address of sendArgs to R7 */
    assignR7((unsigned long) &recvArgs);

    /* Trap to the kernel */
    SVC();

    /* Return size of message received to caller process */
    return recvArgs.rtnvalue;
}



