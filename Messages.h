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
#define MESSAGE_SYS_LIMIT 32


typedef union FromMB_
{
    int value;
    int* addr;
}FromMB;

typedef union Contents_
{
    char value[MESSAGE_SYS_LIMIT];
    char* addr;
}Contents;

/* Structure containing information about messages */
typedef struct Message_
{

    /* ID of destination process */
    FromMB from;

    /*Next pointer for linked list*/
    struct Message_* next;
    /* Size in bytes of message */
    int size;

    Contents contents;
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


}MailBox;




#ifndef GLOBAL_MESSAGES
#define GLOBAL_MESSAGES

extern int kernelSend(int,int,void *, int);
extern int kernelReceive(int,int*,void*,int*);
void initMessagePool(void);

#else

int kernelSend(int,int,void *, int);
int kernelReceive(int,int*,void*,int*);
void addToPool(Message *);
Message * retrieveFromPool(Message*);

#endif /* GLOBAL_SVC */
