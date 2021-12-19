#include <LPC210X.H>
#include "gpio.h"
#include "eventos.h"
#include "cola.h"
#include <string.h>
#include "gestor_UART.h"
#include "serial_port.h"

#define TAM_BUFFER 3000

static volatile unsigned int cursor=0;
static volatile unsigned int ultimo=0;
static volatile unsigned int PrimerCaracter=0;
static volatile char buffer[TAM_BUFFER];

void resume_write(void){	
	// Si aun quedan caracteres por escribir
	if(cursor!=ultimo){	
		// Envia un caracter a la uart0
		sendchar(buffer[cursor]);		
		// Avanza en una unidad el cursor de escritura
		cursor = (cursor+1)%TAM_BUFFER;	
	}else{		
		// Ha acabado la escritura
		PrimerCaracter=0;		
	}
}


void write_string(const char buffer2[]){
	// Variable auxiliar para recorrer el buffer de entrada
	int i;
	// Longitud del buffer de entrada
	unsigned int longitud_buffer = strlen(buffer2);
	// Bucle para copiar el buffer de entrada en el buffer
	// de escritura
	for(i=ultimo;i<(longitud_buffer+ultimo);i++){	
		// Copia el caracter correspondiente del buffer de entrada
		// en el buffer de escritura
		buffer[i%TAM_BUFFER]=buffer2[i-ultimo];
	}	
	// Se actualiza la posicion del ultimo caracter que no se ha escrito
	ultimo=(ultimo+longitud_buffer)%TAM_BUFFER;	
	// Si no hay ninguna escritura en curso
	if(PrimerCaracter != 1){	
		// Se envia el primer caracter a la uart0
		sendchar(buffer[cursor]);		
		// Avanza en una unidad el cursor de escritura
		cursor = (cursor+1) %TAM_BUFFER;		
		// Se indica que ya se ha realizado la primera escritura del buffer actual
		PrimerCaracter=1;
	}
}
