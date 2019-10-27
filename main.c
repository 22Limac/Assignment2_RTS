/*
 * @file    main.c
 * @brief   Entry point to the light-weight messaging kernel
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date
 */

#include "SVC.h"
#include "Process.h"
int process = 0;
/*
 * @brief   definition of processOne the first process registered
 *          by the kernel
 * */
void processOne(void)
{
    process = 1;
}

/*
 * @brief   registers processes.
 *          Sets processOne as Running
 *          traps kernel with service call
 *
 * */
int main(void)
{
    registerProcess(processOne, 1, 4);
    SVC();
	return 0;
}
