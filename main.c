/*
 * @file    main.c
 * @brief   Entry point to the light-weight messaging kernel
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    13-Nov-2019 (edited)
 */

#include "SVC.h"
#include "Process.h"
#include "SYSTICK.h"
#include "UART.h"
#include "Messages.h"
#include "KernelCall.h"


int process = 0;
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
    int mailBox = 1;
    bind(mailBox);

    nice(2);
    char cont[]="hi 2";
    int size = 4;

    sendMessage(2, mailBox, cont, size);
    nice(1);
    unbind(mailBox);
}


/*
 * @brief   definition of process used to test priority management
 */
void Priority4Process2(void)
{
    int mailBox = 2;
    bind(mailBox);

    nice(2);
    int size = 4;
    char cont[4];
    int rtnMailBox;

    recvMessage(mailBox, &rtnMailBox, cont, size);
    forceOutput(cont[0]);

    unbind(mailBox);
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

    int registerResult = 0;

    /* Register idle process first */
    registerResult = registerProcess(idleProcess, 1, 0);

    /* Register other test processes */
    registerResult = registerProcess(Priority4Process1, 1, 4);
    registerResult = registerProcess(Priority4Process1, 2, 43);
    registerResult = registerProcess(Priority4Process1, 2, 4);
    registerResult = registerProcess(Priority4Process2, 3, 4);
    registerResult = registerProcess(Priority4Process2, 4, 4);

    /* Initialize required hardware + interrupts */
    initpendSV();
    UART0_Init();           // Initialize UART0
    InterruptEnable(INT_VEC_UART0);       // Enable UART0 interrupts
    UART0_IntEnable(UART_INT_RX | UART_INT_TX); // Enable Receive and Transmit interrupts
    SysTickPeriod(HUNDREDTH_WAIT);
    SysTickIntEnable();

    /* Trap to begin running processes */
    SVC();

    /* Should never reach here but if we do just exit program */
	return 0;
}
