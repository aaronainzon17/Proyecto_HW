#include <LPC210X.H>
#include "boton_eint0.h"
#include "gpio.h"
#include "eventos.h"
#include "gestor_pulsacion.h"
#include "gestor_alarmas.h"
#include "timer0.h"

static volatile unsigned int ESTADO = no_pulsado;
static volatile unsigned int Boton;
static volatile struct EventInfo Pulsacion_alarma;

//Maquina de estados de las pulsaciones
void Gestor_Pulsacion_Control(uint32_t Id_Evento){
	uint32_t aux;
	switch(ESTADO){ //Dos eventos posibles; Alarma o pulsacion
		case no_pulsado:
			if(Id_Evento == ID_EINT1 || Id_Evento == ID_EINT2){//Se ha pulsado un bot?n
				ESTADO = pulsado;
				Boton = Id_Evento;
				// Meter los campos de la alarma
				Pulsacion_alarma.idEvento = ID_Alarma;
				Pulsacion_alarma.timeStamp = temporizador_leer();
				aux = (Id_Evento & 0xFF000000);
				//1(0x1) porque es periodico y 1000(0X0003E8) de periodo -> (0X8003E8): 24 bits y quedan los 8 de id 
				Pulsacion_alarma.auxData = (aux | 0x008003E8); // 8 bits de mas peso id de evento, bit 9 si es periodica o no y 23 bits el tiempo de periodo
				gestor_alarmas_control_cola(Pulsacion_alarma); //prepara alarma
			}
		break;
		case pulsado:
			if(Id_Evento == ID_Alarma){//LLega alarma para ver si sigue pulsado el botón
				switch(Boton){ 
					case 1:
						EXTINT |= 0x2;
						if((EXTINT & 0x00000002)!= 2){// El botón ya no sigue pulsado
							eint1_clear_nueva_pulsacion();	// Reseteamos la pulsacion
							Pulsacion_alarma.idEvento = ID_Alarma;
							Pulsacion_alarma.timeStamp = temporizador_leer();
							Pulsacion_alarma.auxData = 0x01000000;
							gestor_alarmas_control_cola(Pulsacion_alarma); //parar la alarma introduciendo 0 en el retardo
							ESTADO = no_pulsado;
						}
					break;
					case 2:
						EXTINT |= 0x4;
						if((EXTINT & 0x00000004)!= 4){// El botón ya no sigue pulsado
							eint2_clear_nueva_pulsacion();
							Pulsacion_alarma.idEvento = ID_Alarma;
							Pulsacion_alarma.timeStamp = temporizador_leer();
							Pulsacion_alarma.auxData = 0x02000000;
							gestor_alarmas_control_cola(Pulsacion_alarma); //parar la alarma introduciendo 0 en el retardo
							ESTADO = no_pulsado;
						}
					break;
				}
			}
		break;
	}
}
