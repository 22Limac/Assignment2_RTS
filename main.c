/*
 * @file    main.c
 * @brief   Entry point to the light-weight messaging kernel
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    13-Nov-2019 (edited)
 */
#include <string.h>
#include "Utilities.h"
#include "KernelCall.h"
#include "SVC.h"
#include "Process.h"
#include "SYSTICK.h"
#include "UART.h"
#include "Messages.h"


/*
 * @brief   definition of idleProcess; the first process registered
 *          by the kernel. It must always idle and will only be run
 *          if there are no other processes in place.
 * */
void idleProcess(void)
{
    /* Loop indefinitely */

    while(1);

}

/*
 * @brief   definition of process used to test priority management
 *          downgrades itself to priority 3
 */
void Priority4Process1(void)
{
    int mailBox = bind(ANY);
    printWarning(mailBox);

    int myID =getid();
    char idString[POSITION_DIGITS];
    formatLineNumber(myID, idString);
    printToLine(myID);
    printString("From PID:");
    printSequence(RED_TEXT);
    printString(idString);
    printSequence(CLEAR_MODE);
    printString(" | ");

    nice(2);
    char cont[9];
    int size = 9;
    int toMB = 2;

    int i =0;
    while(i<5)
    {
        strcpy(cont," *hi 2*\0");

        printWarning
        (
                sendMessage(toMB, mailBox, cont, size)
        );

        printWarning
        (
                recvMessage(mailBox, &toMB, cont, size)
        );

        printToLine(myID);
        printString(cont);
        i++;
    }

    printWarning
    (
           unbind(mailBox)
    );

}
void Priority3Process1(void)
{
    int myID =getid();
    char idString[POSITION_DIGITS];
    formatLineNumber(myID, idString);
    printToLine(myID);
    printString("From PID:");
    printSequence(RED_TEXT);
    printString(idString);
    printSequence(CLEAR_MODE);
    printString(" | ");
}


/*
 * @brief   definition of process used to test priority management
 */
void Priority4Process2(void)
{
    int mailBox = 2;

    printWarning
    (
            bind(mailBox)
    );

    int myID =getid();
    char idString[POSITION_DIGITS];
    formatLineNumber(myID, idString);
    printToLine(myID);
    printString("From PID:");
    printSequence(RED_TEXT);
    printString(idString);
    printSequence(CLEAR_MODE);
    printString(" | ");


    nice(2);
    int size = 9;
    char cont[9];
    int rtnMailBox;

    int i =0;
    while(i<5)
    {
        printWarning
        (
                recvMessage(mailBox, &rtnMailBox, cont, size)
        );

        printToLine(myID);
        printString(cont);
        strcpy(cont," *hi 1*\0");

        printWarning
        (
                sendMessage(rtnMailBox, mailBox, cont, size)
        );

        i++;
    }

    printWarning
    (
            unbind(mailBox)
    );
}


/*
 * @brief   registers processes.
 *          Sets highest priority process as Running
 *          traps kernel with service call
 *
 * */
int main(void)
{
    initMessagePool();
    initMailBoxList();

    int registerResult = 0;

    /* Register idle process first */
    registerResult |= registerProcess(idleProcess, 1, 0);

    /* Register other test processes */
    registerResult |= registerProcess(Priority4Process1, 10, 4);
    registerResult |= registerProcess(Priority4Process2, 20, 4);
//    registerResult = registerProcess(Priority3Process1, 10, 4);

    if(!registerResult)
    {
    /* Initialize required hardware + interrupts */
        initpendSV();
        UART0_Init();           // Initialize UART0
        InterruptEnable(INT_VEC_UART0);       // Enable UART0 interrupts
        UART0_IntEnable(UART_INT_RX | UART_INT_TX); // Enable Receive and Transmit interrupts
        SysTickPeriod(HUNDREDTH_WAIT);
        SysTickIntEnable();
        printSequence(CLEAR_SCREEN);

    /* Trap to begin running processes */
        SVC();
    }
    /* Should never reach here but if we do just exit program */
    printSequence(RED_TEXT);
    printString("*INVALID PROCESS*");
	return 0;
}
