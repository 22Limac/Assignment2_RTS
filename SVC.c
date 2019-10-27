/*
 * @file    SVC.c
 * @brief   Contains Service Call (SVC) trap handling functionality.
 *          Declares waitingToRun queues and the running PCB pointer;
 *          contains initialization and manipulation for both.
 *          **not sure if registerProcess belongs here or Process.h**
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date
 */
#include <stdio.h>
#include <stdlib.h>
#define GLOBAL_SVC
#include "SVC.h"
#include "Process.h"
#include "KernelCall.h"

#define PRIORITY_LEVELS 5
#define HIGH_PRIORITY 4
#define LOW_PRIORITY 0
extern void systick_init();
#define RUNNING waitingToRun[currentPriority]
static int currentPriority = 0;

static PCB * waitingToRun[PRIORITY_LEVELS];

/*
 * @brief   Allocates a new process stack frame and PCB
 *          for the process being registered.
 *          sets PCB sp and pid.
 *          calls addPCB to add PCB to waitingToRun with
 *          respective priority
 * @param   [in] void (*code)(void): pointer to the start of the process code
 *          [in] unsigned int pid: Process ID of process being registered
 *          [in] unsigned int priority: Processes initial priority
 * @return  int: **Not really sure how a process would fail registration
 *                 but thats what it's for**
 *
 * */
int registerProcess(void (*code)(void), unsigned int pid, unsigned int priority)
{

   unsigned long *processStack = (unsigned long *)malloc(512*sizeof(unsigned long));
   StackFrame *processSP = (StackFrame*) processStack;
   processSP -> psr = 0x01000000;
   processSP -> pc = (unsigned long)code;

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
 * @param   [in] PCB *new: PCB being added to the queue
 *          [in] unsigned int priority: priority level
 * */
int addPCB(PCB *new, unsigned int priority)
{

    if(waitingToRun[priority])
    {
        new->next = waitingToRun[priority];
        waitingToRun[priority] -> prev -> next = new;
        waitingToRun[priority] -> prev = new;
    }
    else
    {   //first in queue
        waitingToRun[priority] = new;
        waitingToRun[priority]->next = new;
        waitingToRun[priority]->prev = new;
    }
    currentPriority = (currentPriority<priority)? priority: currentPriority;
    return currentPriority;
}

PCB * removePCB()
{
    PCB * toRemove = RUNNING;
    if (RUNNING == RUNNING -> next )
    {
        RUNNING = NULL;
        decrementPriority();
    }
    else
    {
        RUNNING -> next -> prev = RUNNING -> prev;
        RUNNING -> prev ->next = RUNNING ->next;
        RUNNING = RUNNING -> next;
    }
    return toRemove;
}

void decrementPriority(void)
{
    do
    {
        currentPriority--;
    }
    while(!(RUNNING)&&currentPriority);
}

void pendSV(void)
{
    save_registers();
    RUNNING = RUNNING -> next;
    restore_registers();
}

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
PCB * toTerminate;

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
    /* Start SysTick */
//    systick_init();

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
        kcaptr -> rtnvalue = RUNNING ->pid;
    break;
    case NICE:
       kcaptr -> rtnvalue = addPCB(removePCB(),kcaptr->arg1);
    break;
    case TERMINATE:
        toTerminate = removePCB();
        free(&(toTerminate->sp));
        free(toTerminate);
    break;
    default:
        kcaptr -> rtnvalue = -1;
    }
}
}



