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
	U0IER = U0IER | 0x7;														//Interrupt enable register
	VICVectAddr7 = (unsigned long)RSI_uart0;       //Configuramos el vector de interrupciones
	  
	VICVectCntl7 = 0x20 | 6; 
	VICIntEnable = VICIntEnable | (1<<6);	
}


/* implementation of putchar (also used by printf function to output data)    */
void sendchar (int ch)  {                 /* Write character to Serial Port    */
  U0THR = ch;																//Transmit holding register
}

int getchar (void)  {                     /* Read character from Serial Port   */

  while (!(U0LSR & 0x01));

  return (U0RBR);
}

//RSI de la UART
void RSI_uart0 (void) __irq{             
	switch(U0IIR & 0x00000007){ //Miramos en U0IIR los bits que nos interesan
		// Se ha terminado de enviar un caracter
		case 0x2 : 
			cola_guardar_eventos(ID_Continuar,0);//Encolamos evento para continuar escribiendo
		break;
		//Se a recibido un caracter por la UART
		case 0x4 : 
			cola_guardar_eventos(ID_UART,U0RBR);		//llamaremos al gestor con lo que hemos recibido de la interrupcion de la UART
		break;
		
		
	}
	VICVectAddr = 2;// Acknowledge Interrupt
}
