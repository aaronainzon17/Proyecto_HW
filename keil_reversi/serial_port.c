#include <LPC210x.H>                     /* LPC21xx definitions               */
#include "cola.h"
#include "eventos.h"

#define CR     0x0D

void RSI_uart0 (void) __irq;

void init_serial (void)  {               /* Initialize Serial Interface       */
  PINSEL0 = PINSEL0 | 0x5;                				 /* Enable RxD0 and TxD0              */
  U0LCR = 0x83;                          /* 8 bits, no Parity, 1 Stop bit     */
  U0DLL = 200;                            /* 9600 Baud Rate @ 15MHz VPB Clock  */
  U0LCR = 0x03;                          /* DLAB = 0       										*/
	U0IER = U0IER | 0x7;
	VICVectAddr7 = (unsigned long)RSI_uart0;       // set interrupt vector in 0
	// 0x20 bit 5 enables vectored IRQs. 
	// 4 is the number of the interrupt assigned. Number 4 is the Timer 0 (see table 40 of the LPC2105 user manual  
	VICVectCntl7 = 0x20 | 6; 
	VICIntEnable = VICIntEnable | (1<<6);	
}


/* implementation of putchar (also used by printf function to output data)    */
// Revisar THRE IE
void sendchar (int ch)  {                 /* Write character to Serial Port    */
  U0THR = ch;
}

int getchar (void)  {                     /* Read character from Serial Port   */

  while (!(U0LSR & 0x01));

  return (U0RBR);
}


void RSI_uart0 (void) __irq{                     /* Read character from Serial Port   */
	switch(U0IIR & 0x00000007){ //sacar bits del 3 al 1
		// Hay datos que leer
		case 0x4 : 
			// Guarda el evento de pulsacion de una tecla junto con el valor de
			// la tecla que se ha pulsado
			cola_guardar_eventos(ID_UART0,U0RBR);		/* Save character*/
		break;
		
		// Listo para una nueva escritura
		case 0x2 : 
			// Guarda el evento que indica que la uart esta de nuevo lista para recibir 
			// una nueva escritura
			cola_guardar_eventos(ID_Evento_RDY,0);
		break;
	}
	// Acknowledge Interrupt
	VICVectAddr = 2;
}
