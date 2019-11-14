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
#pragma once

enum kernelcallcodes {GETID, NICE, SENDMSG, RECEIVEMSG, TERMINATE, BIND, UNBIND};

typedef struct KernelCallArgs_
{
    unsigned long code;
    int rtnvalue;
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

#ifndef GLOBAL_KERNELCALL
#define GLOBAL_KERNELCALL

extern int bind(int);
extern int unbind(int);
extern int getid(void);
extern int nice(int);
extern void terminate(void);
extern int sendMessage(int, int, void *, int);
extern int recvMessage(int, int*, void *, int);

#else

#endif
