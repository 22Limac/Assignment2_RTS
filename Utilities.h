/*
 * @file    Utilities.h
 * @brief   frequently used constants
 *          Utilities.c function prototypes
 * @author  Liam JA MacDonald
 * @date    23-Sep-2019 (created)
 * @date    10-Oct-2019 (modified)
 */
#pragma     once

#include <stdio.h>
#include <stdlib.h>
#define     disable()   __asm(" cpsid i") //disable interrupts
#define     enable()    __asm(" cpsie i")//enable interrupts
#define     ENTER       0x0d //ASCII Characters
#define     BS          0x08
#define     NUL         0x00
#define     TRUE        1   // For using Integers as booleans
#define     FALSE       0
#define     SUCCESS     1   // To clarify returns values where possible
#define     FAILURE     -1
#define     NULL        0
#define     EMPTY       0       //Queue return values
#define     FULL        0


#ifndef     UTILITIES
#define     UTILITIES


#else

#endif /* UTILITIES */
