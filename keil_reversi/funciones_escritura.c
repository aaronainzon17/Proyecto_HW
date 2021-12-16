#include <LPC210X.H>
#include "gpio.h"
#include "eventos.h"
#include "cola.h"
#include <string.h>
#include "gestor_UART.h"
#include "serial_port.h"

#define MAX_STRING_SIZE 3000

static volatile unsigned int cursorEscritura=0;
static volatile unsigned int lastPos=0;
static volatile unsigned int firstWrite=0;
static volatile char buffer[MAX_STRING_SIZE];

void resume_write(void){	
	// Si aun quedan caracteres por escribir
	if(cursorEscritura!=lastPos){	
		// Envia un caracter a la uart0
		sendchar(buffer[cursorEscritura]);		
		// Avanza en una unidad el cursor de escritura
		cursorEscritura = (cursorEscritura+1)%MAX_STRING_SIZE;	
		// Como no ha acabado de escribir y espera una interrupcion
		// de la uart0 indicando que puede escribir de nuevo indica
		// que el procesador no entre en power-down	
	}else{		
		// Ha acabado la escritura
		firstWrite=0;		
		// El procesador ya puede entrar en modo power-down
	}
}


void write_string(const char buff[]){
	// Variable auxiliar para recorrer el buffer de entrada
	int i;
	// Longitud del buffer de entrada
	unsigned int len = strlen(buff);
	// Bucle para copiar el buffer de entrada en el buffer
	// de escritura
	for(i=lastPos;i<(len+lastPos);i++){	
		// Copia el caracter correspondiente del buffer de entrada
		// en el buffer de escritura
		buffer[i%MAX_STRING_SIZE]=buff[i-lastPos];
	}	
	// Se actualiza la posicion del ultimo caracter que no se ha escrito
	lastPos=(lastPos+len)%MAX_STRING_SIZE;	
	// Si no hay ninguna escritura en curso
	if(firstWrite != 1){	
		// Se envia el primer caracter a la uart0
		sendchar(buffer[cursorEscritura]);		
		// Avanza en una unidad el cursor de escritura
		cursorEscritura = (cursorEscritura+1) %MAX_STRING_SIZE;		
		// Se indica que ya se ha realizado la primera escritura del buffer actual
		firstWrite=1;
	}
}
