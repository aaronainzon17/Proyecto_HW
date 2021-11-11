#include <LPC210x.H>                      
#include <inttypes.h>
#include "cola.h"
//#include "temporizador.h"
#include "gpio.h"

volatile struct Cola cola;

void cola_ini(void){
	cola.sig = 0;				// Último evento tratado
	cola.ult = 0;				// Siguiente posición en la que se introduce un evento
}

void cola_guardar_eventos(uint8_t  ID_evento,  uint32_t  auxData){
    cola.elementos[cola.ult].idEvento = ID_evento;
    cola.elementos[cola.ult].auxData = auxData;
    //cola.elementos[cola.sig].timeStamp = temporizador_leer();
    
    //Implementacion de la circularidad
    if (cola.ult < SIZE - 1){
		cola.ult++;
	}else{
		cola.ult = 0;
	}
    //Overflow 
	if (cola.ult == cola.sig){
		GPIO_marcar_salida(30,1);
		GPIO_escribir(30,1,1);
	}
}
// Funcion que comprueba si la cola tiene nuevos eventos 
// Como c no tiene datos de tipo bool: 0 -> false, 1 -> true
int cola_nuevos_eventos (void) {
    if (cola.sig != cola.ult){
        return 1;
    }else {
        return 0;
    }
}
// Funcion que lee el evento mas antiguo sin procesar de la cola 
void cola_leer_evevento_antiguo(struct EventInfo* elemento) {
    elemento->idEvento = cola.elementos[cola.sig].idEvento;
    elemento->auxData = cola.elementos[cola.sig].auxData;
    elemento->timeStamp = cola.elementos[cola.sig].timeStamp;
    if (cola.sig < SIZE - 1) {
			cola.sig ++;
	}else {
		cola.sig = 0; 
	}
}
