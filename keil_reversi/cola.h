/* guarda para evitar inclusiones múltiples (include guard) */
#ifndef COLA_H
#define COLA_H
#include <inttypes.h>
#include "eventos.h"

#define SIZE 32

struct Cola {
	struct EventInfo elementos[SIZE];
	int sig;
	int ult;
};

//Se crea una nueva cola
void cola_ini(void);
//Comprueba si la cola esta llena
int cola_llena(void);
//Funcion que guarda nuevos eventos en la cola
void cola_guardar_eventos(uint8_t  ID_evento,  uint32_t  auxData);
//Funcion que comprueba si la cola tiene nuevos eventos  
int cola_nuevos_eventos (void);
//Funcion que lee el evento mas antiguo sin procesar de la cola 
void cola_leer_evevento_antiguo(struct EventInfo* elemento);


#endif // COLA_H
