/******************************************************************************/
/*  This file is part of the uVision/ARM development tools                    */
/*  Copyright KEIL ELEKTRONIK GmbH 2002-2004                                  */
/******************************************************************************/
/*                                                                            */
/*  TIME.C:  Time Functions for 100Hz Clock Tick                              */
/*                                                                            */
/******************************************************************************/

#include <LPC210X.H>                            // LPC21XX Peripheral Registers
#include "Timer.h"
#include "pulsacion.h"

volatile long timeval0, timeval1;

void tc0 (void) __irq;    // Generate Interrupt 
void tc1 (void) __irq;    // Generate Interrupt 

/* Setup the Timer Counter 0 Interrupt */
void init_timer0 (void) {
    // configuration of Timer 0
		T0MR0 = 149999;                        // 10mSec = 150.000-1 counts
    T0MCR = 3;                     // Generates an interrupt and resets the count when the value of MR0 is reached
    T0TCR = 1;                             // Timer0 Enable
    // configuration of the IRQ slot number 0 of the VIC for Timer 0 Interrupt
		VICVectAddr0 = (unsigned long)tc0;          // set interrupt vector in 0
    // 0x20 bit 5 enables vectored IRQs. 
		// 4 is the number of the interrupt assigned. Number 4 is the Timer 0 (see table 40 of the LPC2105 user manual  
		VICVectCntl0 = 0x20 | 4;                   
    VICIntEnable = VICIntEnable | 0x00000010;                  // Enable Timer0 Interrupt
}


/* Timer Counter 0 Interrupt executes each 10ms @ 60 MHz CPU Clock */
void tc0 (void) __irq {
    timeval0++;
    T0IR = 1;                              // Clear interrupt flag
    VICVectAddr = 0;                            // Acknowledge Interrupt
}

/* Setup the Timer Counter 0 Interrupt */
void init_timer1 (void) {
    // configuration of Timer 0
		T1MR0 = 14999;                        // 1mSec = 15.000-1 counts
    T1MCR = 5;                   // Generates an interrupt and stops the count when the value of MR1 is reached
    T1TCR = 1;                            // Timer1 Enable
    // configuration of the IRQ slot number 1 of the VIC for Timer 0 Interrupt
		VICVectAddr1 = (unsigned long)tc1;          // set interrupt vector in 0
    // 0x20 bit 5 enables vectored IRQs. 
		// 5 is the number of the interrupt assigned. Number 5 is the Timer 1 (see table 40 of the LPC2105 user manual  
		VICVectCntl1 = 0x20 | 5;                   
    VICIntEnable = VICIntEnable | 0x00000020;                  // Enable Timer0 Interrupt
}


/* Timer Counter 1 Interrupt executes each 1ms @ 60 MHz CPU Clock */
void tc1 (void) __irq {
    timeval1++;
    T1IR = 1;                              // Clear interrupt flag
    VICVectAddr = 0;                            // Acknowledge Interrupt
}

