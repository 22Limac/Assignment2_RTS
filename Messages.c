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
#include "SVC.h"
#include "Process.h"
#include "KernelCall.h"
#include "Utilities.h"
static Message * messagePool = NULL;
/* Message Queues */
static MailBox mailboxList[MAILBOX_AMOUNT];

void addToPool(Message * newMsg)
{
    newMsg->next = messagePool;
    messagePool = newMsg;
}

Message * retrieveFromPool(Message * newPtr)
{
    newPtr = messagePool;
    messagePool = newPtr->next;
    return newPtr;
}

void initMessagePool(void)
{
    int i;
    for(i=0;i<MESSAGE_SYS_LIMIT;i++)
    {
        addToPool(malloc(sizeof(Message)));
    }
}


/*
 * @brief   Adds message to a message queue
 * @param   [in] struct Message * msginfo: pointer to structure containing message information
 * @return  unsigned char: result of enqueueing, 0->success, 1->failure
 */
int kernelSend(int destinationMB, int fromMB, void * contents, int size)
{
   const PCB * runningPCB = getRunningPCB();
   if((mailboxList[fromMB].owner != runningPCB)||
      (!mailboxList[destinationMB].owner)||
      (MESSAGE_SYS_LIMIT<size))
   {return FAILURE;}

   if(mailboxList[destinationMB].owner->message)
   { /* If the owner's PCB is blocked*/
     *(mailboxList[destinationMB].owner->message->from.addr) = fromMB;
     int copySize = (mailboxList[destinationMB].owner->message->size< size)?
                     mailboxList[destinationMB].owner->message->size :
                     size;
      memcpy(mailboxList[destinationMB].owner->message->contents.value, contents, copySize);
      addPCB(mailboxList[destinationMB].owner, mailboxList[destinationMB].owner->priority);
      addToPool(mailboxList[destinationMB].owner->message);
      *(mailboxList[destinationMB].owner->returnValue) = copySize;
      mailboxList[destinationMB].owner->message = NULL;

   }
   else
   {
       Message * newMessage = NULL;
       if(retrieveFromPool(newMessage))
       {
           newMessage->from.value = fromMB;
           newMessage->size = size;
           memcpy(newMessage->contents.value, contents, size);
           if(mailboxList[destinationMB].head)
           {
               mailboxList[destinationMB].tail->next = newMessage;
               mailboxList[destinationMB].tail = newMessage;
           }
           else
           {mailboxList[destinationMB].head = mailboxList[destinationMB].tail = newMessage;}
       }
       else{ return FAILURE;}
   }
   return SUCCESS;
}

/*
 * @brief   Removes a message from desired mailbox
 * @param   [in] unsigned char index: index of queue from which an entry
 *          is to be taken
 * @return  Message: message taken from mailbox. It points to NULL if
 *          desired queue is empty
 */
int kernelReceive(int bindedMB, int* returnMB, void * contents, int * maxSize)
{
    const PCB * runningPCB = getRunningPCB();
    if((mailboxList[bindedMB].owner != runningPCB)||(MESSAGE_SYS_LIMIT<*maxSize))
    {return FAILURE;}

    if(mailboxList[bindedMB].head)
    {
        *returnMB = mailboxList[bindedMB].head->from.value;
        int copySize = (mailboxList[bindedMB].head->size<*maxSize) ?
                mailboxList[bindedMB].head->size :
                *maxSize;
        memcpy(contents, mailboxList[bindedMB].head->contents.value , copySize);
        Message * temp = mailboxList[bindedMB].head;
        mailboxList[bindedMB].head = mailboxList[bindedMB].head ->next;
        addToPool(temp);
    }
    else
    {
        Message * newMessage = NULL;
        if(retrieveFromPool(newMessage))
        {
            removePCB();
            newMessage->from.addr = returnMB;
            newMessage->contents.addr = contents;
            newMessage->size= *maxSize;
            mailboxList[bindedMB].owner->message = newMessage;
            mailboxList[bindedMB].owner->returnValue = maxSize;

            SETPENDSVPRIORITY;
        }
        else{ return FAILURE;}
    }
    return SUCCESS;
}
