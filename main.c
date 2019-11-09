/*
 * @file    main.c
 * @brief   Entry point to the light-weight messaging kernel
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    4-Nov-2019 (edited)
 */

#include "SVC.h"
#include "Process.h"
#include "SYSTICK.h"
#include "UART.h"
#include "Messages.h"

extern int nice(int newPriority);

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
 */
void Priority4Process1(void)
{

    while(1)
    {
        forceOutput('Z');
    }

}


/*
 * @brief   definition of process used to test priority management
 */
void Priority4Process2(void)
{


    while(1)
    {
        forceOutput('U');
    }

}

/*
 * @brief   definition of a process used to set up and output
 *          to UART0
 */

//void UARTProcess(void)
//{
////    unsigned char output;
//
//    // Initialize UART0
//    UART0_Init();
//    InterruptEnable(5);
//
//    // Loop indefinitely
//    while(1)
//    {
////        for(output = '!'; output <= '~'; output++) //TODO: Magic characters
////        {
//            forceOutput('U');
////            forceOutput('\n'); //TODO: Magic character
////        }
//    }
//
//    return;
//}


/*
 * @brief   registers processes.
 *          Sets idleProcess as Running
 *          traps kernel with service call
 *
 * */
int main(void)
{
    initMessagePool();
    /* Register idle process first */
    registerProcess(idleProcess, 1, 0);

    /* Register other test processes */
    registerProcess(Priority4Process1, 2, 3);
    registerProcess(Priority4Process2, 3, 3);

    /* Initialize required hardware + interrupts */
    initpendSV();
    UART0_Init();           // Initialize UART0
    InterruptEnable(INT_VEC_UART0);       // Enable UART0 interrupts
    UART0_IntEnable(UART_INT_RX | UART_INT_TX); // Enable Receive and Transmit interrupts
    SysTickPeriod(TENTH_WAIT);
    SysTickIntEnable();

    /* Trap to begin running processes */
    SVC();

    /* Should never reach here but if we do just exit program */
	return 0;
}
