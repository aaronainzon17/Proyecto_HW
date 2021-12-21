#include <LPC210X.H>
#include "gpio.h"
#include "eventos.h"
#include "cola.h"
#include "gestor_UART.h"
#include "funciones_escritura.h"
#include "SWI_functions.h"

static volatile unsigned int ESTADO = inicio;
static volatile int buffer_UART[10];
static volatile unsigned int len_buffer;

//Gestor de los comandos introducidos por la UART
void gestor_UART(uint32_t cadena){
	int buffer = U0RBR;//Leemos el contenido de U0RBR
	U0THR = buffer;
	switch(ESTADO){
		case inicio:
				if (cadena == '#'){//Comenzamos a escuchar
					len_buffer = 0;
					ESTADO=esperando_fin;
				}
		break;
		case esperando_fin:
			if(cadena == '!'){//Interpretamos fin de la cadena
				ESTADO=inicio;//Cambiamos el estado
				if(buffer_UART[0]=='R' && buffer_UART[1]=='S' && buffer_UART[2]=='T'){//Se ha escrito Reset
					escritura_comenzar("\n");
					disable_isr();
					disable_isr_fiq();
					cola_guardar_eventos(ID_RST,0);//Encolamos evento de tipo reset
					enable_isr();
					enable_isr_fiq();
				}else if(buffer_UART[0]=='N' && buffer_UART[1]=='E' && buffer_UART[2]=='W'){//Se a escrito NEW; Nueva partida
					escritura_comenzar("\n");
					disable_isr();
					disable_isr_fiq();
					cola_guardar_eventos(ID_NEW,0);//Encolamos evento de tipo NEW
					enable_isr();
					enable_isr_fiq();
				}else if(((buffer_UART[0]-0x30)+(buffer_UART[1]-0x30)+(buffer_UART[2]-0x30))% 0x8 == (buffer_UART[3]-0x30)){//Se ha escrito una nueva jugada
					escritura_comenzar("\n");
					disable_isr();
					disable_isr_fiq();
					//Si la jugada es valida se encola un evento de Nueva Jugada
					cola_guardar_eventos(ID_ESPERAR_CONFIRMACION,(buffer_UART[0]%0x30)*100+(buffer_UART[1]%0x30)*10+(buffer_UART[2]%0x30));
					enable_isr();
					enable_isr_fiq();
				}
			}else{
				if(len_buffer<4){//Si la cadena recibida es mayor de 4 volvemos a inicio puesto que no existen comandos tan largos
					buffer_UART[len_buffer] = cadena;//Asignamos al buffer
					len_buffer++;
				}else{
					ESTADO=inicio;//Si nos hemos pasado volvemos a inicio
				}
			}
		break;
	}
}
