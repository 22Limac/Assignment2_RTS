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
#define NUM_MSG_QUEUES (16U)
/* Maximum number of entries that can be contained in a queue */
#define MAX_QUEUE_SIZE (16U)

/* Structure containing information about messages */
struct Message
{
    /* ID of source process */
    unsigned char source;
    /* ID of destination process */
    unsigned char dest;
    /* Pointer to message contents */
    void * msgptr;
    /* Size in bytes of message */
    unsigned int size;
};


/* Structure comprising a single message queue */
struct MessageQueue
{
    /* Index of first occupied slot in message queue */
    unsigned char top;
    /* Index of next empty slot in message queue */
    unsigned char bottom;
    /* Size indicator of message queue */
    unsigned char size;
    /* Array containing queue entries */
    struct Message array[MAX_QUEUE_SIZE];
    /* Owner of message queue */
    PCB * owner;
};


/* Message Queues */
struct MessageQueue Mailbox[NUM_MSG_QUEUES];

/* Function prototypes */
unsigned char enqueueMessage(struct Message * msginfo);
struct Message dequeueMessage(unsigned char index);
