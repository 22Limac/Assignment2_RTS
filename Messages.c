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
#include <stdio.h>
#include <stdlib.h>

static Message * messagePool = NULL;
/* Message Queues */
static MailBox mailboxList[MAILBOX_AMOUNT];

void addToPool(Message * newMsg)
{
    newMsg->from =NULL;
    newMsg->size= NULL;
    *(newMsg->contents)=NULL;
    newMsg->next = messagePool;
    messagePool = newMsg;
}

Message * retrieveFromPool()
{
    Message * newPtr = messagePool;
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

int kernelBind(int desiredMB)
{
        if(!(mailboxList[desiredMB].owner))
        {
            mailboxList[desiredMB].owner = (struct ProcessControlBlock_*)getRunningPCB();
            mailboxList[desiredMB].head =mailboxList[desiredMB].tail = NULL;
            return desiredMB;
        }
        return FAILURE;
}

int kernelUnbind(int releaseMB)
{
    if(mailboxList[releaseMB].owner == getRunningPCB())
    {
        mailboxList[releaseMB].owner = NULL;
        return SUCCESS;
    }
    return FAILURE;
}

/*
 * @brief   Adds message to a mailbox, if destination process is blocked; it transfers message
 *          and unblocks
 * @param   [in] int destinationMB: MB # of the destination process
 *          [in] int fromMB: MB # of the sending process
 *          [in] void* contents: data to be sent
 *          [in] int size: amount of data measured in bytes
 * @return  int: 1->success, -1->failure
 */
int kernelSend(int destinationMB, int fromMB, void * contents, int size)
{
   PCB * runningPCB = (struct ProcessControlBlock_*) getRunningPCB();

   //check the validity of arguments
   if((mailboxList[fromMB].owner != runningPCB)||
      (!(mailboxList[destinationMB].owner))||
      (MESSAGE_SYS_LIMIT<size))
   {return FAILURE;}

   //check if the destination process is blocked
   if(mailboxList[destinationMB].owner->message)
   { /* If the owner's PCB is blocked*/
     mailboxList[destinationMB].owner->message->from = fromMB;
     int copySize = (mailboxList[destinationMB].owner->message->size< size)?
                     mailboxList[destinationMB].owner->message->size :
                     size;
      memcpy(mailboxList[destinationMB].owner->message->contents, contents, copySize);
      addPCB(mailboxList[destinationMB].owner, mailboxList[destinationMB].owner->priority);
      addToPool(mailboxList[destinationMB].owner->message);
      *(mailboxList[destinationMB].owner->returnValue) = copySize;
      mailboxList[destinationMB].owner->message = NULL;

   }
   else
   {
       //if not blocked, fill a message structure from the
       //message pool and put it in the mailbox
       Message * newMessage = retrieveFromPool();
       if(newMessage)
       {
           newMessage->from = fromMB;
           newMessage->size = size;
           memcpy(newMessage->contents, contents, size);
           if(mailboxList[destinationMB].head)
           {
               mailboxList[destinationMB].tail->next = newMessage;
               mailboxList[destinationMB].tail = newMessage;
           }
           else//first message in mailbox
           {mailboxList[destinationMB].head = mailboxList[destinationMB].tail = newMessage;}
       }
       else{ return FAILURE;}
   }
   return SUCCESS;
}

/*
 * @brief   Take message from a mailbox, blocks if mailbox is empty
 *          and unblocks
 * @param   [in] int bindedMB: MB # of the receiving process
 *          [out] int* returnMB: MB # of the process that sent the message
 *          [in/out] void* contents: address where data is stored
 *          [in/out] int* maxSize: [in]maximum amount of bytes the process will take
 *                                 [out] amount of bytes that were copied
 * @return  int: -1->failure, 1->success
 */
int kernelReceive(int bindedMB, int* returnMB, void * contents, int * maxSize)
{
    PCB * runningPCB = (struct ProcessControlBlock_*) getRunningPCB();
    if((mailboxList[bindedMB].owner != runningPCB)||(MESSAGE_SYS_LIMIT<*maxSize))
    {return FAILURE;}

    if(mailboxList[bindedMB].head)
    {
// Mailbox contains at least one message

        *returnMB = mailboxList[bindedMB].head->from;
        int copySize = (mailboxList[bindedMB].head->size<*maxSize) ?
                mailboxList[bindedMB].head->size :
                *maxSize;
        memcpy(contents, mailboxList[bindedMB].head->contents, copySize);
        Message * temp = mailboxList[bindedMB].head;
        mailboxList[bindedMB].head = mailboxList[bindedMB].head ->next;
        addToPool(temp);
    }
    else
    {
// Mailbox empty; block

        Message * newMessage = retrieveFromPool();
        if(newMessage)// Message successfully retrieved from pool
        {
            removePCB();
            returnMB = &newMessage->from;
            contents = newMessage->contents;
            newMessage->size= *maxSize;
            mailboxList[bindedMB].owner->message = newMessage;
            mailboxList[bindedMB].owner->returnValue = maxSize;
            mailboxList[bindedMB].owner->sp = get_PSP();
            runningPCB = (struct ProcessControlBlock_*) getRunningPCB();
            set_PSP(runningPCB->sp);

        }
        else{ return FAILURE;}
    }
    return SUCCESS;
}
