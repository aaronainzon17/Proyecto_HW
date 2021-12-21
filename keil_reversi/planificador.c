#include <LPC210x.H>                       /* LPC210x definitions */
#include "timer0.h"
#include "Power_management.h"
#include "boton_eint0.h"
#include <stddef.h>
#include "sudoku_2021.h"
#include "gpio.h"
#include "boton_eint0.h"
#include "gestor_alarmas.h"
#include "cola.h"
#include "planificador.h"
#include "gestor_pulsacion.h"
#include "eventos.h"
#include "gestor_IO.h"
#include "tableros.h"
#include "SWI_functions.h"
#include "gestor_UART.h"
#include "funciones_escritura.h"
#include "serial_port.h"
#include "gestor_WD.h"
#include "planificador.h"


// Nota: wait es una espera activa. Se puede eliminar poniendo el procesador en modo iddle. Probad a hacerlo

extern int candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int candidatos_actualizar_arm_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

//static volatile struct EventInfo alarma_visualizacion;

extern int gestor_SC_in(void);
extern int gestor_SC_out(void);



// MAIN 
void planificador (void) {
	struct EventInfo Evento;
	//int t1,t2;				// Variables de medición de tiempo
	//int tot;
	int PD_Flag = 0;					// Flag para salir de power down
	int Reset = 0;
	int Parpadeo=0;
	uint32_t volatile buffer;
	int iddle_bit_flag = 0;
	//Iniciamos los componentes
	iniciar();
	WT_init(50);
	/*feed_watchdog();
	while(1){
		cola_guardar_eventos(0,0);
		cola_guardar_eventos(0,0);
		cola_leer_evevento_antiguo(&Evento);
		feed_watchdog();
	}*/
	while(1){	
		feed_watchdog();//Activamos el watchdog para que detecte bucles infinitos
		if(cola_nuevos_eventos()){//Si hay nuevos eventos en la cola
				cola_leer_evevento_antiguo(&Evento);//Desencolamos
				switch(Evento.idEvento){
					case ID_Alarma:	// Alarma pulsacion para comprobar si el boton pulsado lo sigue estando
						sudoku(ID_Alarma);
					break;						
					case ID_EINT1:	// Nueva pulsacion EINT1, aceptar jugada
						introducir_alarma_power();		// Se resetea la alarma de power down
						Gestor_Pulsacion_Control(Evento.idEvento);	//Pasa a pulsado y pone la alarma; Control pulsacion
						if(PD_Flag == 1){					// Si vienes de power down no haces nada
							PD_Flag = 0;						// SD se pone a 0 para indicar que no estamos en power down
						}else{
							if(Reset==0){						//Si vienes de reset muestras el tablero y permites jugar
								Reset=1;
								sudoku_jugar();
							}else{
								sudoku(ID_EINT1);}		//Aceptas la jugada
						}
					break;
					case ID_EINT2:																			// Nueva pulsacion EINT2
						introducir_alarma_power();													// Se resetea la alarma de power down
						Gestor_Pulsacion_Control(Evento.idEvento);//Cambias a pulsado y pones la alarma de comprobación de pulsación
						if(PD_Flag == 1){					// Si vienes de power down no haces nada
							PD_Flag = 0;						// SD se pone a 0 para indicar que no estamos en power down
						}else{
							if(Reset==0){						//Si vienes de reset muestras el tablero y permites jugar
								Reset=1;
								sudoku_jugar();
							}else{
								sudoku(ID_EINT2);}		//Cancelas la jugada
						}					
					break;
					case ID_Alarma_visualizacion:											// Alarma Visualizacion
						sudoku(ID_Alarma_visualizacion);
					break;
					case ID_mostrar_vis:																			// Mostrar visualizacion de la GPIO
							sudoku_mostrar_visualizacion(Evento);
					break;
					case ID_bit_val:											// Poner el bit de validación a 1 durante 1 segundo; Jugada exitosa; Practica2
						sudoku(ID_bit_val);
					break;
					case ID_fin_val:						//Acaba la alarma de 1s y ponemos validar a 0 otra vez; Fin bit; Practica2
						sudoku(ID_fin_val);
					break;
					case ID_power_down:							// Acaba la alarma y entra en powerdown		
						apagar_alarma_iddle();
						PD_Flag = 1;						// Actualizamos el flag de powerdown 
						PM_power_down(); 				//Entramos en powerdown
						introducir_alarma_iddle();	//Al salir ponemos la alarma del latido de iddle
					break;
					case ID_timer_0:	//Llega timer 0
						gestor_alarmas_control_alarma();//Se resta 1 a cada periodo de las alarmas activas
					break;
					case ID_iddle:	//Bit de iddle
						if(iddle_bit_flag == 0){
							gestor_IO_escribir_bit_powerdown();//Enciende el bit de latido de iddle
							iddle_bit_flag = 1;
						}else{
							gestor_IO_apagar_bit_powerdown();//Apaga el bit de latido de iddle
							iddle_bit_flag = 0;
						}
					break;
					case ID_UART://Llega una interrupcion por linea serie
						introducir_alarma_power();//Reseteamos la alarma de powerdown porque el usuario sigue interaccionando
						gestor_UART(Evento.auxData);//Llamamos al gestor de linea serie
					break;
					case ID_RST://Llega el evento de reset por linea serie
						Reset=0;
						sudoku_reset();//Ejecutamos el reset
					break;
					case ID_NEW://Llega el evento de nueva partida
						Reset=1;
						sudoku_nueva_partida();//Se actualizan los candidatos
						sudoku_jugar();//Mostrar tablero y candidatos
					break;
					case ID_Continuar://Llega evento de continuar escribiendo en la UART
						escritura_continuar();
					break;
					case ID_ESPERAR_CONFIRMACION://Llega evento para esperar el aceptar la jugada o el rechazarla
						buffer=Evento.auxData;//Guardamos el buffer que te viene de la UART
						sudoku_mostrar_vista_previa(buffer);//Mostramos la vista previa de la jugada a introducir
						introducir_alarma_aceptar();//Introducimos la alarma del tiempo que nos queda para aceptar
						introducir_alarma_parpadeo_aceptar();//Introducimos la alarma del parpadeo durante el periodo en el que se puede aceptar
					break;
					case ID_FIN_ACEPTAR	://Llega la alarma de fin del periodo aceptar por lo que se acepta la jugada automáticamente
						quitar_alarma_parpadeo_aceptar();
						sudoku_jugada_UART(buffer);//Se ejecuta la jugada
					break;
					case ID_FINAL_JUEGO	://Llega un evento de que a finalizado el juego
						Reset=0;
						sudoku_reset();//Se resetea la partida
					break;
					case ID_PARPADEO_ACEPTAR	://Llega el evento de tratar el bit de parpadeo durante el tiempo de aceptar
						if(Parpadeo==0){Parpadeo=1;gestor_IO_escribir_en_gpio(13,1,1);}else{Parpadeo=0;gestor_IO_escribir_en_gpio(13,1,0);}					
					break;
				}
		}else{
			PM_idle();	//Si no hay eventos en la cola el procesador va a modo iddle
		}
	}
}
