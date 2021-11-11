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

//void Gestor_Pulsacion_Maquina_Estados(struct Elemento E){
void Gestor_Pulsacion_Control(uint32_t Id_Evento){
	uint32_t aux;
	switch(ESTADO){ //Dos eventos posibles; Alarma o pulsación
		case no_pulsado:
			if(Id_Evento == 1 || Id_Evento == 2){//Se ha pulsado un bot?n
				ESTADO = pulsado;
				Boton = Id_Evento;
				// Meter los campos de la alarma
				Pulsacion_alarma.idEvento = 0;
				Pulsacion_alarma.timeStamp = temporizador_leer();
				aux = (Id_Evento & 0xFF000000);
				//1(0x1) porque es periodico y 100(0X000064) de periodo -> (0X800064): 24 bits y quedan los 8 de id 
				Pulsacion_alarma.auxData = (aux | 0x00800064); // 8 bits de mas peso id de evento, bit 9 si es periodica o no y 23 bits el tiempo de periodo
				gestor_alarmas_control_cola(Pulsacion_alarma); //prepara alarma
			}
		break;
		case pulsado:
			if(Id_Evento == 0){//LLega alarma
				switch(Boton){ //Eint 0
					case 1:
						EXTINT |= 0x2;
						if((EXTINT & 0x00000002)!= 2){//DONE
							eint1_clear_nueva_pulsacion();
							Pulsacion_alarma.idEvento = 0;
							Pulsacion_alarma.timeStamp = temporizador_leer();
							Pulsacion_alarma.auxData = 0x01000000;
							gestor_alarmas_control_cola(Pulsacion_alarma); //parar alarma
							ESTADO = no_pulsado;
						}
					break;
					case 2:
						EXTINT |= 0x4;
						if((EXTINT & 0x00000004)!= 4){
							eint2_clear_nueva_pulsacion();
							Pulsacion_alarma.idEvento = 0;
							Pulsacion_alarma.timeStamp = temporizador_leer();
							Pulsacion_alarma.auxData = 0x02000000;
							gestor_alarmas_control_cola(Pulsacion_alarma); //parar alarma
							ESTADO = no_pulsado;
						}
					break;
				}
			}
		break;
	}
}
