/*
 * @file    SVC.c
 * @brief   Contains Service Call (SVC) trap handling functionality.
 *          Declares waitingToRun queues and the running PCB pointer;
 *          contains initialization and manipulation for both.
 *          **not sure if registerProcess belongs here or Process.h**
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    7-Nov-2019 (edited)
 */
#include <stdio.h>
#include <stdlib.h>
#define GLOBAL_SVC
#include "SVC.h"
#include "Process.h"
#include "KernelCall.h"
#include "Messages.h"

#define HIGH_PRIORITY 4
#define LOW_PRIORITY 0
#define RUNNING waitingToRun[currentPriority]
static int currentPriority = 0;

#define MAX_STACK_SIZE (512U)
#define STARTING_PSR (0x01000000U)

/* Macro used to set the priority of the pendSV interrupt */
#define SETPENDSVPRIORITY ((*(volatile unsigned long *)0xE000ED20) |= 0x00E00000UL)

extern void terminate(void);

static PCB * waitingToRun[PRIORITY_LEVELS];

/*
 * @brief   Allocates a new process stack frame and PCB
 *          for the process being registered.
 *          sets PCB sp and pid.
 *          calls addPCB to add PCB to waitingToRun with
 *          respective priority
 * @param   [in] void (*code)(void): pointer to the start of the process code
 *          [in] unsigned int pid: Process ID of process being registered
 *          [in] unsigned char priority: Process' initial priority
 * @return  int: if sucessful, will return 0. Otherwise, return 1, in this case
 *               the desired process will not be registered and the program will
 *               continue to run.
 *
 */
int registerProcess(void (*code)(void), unsigned int pid, unsigned char priority)
{
   /* TODO: First check that process ID has not already been taken */
   unsigned long *processStack = (unsigned long *)malloc(MAX_STACK_SIZE*sizeof(unsigned long));
   StackFrame *processSP = (StackFrame*) processStack;
   processSP -> psr = STARTING_PSR;
   processSP -> pc = (unsigned long)code;
   processSP -> lr = (unsigned long)terminate;

   PCB * newProcess = (PCB*)malloc(sizeof(PCB));
   newProcess -> sp = (unsigned long) processSP;
   newProcess -> pid = pid;

   addPCB(newProcess, priority);
   return 1;
}

/*
 * @brief   Adds a PCB to the end of a waitingToRun queue.
 *          If its the first process in the queue its next
 *          and prev pointers are set to itself.
 *          Otherwise, its added to the end, and pointers are
 *          reassigned accordingly
 * @param   [in] PCB *newPCB: PCB being added to the queue
 * @param   [in] unsigned int newPriority: Priority of queue to which
 *          newPCB will be added
 * */
int addPCB(PCB *newPCB, unsigned int newPriority)
{
    /* Must check whether desired queue is empty */
    if(waitingToRun[newPriority] != NULL)
    {
        /* Must add process to tail of priority queue */
        newPCB->next = waitingToRun[newPriority];
        waitingToRun[newPriority] -> prev -> next = newPCB;
        waitingToRun[newPriority] -> prev = newPCB;
    }
    else
    {   /* Desired queue is empty so the process becomes its only entry */
        waitingToRun[newPriority] = newPCB;
        waitingToRun[newPriority]->next = newPCB;
        waitingToRun[newPriority]->prev = newPCB;
    }

    /* Set new priority of process and adjust current operating priority */
    newPCB->priority = newPriority;
    currentPriority = (currentPriority<newPriority)? newPriority: currentPriority;
    return currentPriority;
}

/*
 * @brief   Function used to remove a process from a waiting to run queue
 * @return  PCB *: Pointer to PCB of process removed from queue
 */
PCB * removePCB()
{
    PCB * toRemove = RUNNING;

    /* Check whether process is the queue's only entry */
    if (RUNNING == RUNNING -> next )
    {
        /* This waiting to run queue is now empty so
         * must move to the next highest priority.
         */
        RUNNING = NULL;
        decrementPriority();
    }
    else
    {
        /* There are other entries in this queue so make adjacent PCBs point
         * to each other and advance running process pointer.
         */
        RUNNING -> next -> prev = RUNNING -> prev;
        RUNNING -> prev -> next = RUNNING ->next;
        RUNNING = RUNNING -> next;
    }

    return toRemove;
}

/*
 * @brief   Decrements operating priority until a non-empty queue is found
 */
void decrementPriority(void)
{
    /* Decrement operating priority if current waiting to run queue is empty */
    while((RUNNING == NULL) && (currentPriority >= 0))
    {
        currentPriority--;
    }
}

/*
 * @brief   Configures pendSV interrupt by setting it to the lowest
 *          possible priority allowing other kernel calls to trigger
 *          the pendSV routine upon finishing their business.
 */
void initpendSV(void)
{
    /* Set pendSV to lowest possible priority */
    SETPENDSVPRIORITY;

    return;
}


/*
 * @brief   pendSV ISR that carries out context switches
 */
void pendSV(void)
{
    save_registers();
    RUNNING -> sp = get_PSP();
    RUNNING = RUNNING -> next;
    set_PSP(RUNNING -> sp);
    restore_registers();
}

/*
 * @brief   Entry point of SVC routine
 */
void SVCall(void)
{
/* Supervisor call (trap) entry point
 * Using MSP - trapping process either MSP or PSP (specified in LR)
 * Source is specified in LR: F1 (MSP) or FD (PSP)
 * Save r4-r11 on trapping process stack (MSP or PSP)
 * Restore r4-r11 from trapping process stack to CPU
 * SVCHandler is called with r0 equal to MSP or PSP to access any arguments
 */

/* Save LR for return via MSP or PSP */
__asm("     PUSH    {LR}");

/* Trapping source: MSP or PSP? */
__asm("     TST     LR,#4");    /* Bit #3 (0100b) indicates MSP (0) or PSP (1) */
__asm("     BNE     RtnViaPSP");

/* Trapping source is MSP - save r4-r11 on stack (default, so just push) */
__asm("     PUSH    {r4-r11}");
__asm("     MRS r0,msp");
__asm("     BL  SVCHandler");   /* r0 is MSP */
__asm("     POP {r4-r11}");
__asm("     POP     {PC}");

/* Trapping source is PSP - save r4-r11 on psp stack (MSP is active stack) */
__asm("RtnViaPSP:");
__asm("     mrs     r0,psp");
__asm("     stmdb   r0!,{r4-r11}"); /* Store multiple, decrement before */
__asm("     msr psp,r0");
__asm("     BL  SVCHandler");   /* r0 Is PSP */

/* Restore r4..r11 from trapping process stack  */
__asm("     mrs     r0,psp");
__asm("     ldmia   r0!,{r4-r11}"); /* Load multiple, increment after */
__asm("     msr psp,r0");
__asm("     POP     {PC}");

}


/*
 * @brief   Kernel side of message sending operation
 * @param   [in] struct Message * msginfo:
 *          pointer to structure containing information required
 *          for message sending
 * @return  unsigned int: size of message received
 */
unsigned int KernelSendMessage(struct Message * msginfo)
{
    /* Check whether destination queue's owner is currently blocked */
    if(Mailbox[msginfo->dest].owner->message != NULL)
    {
        /* Recipient process is blocked so give information directly to process and unblock it */
        //TODO: Limit number of copied bytes
        memcpy(Mailbox[msginfo->dest].owner->message, msginfo->msgptr, msginfo->size);

        /* Unblock recipient process */
        addPCB(Mailbox[msginfo->dest].owner, Mailbox[msginfo->dest].owner->priority);
    }
    else
    {
        /* Recipient process is not currently looking for message so add it to destination
         * queue and move on.
         */
        enqueueMessage(msginfo);
    }
//TODO: Must adjust return value in case of failure
    return msginfo->size;
}


/*
 * @brief   Kernel side of message receiving operation
 * @param   [in] struct Message * msginfo:
 *          pointer to structure containing information required
 *          for message receipt
 * @return  unsigned int: size of message received
 */
unsigned int KernelRecvMessage(struct Message * msginfo)
{
//TODO: Must check that queue in question is owned by currently
//      running process
    struct Message newMessage = dequeueMessage(msginfo->dest);

    /* Check dequeue was successful */
    if(newMessage.msgptr == NULL)
    {
        /* No message was found queued so block running process */
        removePCB();
    }
    else
    {
        /* Copy message over to queue's owner */
        //TODO: Limit number of copied bytes
        memcpy(RUNNING->message->msgptr, newMessage.msgptr, newMessage.size);
    }

//TODO: Adjust return value for failures. right now this returns 0 upon failure
    return newMessage.size;
}

/*
 * @brief   Handler of service calls.
 *          Carries out the requested kernel operation (e.g. getID, nice, etc.)
 */
void SVCHandler(StackFrame *argptr)
{
/*
 * Supervisor call handler
 * Handle startup of initial process
 * Handle all other SVCs such as getid, terminate, etc.
 * Assumes first call is from startup code
 * Argptr points to (i.e., has the value of) either:
   - the top of the MSP stack (startup initial process)
   - the top of the PSP stack (all subsequent calls)
 * Argptr points to the full stack consisting of both hardware and software
   register pushes (i.e., R0..xPSR and R4..R10); this is defined in type
   stack_frame
 * Argptr is actually R0 -- setup in SVCall(), above.
 * Since this has been called as a trap (Cortex exception), the code is in
   Handler mode and uses the MSP
 */
static int firstSVCcall = TRUE;
struct kCallArgs *kcaptr;
PCB * callerPCB;

//TODO: Must disable any and all interrupts here; primarily SYSTICK interrupts

if (firstSVCcall)
{
/*
 * Force a return using PSP
 * This will be the first process to run, so the eight "soft pulled" registers
   (R4..R11) must be ignored otherwise PSP will be pointing to the wrong
   location; the PSP should be pointing to the registers R0..xPSR, which will
   be "hard pulled"by the BX LR instruction.
 * To do this, it is necessary to ensure that the PSP points to (i.e., has) the
   address of R0; at this moment, it points to R4.
 * Since there are eight registers (R4..R11) to skip, the value of the sp
   should be increased by 8 * sizeof(unsigned int).
 * sp is increased because the stack runs from low to high memory.
*/
    set_PSP(RUNNING-> sp + 8 * sizeof(unsigned int));

    firstSVCcall = FALSE;

    /*
     - Change the current LR to indicate return to Thread mode using the PSP
     - Assembler required to change LR to FFFF.FFFD (Thread/PSP)
     - BX LR loads PC from PSP stack (also, R0 through xPSR) - "hard pull"
    */
    __asm(" movw    LR,#0xFFFD");  /* Lower 16 [and clear top 16] */
    __asm(" movt    LR,#0xFFFF");  /* Upper 16 only */
    __asm(" bx  LR");          /* Force return to PSP */
}
else /* Subsequent SVCs */
{
/*
 * kcaptr points to the arguments associated with this kernel call
 * argptr is the value of the PSP (passed in R0 and pointing to the TOS)
 * the TOS is the complete stack_frame (R4-R10, R0-xPSR)
 * in this example, R7 contains the address of the structure supplied by
    the process - the structure is assumed to hold the arguments to the
    kernel function.
 * to get the address and store it in kcaptr, it is simply a matter of
   assigning the value of R7 (arptr -> r7) to kcaptr
 */

    kcaptr = (struct kCallArgs *) argptr -> r7;
    switch(kcaptr -> code)
    {
    case GETID:
        kcaptr -> rtnvalue = RUNNING -> pid;
    break;
    case NICE:
       callerPCB = RUNNING;
       kcaptr -> rtnvalue = addPCB(removePCB(),kcaptr->arg1);
       /* Here, RUNNING has been changed to the PCB of the process that is to be
        * run next. If RUNNING does not point to the process that requested a nice()
        * then a context switch is required. Note that no registers are pushed/pulled
        * because the caller's registers have been pushed prior to arriving here and
        * the new RUNNING's registers will be pulled once this service call is concluded.
        */
       if(RUNNING != callerPCB)
       {
           callerPCB -> sp = get_PSP();
           set_PSP(RUNNING -> sp);
       }
    break;
    case SENDMSG:
        kcaptr -> rtnvalue = KernelSendMessage((struct Message *)kcaptr->arg1);
    break;
    case RECEIVEMSG:
        kcaptr -> rtnvalue = KernelRecvMessage((struct Message *)kcaptr->arg1);
    break;
    case TERMINATE:
        callerPCB = removePCB();
        free(&(callerPCB->sp));
        free(callerPCB);
        set_PSP(RUNNING -> sp);
    break;
    default:
        kcaptr -> rtnvalue = -1;
    }
}
}



