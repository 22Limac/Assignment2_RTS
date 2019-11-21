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

#define  NEXT i+1
#define  PREV i-1
#define  STARTING_INDEX 0
static Message * messagePool = NULL;
/* Message Queues */
static MailBox mailboxList[MAILBOX_AMOUNT];
static MailBox * freeMailBox;

/*
 * @brief   Initializes the doubly linked list connecting unowned
 *          mailboxs allowing bind any in constant time
 */
void initMailBoxList(void)
{
    int i = STARTING_INDEX;

    mailboxList[i].index = i;

    //initialize free to mailbox at starting index
    freeMailBox = &mailboxList[i];

    //first and last mailboxes must be initialized to point
    //to eachother to give circular doubly linked list functionality

    mailboxList[MAILBOX_MAX_INDEX].nextFree = &mailboxList[i];
    mailboxList[i].prevFree = &mailboxList[MAILBOX_MAX_INDEX];
    mailboxList[i].nextFree = &mailboxList[NEXT];
    for(i; i<MAILBOX_MAX_INDEX;i++)
    {
        mailboxList[i].index = i;
        mailboxList[i].nextFree = &mailboxList[NEXT];
        mailboxList[i].prevFree = &mailboxList[PREV];
    }
    mailboxList[i].index = i;
    mailboxList[i].prevFree = &mailboxList[PREV];
}

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
    if(desiredMB == ANY)
    {
        if(freeMailBox)
        {
            desiredMB = freeMailBox->index;
            freeMailBox->nextFree->prevFree=freeMailBox->prevFree;
            freeMailBox->prevFree->nextFree=freeMailBox->nextFree;
            freeMailBox = (freeMailBox->nextFree==freeMailBox)? NULL : freeMailBox->nextFree;

            mailboxList[desiredMB].owner = (struct ProcessControlBlock_*)getRunningPCB();
            mailboxList[desiredMB].head =mailboxList[desiredMB].tail = NULL;
        }
        else
        {
            desiredMB = BIND_FAIL;
        }

    }
    else
    {
        if(!(mailboxList[desiredMB].owner))
        {
            mailboxList[desiredMB].owner = (struct ProcessControlBlock_*)getRunningPCB();
            mailboxList[desiredMB].head =mailboxList[desiredMB].tail = NULL;
        }
        else
        {
            desiredMB = BIND_FAIL;
        }
    }
    return desiredMB;
}

int kernelUnbind(int releaseMB)
{
    if(mailboxList[releaseMB].owner == getRunningPCB())
    {
        mailboxList[releaseMB].owner = NULL;
        mailboxList[releaseMB].nextFree = freeMailBox;
        mailboxList[releaseMB].prevFree = freeMailBox->prevFree;
        freeMailBox->prevFree =  &mailboxList[releaseMB];
        freeMailBox = &mailboxList[releaseMB];
        return SUCCESS;
    }
    return UNBIND_FAIL;
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
   {return SEND_FAIL;}

   //check if the destination process is blocked
   if(mailboxList[destinationMB].owner->contents)
   { /* If the owner's PCB is blocked*/
     mailboxList[destinationMB].owner->from = fromMB;
     int copySize = (mailboxList[destinationMB].owner->size< size)?
                     mailboxList[destinationMB].owner->size :
                     size;
      memcpy(mailboxList[destinationMB].owner->contents, contents, copySize);
      addPCB(mailboxList[destinationMB].owner, mailboxList[destinationMB].owner->priority);
      *(mailboxList[destinationMB].owner->returnValue) = copySize;
      mailboxList[destinationMB].owner->contents = NULL;
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
               mailboxList[destinationMB].tail->next=NULL;
           }
           else//first message in mailbox
           {
               mailboxList[destinationMB].head = mailboxList[destinationMB].tail = newMessage;
               mailboxList[destinationMB].head->next=mailboxList[destinationMB].tail->next=NULL;

           }
       }
       else{ return SEND_FAIL;}
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
    {return RECV_FAIL;}

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
        removePCB();
        returnMB = &mailboxList[bindedMB].owner->from;
        mailboxList[bindedMB].owner->contents = contents;
        mailboxList[bindedMB].owner->size= *maxSize;
        mailboxList[bindedMB].owner->returnValue = maxSize;
        mailboxList[bindedMB].owner->sp = get_PSP();
        runningPCB = (struct ProcessControlBlock_*) getRunningPCB();
        set_PSP(runningPCB->sp);
    }
    return SUCCESS;
}
