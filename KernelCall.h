/*
 * @file    KernelCall.h
 * @brief   Contains functionality to identify command
 *          passed from holding buffer and call its respective
 *          function with arguments
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date
 */

#ifndef KERNELCALL_H_
#define KERNELCALL_H_

enum kernelcallcodes {GETID, NICE, TERMINATE};

struct kCallArgs
{
    unsigned int code;
    int rtnvalue;
    unsigned int arg1;
    unsigned int arg2;
};

struct pMsgStruct
{
    int toMailbox;
    int fromMailbox;
    void * msg;
    int size;
};

#endif /* KERNELCALL_H_ */
