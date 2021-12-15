#include <LPC210X.H>
#include "gpio.h"
#include "eventos.h"
#include "cola.h"
#include "gestor_UART.h"
#include "funciones_escritura.h"

static volatile unsigned int ESTADO = inicio;
static volatile int buffer[10];
static volatile unsigned int bufferLen;


//M?quina de estados del gestor de comandos.
//Devuelve un entero para indicar si est? esperando un evento 
void gestor_UART(uint32_t entrada){
	int ultimo = U0RBR;
	U0THR = ultimo;
	switch(ESTADO){
		case inicio:
				if (entrada == '#'){
					ESTADO=fin;
					bufferLen = 0;
				}
		break;
		
		case fin:
			if(entrada == '!'){
				if(buffer[0]=='R' && buffer[1]=='S' && buffer[2]=='T'){
					write_string("\n");
					cola_guardar_eventos(ID_RST,0);
				}else if(buffer[0]=='N' && buffer[1]=='E' && buffer[2]=='W'){
					write_string("\n");
					cola_guardar_eventos(ID_NEW,0);
				}else if(((buffer[0]-0x30)+(buffer[1]-0x30)+(buffer[2]-0x30))% 0x8 == (buffer[3]-0x30)){
					write_string("\n");
					cola_guardar_eventos(ID_JUGADA,(buffer[0]%0x30)*100+(buffer[1]%0x30)*10+(buffer[2]%0x30));
				}
				ESTADO=inicio;
			}else{
				if(bufferLen<4){
					buffer[bufferLen] = entrada;
					bufferLen++;
				}else{
					ESTADO=inicio;
				}
			}
		break;
	}
}
