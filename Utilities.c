/*
 * Utilities.c
 *
 *  Created on: Nov 6, 2019
 *      Author: LiamMacDonald
 */
#define GLOBAL_UTILITIES
#define TWO_DIGITS 2
#include "Utilities.h"

void formatLineNumber(int val, char* rtn)
{
    if(val<TWO_DIGITS)//only values less than ten have zeros added to the tens placement
    {
        sprintf(rtn,"0%d",val);
    }
    else
    {
        sprintf(rtn,"%d",val);
    }
}

