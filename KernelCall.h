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


typedef struct KernelCallArgs_
{
    unsigned long code;
    long rtnvalue;
    unsigned long arg1;
    unsigned long arg2;
}KernelArgs;

typedef struct SendMessage_
{
    int destinationMB;
    int fromMB;
    void * contents;
    int size;
}SendMessage;

typedef struct ReceiveMessage_
{
    int bindedMB;
    int * returnMB;
    void * contents;
    int maxSize;
}ReceiveMessage;


#endif /* KERNELCALL_H_ */
