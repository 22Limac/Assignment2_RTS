/*
 * @file    KernelCall.h
 * @brief   Contains functionality to identify command
 *          passed from holding buffer and call its respective
 *          function with arguments
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    20-Oct-2019 (created)
 * @date    4-Nov-2019 (edited)
 */

#ifndef KERNELCALL_H_
#define KERNELCALL_H_

enum kernelcallcodes {GETID, NICE, SENDMSG, RECEIVEMSG, TERMINATE};

struct kCallArgs
{
    unsigned int code;
    int rtnvalue;
    unsigned long arg1;
    unsigned long arg2;
};

struct pMsgStruct
{
    int toMailbox;
    int fromMailbox;
    void * msg;
    int size;
};

#endif /* KERNELCALL_H_ */
