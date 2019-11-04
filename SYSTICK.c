/*
 * @file    SYSTICK.c
 * @brief   Contains SYSTICK timer handling functionality. Facilitates
 *          setup of the timer as well as its ISR requesting a context
 *          switch.
 * @author  Liam JA MacDonald
 * @author  Patrick Wells
 * @date    29-Oct-2019 (created)
 * @date    1-Nov-2019 (edited)
 */
#define GLOBAL_SYSTICK
/* Register Addresses */
#define REG_ST_CTL    (*((volatile unsigned long *)0xE000E010))
#define REG_ST_RELOAD (*((volatile unsigned long *)0xE000E014))

/* Control/Status Bit Locations */
#define ST_CTL_COUNT      (0x00010000UL)
#define ST_CTL_CLK_SRC    (0x00000004UL)
#define ST_CTL_INTEN      (0x00000002UL)
#define ST_CTL_ENABLE     (0x00000001UL)

/* SYSTICK Reload period: 16 777 777 / 100 = 167 777 */
#define ST_RELOAD_PERIOD  (0x00028F61UL)

/* Macro used to request a pendSV call */
#define CALLPENDSV ((*(volatile unsigned long *)0xE000ED04) |= 0x10000000UL)

/*
 * @brief   Initializes the SYSTICK timer and configures its interrupt.
 *          The SYSTICK timer will begin operation and cause interrupts
 *          as soon as this function is finished.
 *
 */
void initSYSTICK(void)
{
    /* Set interrupt period to every 1/10 of a second */
    REG_ST_RELOAD = ST_RELOAD_PERIOD;

    /* Set counter enable and internal clock source bits in control register */
    REG_ST_CTL |= ST_CTL_CLK_SRC | ST_CTL_ENABLE;

    /* Enable SysTick counter interrupts */
    REG_ST_CTL |= ST_CTL_INTEN;

    return;
}

/*
 * @brief ISR of SYSTICK requesting a context switch
 *
 */
void SYSTICKHandler(void)
{
    /* Request a pendSV call */
    CALLPENDSV;
}
