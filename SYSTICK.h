/*
 * @file    SYSTICK.h
 * @brief   Contains functionality to identify command
 *          passed from holding buffer and call its respective
 *          function with arguments
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    4-Nov-2019 (edited)
 */

#ifndef SYSTICK_H_
#define SYSTICK_H_

extern void initSYSTICK(void);

#else

void initSYSTICK(void);

#endif /* SYSTICK_H_ */
