#include <LPC210x.H>                      
#include <inttypes.h>
#include "cola.h"
//#include "temporizador.h"
#include "gpio.h"
//static
struct Cola cola;

void cola_ini(void){
	cola.sig = -1;				// siguiente evento a ser tratado
	cola.ult = -1;				// Siguiente posicion en la que se introduce un evento
}

int cola_llena(void){
	if((cola.sig == 0 && cola.ult == SIZE - 1) || (cola.sig == cola.ult + 1)){ //If que te dice si si metes un evento algun evento se va a quedar sin tartar
		GPIO_marcar_salida(30,1);	//Ponemos a 1 el bit 30 de la gpio
		GPIO_escribir(30,1,1);
		return 1;
	}else{//LA cola no esta llena
		GPIO_marcar_salida(30,1);//Ponemos a 0 el bit 30 de la gpio
		GPIO_escribir(30,1,0);
		return 0;
	}
}

void cola_guardar_eventos(uint8_t  ID_evento,  uint32_t  auxData){
		while(cola_llena() != 0){}
		//Implementacion de la circularidad
		if (cola.ult < SIZE - 1){
			cola.ult++;
		}else{
			cola.ult = 0;
		}
		cola.elementos[cola.ult].idEvento = ID_evento;
		cola.elementos[cola.ult].auxData = auxData;
		//cola.elementos[cola.sig].timeStamp = temporizador_leer();
}
// Funcion que comprueba si la cola tiene nuevos eventos 
int cola_nuevos_eventos (void) {
    if (cola.sig != cola.ult){
        return 1;
    }else {
        return 0;
    }
}
// Funcion que lee el evento mas antiguo sin procesar de la cola 
void cola_leer_evevento_antiguo(struct EventInfo* elemento) {
		if (cola.sig < SIZE - 1) {
				cola.sig ++;
		}else {
			cola.sig = 0; 
		}
    elemento->idEvento = cola.elementos[cola.sig].idEvento;
    elemento->auxData = cola.elementos[cola.sig].auxData;
    elemento->timeStamp = cola.elementos[cola.sig].timeStamp;
}
