#include <LPC210x.H>                      
#include <inttypes.h>
#include <stddef.h>
#include "eventos.h"
#include "gestor_IO.h"
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
#include "tableros.h"
#include "funciones_escritura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SWI_functions.h"
#include "gestor_RTC.h"



extern int candidatos_actualizar_arm_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);
extern int candidatos_actualizar_arm_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS]);

static volatile int entradas_anterior = 0x0000FC0;	//Entrada anterior útil para recordar en la visualización la entrada que había antes para comprobar si ha cambiado o no
static volatile int entradas_nuevo,tiempo_actualizar = 0,t1,t2,tot;//Variables útiles para el cálculo de tiempos
static volatile int antiguo_valor,fila_cancelar,columna_cancelar;//Variables útiles para cálculo y propagacion de los errores
static volatile int valor_error,hay_error=0;//VAriables para los errores
char buffer[25];//Buffers para las strings provenientes de la UART (funcion mostrar tablero)
char total[200];

static volatile CELDA//Cuadricula para cuando reseteamos.
cuadricula[NUM_FILAS][NUM_COLUMNAS] =
{
0x0015, 0x0000, 0x0000, 0x0013, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0000, 0x0000, 0x0019, 0x0000, 0x0000, 0x0000, 0x0015, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0019, 0x0016, 0x0017, 0x0000, 0x0015, 0x0000, 0x0013, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0018, 0x0000, 0x0019, 0x0000, 0x0000, 0x0016, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0015, 0x0018, 0x0016, 0x0011, 0x0014, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0014, 0x0012, 0x0000, 0x0013, 0x0000, 0x0017, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0017, 0x0000, 0x0015, 0x0000, 0x0019, 0x0012, 0x0016, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0016, 0x0000, 0x0000, 0x0000, 0x0018, 0x0000, 0x0000, 0x0000, 0x0000, 0, 0, 0, 0, 0, 0, 0,
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0012, 0x0000, 0x0000, 0x0011, 0, 0, 0, 0, 0, 0, 0
};


/* *****************************************************************************
 * propaga el valor de una determinada celda en C
 * para actualizar las listas de candidatos
 * de las celdas en su su fila, columna y región y propaga los errores mostrando las celdas que causan el error*/
/* Recibe como parametro la cuadricula, y la fila y columna de
 * la celda a propagar; no devuelve nada
 */
void candidatos_propagar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
	uint8_t fila, uint8_t columna)
{
    uint8_t j, i , init_i, init_j, end_i, end_j, error;
    /* puede ayudar esta "look up table" a mejorar el rendimiento */
    const uint8_t init_region[NUM_FILAS] = {0, 0, 0, 3, 3, 3, 6, 6, 6};

    /* valor que se propaga */
    uint8_t valor = celda_leer_valor(cuadricula[fila][columna]);

    /* recorrer fila descartando valor de listas candidatos */
    for (j=0;j<NUM_FILAS;j++){
				error=cuadricula[fila][j]&0x00000020;
				if(hay_error==0){celda_eliminar_candidato(&cuadricula[fila][j],valor);}	//No quitar candidato
				if(hay_error==0 && j==columna){celda_quitar_bit_error(&cuadricula[fila][j]);}//Si corrijes el error quitamos error
				if(hay_error==1 && valor_error == celda_leer_valor(cuadricula[fila][j])){celda_introducir_bit_error(&cuadricula[fila][j]);}//SI hay error ponemos error en la causa
				if(hay_error==0 && error ==0x00000020 && antiguo_valor==celda_leer_valor(cuadricula[fila][j])){celda_quitar_bit_error(&cuadricula[fila][j]);}//Si se a corregido el valor quita error de la causa
				if(hay_error==1 && error ==0x00000020 && valor_error!=celda_leer_valor(cuadricula[fila][j]) && antiguo_valor==celda_leer_valor(cuadricula[fila][j]) && j!=columna){
						celda_quitar_bit_error(&cuadricula[fila][j]);}//Si introduces un nuevo error que lo causa una casilla diferente cambiar los bits de error de las causantes
		}

    /* recorrer columna descartando valor de listas candidatos */
    for (i=0;i<NUM_FILAS;i++){
				error=cuadricula[i][columna]&0x00000020;
				if(hay_error==0){celda_eliminar_candidato(&cuadricula[i][columna],valor);}	//No quitar candidato
				if(hay_error==0 && i==fila){celda_quitar_bit_error(&cuadricula[i][columna]);}//Si corrijes el error quitamos error
				if(hay_error==1 && valor_error == celda_leer_valor(cuadricula[i][columna])){celda_introducir_bit_error(&cuadricula[i][columna]);}//SI hay error ponemos error en la causa
				if(hay_error==0 && error ==0x00000020 && antiguo_valor==celda_leer_valor(cuadricula[i][columna])){celda_quitar_bit_error(&cuadricula[i][columna]);}//Si se a corregido el valor quita error de la causa
				if(hay_error==1 && error ==0x00000020 && valor_error!=celda_leer_valor(cuadricula[i][columna]) && antiguo_valor==celda_leer_valor(cuadricula[i][columna]) && i!=fila){
						celda_quitar_bit_error(&cuadricula[i][columna]);}
		}

    /* determinar fronteras región */
    init_i = init_region[fila];
    init_j = init_region[columna];
    end_i = init_i + 3;
    end_j = init_j + 3;

    /* recorrer region descartando valor de listas candidatos */
    for (i=init_i; i<end_i; i++) {
      for(j=init_j; j<end_j; j++) {
					error=cuadricula[i][j]&0x00000020;
					if(hay_error==0){celda_eliminar_candidato(&cuadricula[i][j],valor);}	//No quitar candidato
					if(hay_error==0 && i==fila && j==columna){celda_quitar_bit_error(&cuadricula[i][j]);}//Si corrijes el error quitamos error
					if(hay_error==1 && valor_error == celda_leer_valor(cuadricula[i][j])){celda_introducir_bit_error(&cuadricula[i][j]);}//SI hay error ponemos error en la causa
					if(hay_error==0 && error ==0x00000020 && antiguo_valor==celda_leer_valor(cuadricula[i][j])){celda_quitar_bit_error(&cuadricula[i][j]);}//Si se a corregido el valor quita error de la causa
					if(hay_error==1 && error ==0x00000020 && valor_error!=celda_leer_valor(cuadricula[i][j]) && antiguo_valor==celda_leer_valor(cuadricula[i][j]) && j!=columna && i!=fila){
						celda_quitar_bit_error(&cuadricula[i][j]);}
			}
    }
}

//Funcion auxiliar que calcula cuantos candidatos tiene una celda
int sudoku_numero_candidatos(int fila, int columna){
	int candidatos;		
	candidatos = ((cuadricula_C_C[fila][columna] >> 7));					// calculamos los candidatos
	candidatos = candidatos & 0x1FF;
	return candidatos;
}

//SI tras introducir un número valido, el usuario introduce un número erroneo en la misma celda
//el número anterior debe volver a aparecer como candidato, esta función vuelve a añadirlo como candidatos
//a las celdas correspondientes
void sudoku_introducir_candidatos(int valor,int fila,int columna){
		uint8_t j, i , init_i, init_j, end_i, end_j;
		const uint8_t init_region[9] = {0, 0, 0, 3, 3, 3, 6, 6, 6};
   

    /* recorrer fila introduciendo valor de listas candidatos */
    for (j=0;j<9;j++){
			celda_introducir_candidatos(&cuadricula_C_C[fila][j],valor);
		}

    /* recorrer columna introduciendo valor de listas candidatos */
    for (i=0;i<9;i++){
				celda_introducir_candidatos(&cuadricula_C_C[i][columna],valor);
		}

    /* determinar fronteras región */
    init_i = init_region[fila];
    init_j = init_region[columna];
    end_i = init_i + 3;
    end_j = init_j + 3;

    /* recorrer region introduciendo valor de listas candidatos */
    for (i=init_i; i<end_i; i++) {
      for(j=init_j; j<end_j; j++) {
				celda_introducir_candidatos(&cuadricula_C_C[i][j],valor);
	    }
    }
}

/* *****************************************************************************
 * calcula todas las listas de candidatos (9x9)
 * necesario tras borrar o cambiar un valor (listas corrompidas)
 * retorna el numero de celdas vacias */

/* Init del sudoku en codigo C invocando a propagar en C
 * Recibe la cuadricula como primer parametro
 * y devuelve en celdas_vacias el n�mero de celdas vacias
 */
static int candidatos_actualizar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{
    int celdas_vacias = 0;
    uint8_t i,j;

	//borrar todos los candidatos
    for(i = 0; i < NUM_FILAS; i++) {
        for(j = 0; j < NUM_FILAS; j++) {
            celda_establecer_todos_candidatos(&cuadricula[i][j]); 
        }
    }
	//recalcular candidatos de las celdas vacias calculando cuantas hay vacias
    for(i = 0; i < NUM_FILAS; ++i) {
        for(j = 0; j < NUM_FILAS; ++j) {
            if (celda_leer_valor(cuadricula[i][j]) == 0x0){
                celdas_vacias++;
            }else{
                //candidatos_propagar_arm(cuadricula,i,j);
                candidatos_propagar_c(cuadricula,i,j);
            }
        }
    }

	//retornar el numero de celdas vacias
	return celdas_vacias; // por favor eliminar una vez completada la función
}

/* Init del sudoku en codigo C invocando a propagar en arm
 * Recibe la cuadricula como primer parametro
 * y devuelve en celdas_vacias el n�mero de celdas vacias
 */
int candidatos_actualizar_c_arm(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
{	
	int celdas_vacias = 0;
    uint8_t i,j;

	//borrar todos los candidatos
    for(i = 0; i < NUM_FILAS; i++) {
        for(j = 0; j < NUM_FILAS; j++) {
            celda_establecer_todos_candidatos(&cuadricula[i][j]); 
        }
    }
	//recalcular candidatos de las celdas vacias calculando cuantas hay vacias
    for(i = 0; i < NUM_FILAS; ++i) {
        for(j = 0; j < NUM_FILAS; ++j) {
            if (celda_leer_valor(cuadricula[i][j]) == 0x0){
                celdas_vacias++;
            }else{
                candidatos_propagar_arm(cuadricula,i,j);
                //candidatos_propagar_c(cuadricula,i,j);
            }
        }
    }

	//retornar el numero de celdas vacias
	return celdas_vacias; // por favor eliminar una vez completada la función
	
	//return candidatos_actualizar_arm_c(cuadricula); // por favor eliminar una vez completada la función
}

static int cuadricula_candidatos_verificar(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS],
	 CELDA solucion[NUM_FILAS][NUM_COLUMNAS])
{
    int correcto = 1;
    uint8_t i;
    uint8_t j;

    for(i=0; i < NUM_FILAS && 1 == correcto; ++i) {
       for(j=0; j < NUM_FILAS && 1 == correcto; ++j) {
	   correcto = cuadricula[i][j] == solucion[i][j];
       }
    }
    return correcto;
}

/* ************************************************************************
 * programa principal del juego que recibe el tablero
 */
int
sudoku9x9(CELDA cuadricula_C_C[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_C_ARM[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_ARM_ARM[NUM_FILAS][NUM_COLUMNAS],
	CELDA cuadricula_ARM_C[NUM_FILAS][NUM_COLUMNAS],
	CELDA solucion[NUM_FILAS][NUM_COLUMNAS])
{
    int celdas_vacias[4];     //numero de celdas aun vacias
    int correcto = 0;
	  size_t i;
    /* calcula lista de candidatos, versi�n C */
    celdas_vacias[0] = candidatos_actualizar_c(cuadricula_C_C);

    //    /* Init C con propagar arm */
    celdas_vacias[1] = candidatos_actualizar_c_arm(cuadricula_C_ARM);

    //    /* Init arm con propagar arm */
    celdas_vacias[2] = candidatos_actualizar_arm_arm(cuadricula_ARM_ARM);

    //    /* Init arm con propagar c */
    celdas_vacias[3] = candidatos_actualizar_arm_c(cuadricula_ARM_C);
	
	  for (i=1; i < 4; ++i) {
			if (celdas_vacias[i] != celdas_vacias[0]) {
				return -1;
			}
		}

    /* verificar que la lista de candidatos C_C calculada es correcta */
    correcto = cuadricula_candidatos_verificar(cuadricula_C_C,solucion);
    correcto += cuadricula_candidatos_verificar(cuadricula_ARM_ARM,solucion);
    correcto += cuadricula_candidatos_verificar(cuadricula_C_ARM,solucion);
    correcto += cuadricula_candidatos_verificar(cuadricula_ARM_C,solucion);
    return correcto;
}
/*Función practica 2 que ponía el bit 13 a 1 durante un segundo cuando la jugada era valida*/
void sudoku_validacion_1s (void){
		gestor_IO_escribir_en_gpio(13,1,1);
		gestor_alarma_visualizacion_1s();
}

/*Funcion que transforma un entero de una cifra en un char []*/
void itoa(int numero,char letra[]){
	if(numero==1){letra[0] = '1';}
	else if(numero==2){letra[0] = '2';}
	else if(numero==3){letra[0] = '3';}
	else if(numero==4){letra[0] = '4';}
	else if(numero==5){letra[0] = '5';}
	else if(numero==6){letra[0] = '6';}
	else if(numero==7){letra[0] = '7';}
	else if(numero==8){letra[0] = '8';}
	else if(numero==9){letra[0] = '9';}
	else if(numero==0){letra[0] = '0';}
}

/*Funcion que transforma un entero de más de una cifra en un char []*/
void itoa_varios(int numero,char letra[]){
	if(numero==0){letra[0]='0';letra[1]='\0';}
	else{
		int resto = numero;
		int num = numero;
		int len = 0;
		while (num > 0){
			num = num/10;
			len++;
		}
		num = numero;
		letra[len] = '\0';
		len--;
		while(num > 0 && len >= 0){
			resto = num%10;
			num = num/10;
			if(resto==1){letra[len] = '1';}
			else if(resto==2){letra[len] = '2';}
			else if(resto==3){letra[len] = '3';}
			else if(resto==4){letra[len] = '4';}
			else if(resto==5){letra[len] = '5';}
			else if(resto==6){letra[len] = '6';}
			else if(resto==7){letra[len] = '7';}
			else if(resto==8){letra[len] = '8';}
			else if(resto==9){letra[len] = '9';}
			else if(resto==0){letra[len] = '0';}
		len--;
		}
	}
}
/*Función que escribe en la UART los candidatos de una celda*/
void get_candidatos(int fila, int columna,char can[]){
		int candidatos;
		
		candidatos = ((cuadricula_C_C[fila][columna] >> 7));					// calculamos los candidatos
		candidatos = candidatos & 0x1FF;
		if((candidatos & 0x001) == 0){
			write_string("1");
		}
		if((candidatos & 0x002) == 0){
			write_string("2");
		}
		if((candidatos & 0x004) == 0){
			write_string("3");
		}
		if((candidatos & 0x008) == 0){
			write_string("4");
		}
		if((candidatos & 0x010) == 0){
			write_string("5");
		}
		if((candidatos & 0x020) == 0){
			write_string("6");
		}
		if((candidatos & 0x040) == 0){
			write_string("7");
		}
		if((candidatos & 0x080) == 0){
			write_string("8");
		}
		if((candidatos & 0x100) == 0){
			write_string("9");
		}
		write_string("\n");
}
//Función que escribe en la UART los candidatos de cada celda
void mostrar_candidatos(void){
	int fila, columna;
	char can[]="";
	char fila2 []= "";
	char columna2 [] = "";
	write_string("Candidatos:\n");
	for(fila=0;fila<9;fila++){
		for(columna=0;columna<9;columna++){
			itoa(fila,fila2);
			itoa(columna,columna2);
			write_string(fila2);
			write_string(columna2);
			write_string(":");
			get_candidatos(fila,columna,can);
		}
	}
		
}
/*Función que se ejecuta cuando llega al planificador la orden de reset la cual puede llegar 
  cuando se introduce #RST! por la UART o cuando finaliza la partida de forma exitosa*/
void sudoku_reset (void){
	int j,k,minutos,segundos;
	char minutos_letra []= "";
	char segundos_letra []= "";
	char total_actualizar_letra []= "";
	//Reseteamos el tablero
	for(j=0;j<9;j++){
		for(k=0;k<16;k++){
				cuadricula_C_C[j][k] = cuadricula[j][k];	// Reiniciamos cada casilla
		}
	}
	//Actualizamos los candidatos de nuevo
	candidatos_actualizar_c(cuadricula_C_C);	// Actualizamos candidatos
	//Reseteamos variable globales
	hay_error=0;
	tot=0;
	tiempo_actualizar=0;
	//convertimos los enteros a char
	minutos =RTC_leer_minutos();
	segundos= RTC_leer_segundos();
	itoa_varios(minutos,minutos_letra);
	itoa_varios(segundos,segundos_letra);
	itoa_varios(tiempo_actualizar,total_actualizar_letra);
	tiempo_actualizar = 0;
	//Mostramos por la UART las estadisticas de la partida
	write_string("Se han jugado ");
	write_string(minutos_letra);
	write_string(" minutos y ");
	write_string(segundos_letra);
	write_string(" segundos.\n");
	write_string("Tiempo de Actualizar:");
	write_string(total_actualizar_letra);
	write_string("\n");
	write_string("Nueva Partida\n----------------------------\n");
	//Dejamos al jugador iniciar una nueva partida
}


/*Función que ejecuta una jugada del sudoku*/
void sudoku_jugada_principal (int fila, int columna, int nuevo_valor){
		int guarda,valor;
		// La casilla introducida no es pista o se ha introducido fila = 0, columna = 0, valor = 0
		if((((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) != 0x00000001) || (fila == 0 && columna == 0 && nuevo_valor ==0)){	
			if(fila == 0 && columna == 0 && nuevo_valor ==0){
				//Reiniciamos el juego
				sudoku_reset();
			}else{	// Se puede escribir en esa casilla porque no es pista
				guarda = (cuadricula_C_C[fila][columna] >> 6);	
				guarda = guarda >> nuevo_valor;	// Compruebas que el valor a introducir esta como candidato
				
				if( (guarda & 0x00000001) == 0 ){	// Si el bit esta a uno quitas bit de error
					celda_quitar_bit_error(&cuadricula_C_C[fila][columna]);
					hay_error=0;
				}else{
					celda_introducir_bit_error(&cuadricula_C_C[fila][columna]);// Si no pones bit de error
					hay_error=1;
					valor_error=celda_leer_valor(cuadricula_C_C[fila][columna]);
				}
				valor= cuadricula_C_C[fila][columna] & 0x0000000F;
				if((hay_error==1) && (valor  != 0)){
					sudoku_introducir_candidatos(antiguo_valor,fila,columna);
				}
				
				celda_poner_valor(&cuadricula_C_C[fila][columna],nuevo_valor);	// Escribes el nuevo valor
				if(cuadricula_candidatos_verificar(cuadricula_C_C,solucion) == 1){cola_guardar_eventos(ID_FINAL_JUEGO,0);}
				valor_error=nuevo_valor;
				candidatos_propagar_c(cuadricula_C_C,fila,columna);	// Propagas el nuevo valor
				sudoku_jugar();
				sudoku_validacion_1s();
			}
		}
}

void sudoku_mostrar_vista_previa(int buffer){
	int fila,val;
	int columna;
	int nuevo_valor;
	fila = buffer/100;
	fila_cancelar=fila;
	columna = (buffer/10)%((buffer/100)*10);
	columna_cancelar=columna;
	nuevo_valor = buffer%((buffer/10)*10);
	val = cuadricula_C_C[fila][columna] & 0x0000000F;
	antiguo_valor = val;
	cuadricula_C_C[fila][columna]&= 0xFFFFFFF0;
	cuadricula_C_C[fila][columna] += nuevo_valor;	// Escribes el nuevo valor
	write_string("Vista Previa de la jugada\n");
	mostrar_tablero();
}

void sudoku_aceptar_jugada(void){
	quitar_alarma_aceptar();
	quitar_alarma_parpadeo_aceptar();
	disable_isr();
	cola_guardar_eventos(ID_FIN_ACEPTAR,0);
	enable_isr();
}

void sudoku_restaurar_estado(){
	cuadricula_C_C[fila_cancelar][columna_cancelar]&= 0xFFFFFFF0;
	cuadricula_C_C[fila_cancelar][columna_cancelar] += antiguo_valor;	// Escribes el valor antiguo
}

void sudoku_cancelar_jugada(void){
	quitar_alarma_parpadeo_aceptar();
	quitar_alarma_aceptar();
	sudoku_restaurar_estado();
	sudoku_jugar();
}

void sudoku_jugada (void){
//		struct EventInfo led_val;	// Evento que se genera cuando se ha introducido una entrada que corresponde a una pista y se activa el bit 13 de la GPIO
		int fila;
		int columna;
		int nuevo_valor;
		//int guarda;
		fila = gestor_IO_leer_de_gpio(16,4);
		columna = gestor_IO_leer_de_gpio(20,4);
		nuevo_valor = gestor_IO_leer_de_gpio(24,4);
		sudoku_jugada_principal (fila,columna,nuevo_valor);
}

void sudoku_jugada_UART (int auxdata){
//		struct EventInfo led_val;	// Evento que se genera cuando se ha introducido una entrada que corresponde a una pista y se activa el bit 13 de la GPIO
		int fila;
		int columna;
		int nuevo_valor;
		//int guarda;
		fila = auxdata/100;
		columna = (auxdata/10)%((auxdata/100)*10);
		nuevo_valor = auxdata%((auxdata/10)*10);
		sudoku_jugada_principal (fila,columna,nuevo_valor);
}



void sudoku_jugada_borrar(void){
//	struct EventInfo led_val2;	// Evento que se genera cuando se ha introducido una entrada que corresponde a una pista y se activa el bit 13 de la GPIO
	int fila;
	int columna;
	fila = gestor_IO_leer_de_gpio(16,4);
	columna = gestor_IO_leer_de_gpio(20,4);
	if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) != 0x00000001){	// Comprobamos que la casilla no es pista por lo que no se podría borrar
		cuadricula_C_C[fila][columna] = 0x00000000;							// Eliminamos el valor de la casilla
		t1 =temporizador_leer();
		candidatos_actualizar_c(cuadricula_C_C);							// Actualizamos los candidatos
		t2 =temporizador_leer();
		tot = t2-t1;
		tiempo_actualizar+= tot;
		sudoku_validacion_1s();
	}
}

void sudoku_programar_visualizacion(void){
	struct EventInfo Most_Vis;	// Evento que se genera para mostrarla visualización una vez a habido cambios en la entrada
	entradas_nuevo = GPIO_leer(16,12);			// Lees las entradas de la GPIO
	if (entradas_anterior != entradas_nuevo){	// Si la entrada ha cambiado
		introducir_alarma_power();				// Reseteas la alarma de power_down
		Most_Vis.idEvento = ID_mostrar_vis;					// Generamos evento de visualización si ha cambiado la entrada para actualizar los candidatos y el valor
		Most_Vis.auxData = entradas_nuevo;	
		//Antes de encolar habría que inhabilitar las interrupciones pero lo haremos en la 3 con softirq
		disable_isr();
		cola_guardar_eventos(Most_Vis.idEvento,Most_Vis.auxData);		
		enable_isr();
	}	//Sino no haces nada
}


void mostrar_tablero(void){
	int k,s,val,col,fil;
	char linea[] ="+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n";
	char letra[] = "";
	write_string("\n\n");
	
	for(k=0;k<19;k++){//fila
		if(k%2==0){
			write_string(linea);
		}else{
			for(s=0;s<19;s++){//columna
				if(s==0||s==6||s==12||s==18){
					write_string("|");
				}else if(s==2||s==4||s==8||s==10||s==14||s==16){
					write_string("!");
				}else{
					fil=k/2;
					col=s/2;
					if ((((cuadricula_C_C[fil][col] >> 5) & 0x00000001) == 0x00000001)&&(((cuadricula_C_C[fil][col] >> 4) & 0x00000001) == 0x00000001)){
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						write_string("X");
						write_string(letra);
					}else if(((cuadricula_C_C[fil][col] >> 4) & 0x00000001) == 0x00000001){
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						write_string("P");
						write_string(letra);
					}else if (((cuadricula_C_C[fil][col] >> 5) & 0x00000001) == 0x00000001){
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						write_string("E");
						write_string(letra);
					}else if((cuadricula_C_C[fil][col] & 0x0000000F) != 0x00000000){
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						write_string(letra);
						write_string(" ");
					}else{
						write_string("  ");
					}
				}
			}
			write_string("\n");
		}
	}
}



void sudoku (int Evento){
    switch(Evento){
					case ID_Alarma:	
						Gestor_Pulsacion_Control(Evento);
					break;
					case ID_EINT1:						
						sudoku_aceptar_jugada();
					break;
					case ID_EINT2:	
						sudoku_cancelar_jugada();
					break;
					case ID_Alarma_visualizacion:	
						sudoku_programar_visualizacion();
					break;
					case ID_bit_val:	
						sudoku_validacion_1s();
					break;
					case ID_fin_val:	
						gestor_IO_escribir_bit_validar();
					break;
		}		
}

void sudoku_mostrar_visualizacion(struct EventInfo Evento){
	int fila;
	int columna;
	int valor_celda;
	int candidatos;
	fila = gestor_IO_leer_de_gpio(16,4); 												// Se lee la fila de la GPIO
	columna = gestor_IO_leer_de_gpio(20,4);												// Se lee la fila de la GPIO
	if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) == 0x00000001){	// Si la casilla es una pista enciende el bit de validar hasta que se deselecciona
		gestor_IO_escribir_en_gpio(13,1,1); 												//Bit de validar permanente
	}else{
		gestor_IO_escribir_en_gpio(13,1,0);												// Sino quitamos el bit de validar permanente
	}
	valor_celda = celda_leer_valor(cuadricula_C_C[fila][columna]);			// Sacamos el valor de la celda
	GPIO_escribir(0,4,valor_celda);											// Escribimos el valor de la celda
	candidatos = ((cuadricula_C_C[fila][columna] >> 7));					// calculamos los candidatos
	candidatos = ~candidatos;
	candidatos = candidatos & 0x1FF;
	gestor_IO_escribir_en_gpio(4,9,candidatos);											// Escribimos los candidatos
	entradas_anterior = Evento.auxData;										// Actualizamos la entrada para la siguiente pasada
}

void sudoku_iniciar_tablero(void){
	t1=temporizador_leer();
	candidatos_actualizar_c(cuadricula_C_C);
	t2 =temporizador_leer();
	tot = t2-t1;
	tiempo_actualizar += tot;
}

void sudoku_inicio(void){
	write_string("Leyenda\n#RST!: Parar juego.\n#NEW!: Nueva partida.\n#fcvs!: f(fila), c(columna), v(nuevo valor)\n");
	write_string("Nueva Partida\n----------------------------\n");
	write_string("Comando:");
}

void sudoku_jugar(void){
	mostrar_tablero();
	mostrar_candidatos();
	write_string("Comando:");
}

void sudoku_nueva_partida(void){
	t1=temporizador_leer();
	candidatos_actualizar_c(cuadricula_C_C);
	t2 =temporizador_leer();
	tot = t2-t1;
}
