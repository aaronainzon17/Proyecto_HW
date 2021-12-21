#include <LPC210x.H>                      
#include <inttypes.h>
#include "gestor_WD.h"
#include "SWI_functions.h"

void WT_init(int sec){
	// first, it checks for a previous watchdog time out 
	//WDTOF (bit 2)The Watchdog Time-Out Flag is set when the watchdog times out. This flag is cleared by software.
	if( WDMOD & 0x04 ) {					   //Se comprueba si ha saltado el WD
		WDMOD &= ~0x04;						   // Reseteamos el bit de WD
  }
	// Interval timeout Pclk*WDTC*4
	// valor minimo WDTC= PCLK * 256 * 4; valor maximo PCLK * 2^32 * 4
  WDTC  = sec*1500000;						   			// Set watchdog time out value
  WDMOD = 0x03;                            //Bit de Enable y bit de reset 
}

void feed_watchdog (void) {				   /* Reload the watchdog timer       */
// esta es la secuencia necesaria para que el watchdog se reinicialice. Si no se alimenta al WT antes de que termine la cuenta resetear? el sistema (si est? habilitado para ello)
// aunque se active el watchdog, este no har? nada hasta que no se le alimente por primera vez
//Important! Interrupts must be disabled during the feed sequence. An abort condition will occur if an interrupt happens during the feed sequence
  disable_isr();
	disable_isr_fiq();
	WDFEED = 0xAA;						   
  WDFEED = 0x55; 
	enable_isr();
	enable_isr_fiq();
}
