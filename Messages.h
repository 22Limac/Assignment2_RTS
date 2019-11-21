/*
 * @file    Message.h
 * @brief   Contains function prototypes allowing the kernel
 *          to utilize message queues.
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    29-Oct-2019 (created)
 * @date    4-Nov-2019 (edited)
 */
#pragma once
#include "Process.h"
/* Maximum number of message queues allowed */
#define MAILBOX_AMOUNT 16
#define MAILBOX_MAX_INDEX MAILBOX_AMOUNT - 1
#define MESSAGE_SYS_LIMIT 32

/* Structure containing information about messages */
typedef struct Message_
{

    /* ID of destination process */
    int from;

    /*Next pointer for linked list*/
    struct Message_* next;
    /* Size in bytes of message */
    int size;

    char contents[MESSAGE_SYS_LIMIT];
}Message;


/* Structure comprising a single message queue */
typedef struct MailBox_
{
    /* Owner of message queue */
    struct ProcessControlBlock_ * owner;
    /* Index of first occupied slot in message queue */
    Message* head;
    /* Index of next empty slot in message queue */
    Message* tail;

    struct MailBox_ * nextFree;

    struct MailBox_ * prevFree;

    int index;

}MailBox;

#ifndef GLOBAL_MESSAGES
#define GLOBAL_MESSAGES

extern int kernelBind(int);
extern int kernelUnbind(int);
extern int kernelSend(int,int,void *, int);
extern int kernelReceive(int,int*,void*,int*);
extern void initMessagePool(void);
extern void initMailBoxList(void);

#else

int kernelSend(int,int,void *, int);
int kernelReceive(int,int*,void*,int*);
void addToPool(Message *);
Message * retrieveFromPool(void);

#endif /* GLOBAL_SVC */
