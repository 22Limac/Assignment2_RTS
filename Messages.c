/*
 * @file    Messages.c
 * @brief
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    29-Oct-2019 (created)
 * @date    4-Nov-2019 (edited)
 */
#define GLOBAL_MESSAGES
#include "Messages.h"


/*
 * @brief   Adds message to a message queue
 * @param   [in] struct Message * msginfo: pointer to structure containing message information
 * @return  unsigned char: result of enqueueing, 0->success, 1->failure
 */
unsigned char enqueueMessage(struct Message * msginfo)
{
    unsigned char result = 0U;

    /* First check if queue is currently full */
    if(Mailbox[msginfo->dest].size == MAX_QUEUE_SIZE)
    {
        /* Mailbox is currently full so adjust return value */
        result = 1U;
    }
    else
    {
        /* Mailbox is not full so insert message */
        Mailbox[msginfo->dest].array[Mailbox[msginfo->dest].bottom] = *msginfo;
        Mailbox[msginfo->dest].bottom = (Mailbox[msginfo->dest].bottom + 1U) % MAX_QUEUE_SIZE;
    }

    return result;
}

/*
 * @brief   Removes a message from desired mailbox
 * @param   [in] unsigned char index: index of queue from which an entry
 *          is to be taken
 * @return  Message: message taken from mailbox. It points to NULL if
 *          desired queue is empty
 */
struct Message dequeueMessage(unsigned char index)
{
    struct Message rtnmessage = {0, 0, 0, 0};

    /* First check if queue is currently empty */
    if(Mailbox[index].size == 0U)
    {
        /* Mailbox is empty so leave result as is */
    }
    else
    {
        /* Mailbox can be dequeued so obtain top entry */
        rtnmessage = Mailbox[index].array[Mailbox[index].top];
        Mailbox[index].top = (Mailbox[index].top + 1U) % MAX_QUEUE_SIZE;
        Mailbox[index].size--;
    }

    return rtnmessage;
}
