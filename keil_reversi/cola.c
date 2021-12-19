#include <LPC210x.H>                      
#include <inttypes.h>
#include "cola.h"
//#include "temporizador.h"
#include "gpio.h"
//static
struct Cola cola;
//Funcion que inicia los campos de la cola
void cola_ini(void){
	cola.siguiente_tratar = -1;				// siguiente evento a ser tratado
	cola.siguiente_encolar = -1;				// Siguiente posicion en la que se introduce un evento
}
//Funcion que evalua si la cola esta llena o no; 1 si esta llena, 0 en caso contrario
int cola_llena(void){
	if((cola.siguiente_tratar == 0 && cola.siguiente_encolar == SIZE - 1) || (cola.siguiente_tratar == cola.siguiente_encolar + 1)){ //If que te dice si si metes un evento algun evento se va a quedar sin tartar
		GPIO_marcar_salida(30,1);	//Ponemos a 1 el bit 30 de la gpio
		GPIO_escribir(30,1,1);
		return 1;
	}else{//LA cola no esta llena
		GPIO_marcar_salida(30,1);//Ponemos a 0 el bit 30 de la gpio
		GPIO_escribir(30,1,0);
		return 0;
	}
}
//Funcion que guarda un evento en la cola
void cola_guardar_eventos(uint8_t  ID_evento,  uint32_t  auxData){
		while(cola_llena() != 0){}//Si la cola esta llena se queda esperando
		//Implementacion de la circularidad
		if (cola.siguiente_encolar < SIZE - 1){
			cola.siguiente_encolar++;
		}else{
			cola.siguiente_encolar = 0;
		}
		cola.elementos[cola.siguiente_encolar].idEvento = ID_evento;
		cola.elementos[cola.siguiente_encolar].auxData = auxData;
		//cola.elementos[cola.sig].timeStamp = temporizador_leer();
}
// Funcion que comprueba si la cola tiene nuevos eventos 
int cola_nuevos_eventos (void) {
    if (cola.siguiente_tratar != cola.siguiente_encolar){
        return 1;
    }else {
        return 0;
    }
}
// Funcion que lee el evento mas antiguo sin procesar de la cola 
void cola_leer_evevento_antiguo(struct EventInfo* elemento) {
		if (cola.siguiente_tratar < SIZE - 1) {
				cola.siguiente_tratar ++;
		}else {
			cola.siguiente_tratar = 0; 
		}
		//Actualizamos los campos del evento
    elemento->idEvento = cola.elementos[cola.siguiente_tratar].idEvento;
    elemento->auxData = cola.elementos[cola.siguiente_tratar].auxData;
    elemento->timeStamp = cola.elementos[cola.siguiente_tratar].timeStamp;
}
