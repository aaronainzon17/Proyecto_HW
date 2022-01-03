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

//Funcion que escribe una nueva cadena. Recibe como parámertros la fila, columna y nuevo valor introducidos por la UART en buffer
void escritura_comenzar(const char buffer2[]){
	int i;
	unsigned int bufferlen = strlen(buffer2);
	for(i=ultimo;i<(bufferlen+ultimo);i++){	
		buffer[i%TAM_BUFFER]=buffer2[i-ultimo];//Se copia el buffer de entrada en el utilizado para escribir
	}	
	ultimo=(ultimo+bufferlen)%TAM_BUFFER;	//Actualiza ultimo
	if(PrimerCaracter != 1){//No se esta escribiendo	
		sendchar(buffer[cursor]);		
		cursor = (cursor+1) %TAM_BUFFER;//Actualiza cursor	
		PrimerCaracter=1;//Se ha comezado a escribir
	}
}
//Funcion para continuar la escritura
void escritura_continuar(void){	
	//Se a acabado de escribir
	if(ultimo==cursor){	
		PrimerCaracter=0;	
	}else{//No hemos finalizado la escritura		
		sendchar(buffer[cursor]);		
		cursor = (cursor+1)%TAM_BUFFER;	//Actualizamos el cursor
	}
}


