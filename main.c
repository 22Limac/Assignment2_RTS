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

    /* Should never reach here but if so do a terminate */
    return;
}

/*
 * @brief   definition of process used to test priority management
 */
void Priority4Process1(void)
{
    unsigned int counter = 0U;

    nice(3);

    while(1)
    {
        counter++;
    }


    return;
}


/*
 * @brief   definition of process used to test priority management
 */
void Priority4Process2(void)
{
    unsigned int counter = 0U;

    while(1)
    {
        counter++;
    }

    return;
}

/*
 * @brief   definition of a process used to set up and output
 *          to UART0
 */
/*
void UARTProcess(void)
{
    unsigned char output;

    /* Initialize UART0
    UART0_Init();
    InterruptEnable(5);

    /* Loop indefinitely
    while(1)
    {
        for(output = '!'; output <= '~'; output++) //TODO: Magic characters
        {
            forceOutput(output);
            forceOutput('\n'); //TODO: Magic character
        }
    }

    return;
}
*/

/*
 * @brief   registers processes.
 *          Sets idleProcess as Running
 *          traps kernel with service call
 *
 * */
int main(void)
{
    /* Register idle process first */
    registerProcess(idleProcess, 1, 0);

    /* Register other test processes */
    registerProcess(Priority4Process1, 2, 4);
    registerProcess(Priority4Process2, 3, 4);

    /* Initialize required hardware + interrupts */
    initpendSV();
    initSYSTICK();

    /* Trap to begin running processes */
    SVC();

    /* Should never reach here but if we do just exit program */
	return 0;
}
