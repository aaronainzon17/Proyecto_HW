#include <LPC210x.H>                      
#include <inttypes.h>
#include <stddef.h>
#include "eventos.h"
#include "gestor_IO.h"
#include "timer0.h"
#include "gestor_energia.h"
#include "eint_init.h"
#include <stddef.h>
#include "sudoku_2021.h"
#include "gpio.h"
#include "eint_init.h"
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

//Entrada anterior útil para recordar en la visualización la entrada que había antes para comprobar si ha cambiado o no
static volatile int entradas_anterior = 0x0000FC0;	
//Variables útiles para el cálculo de tiempos
static volatile int entradas_nuevo,tiempo_actualizar = 0,t1,t2,tot;
//Variables útiles para cálculo y propagacion de los errores
static volatile int antiguo_valor,fila_cancelar,columna_cancelar;
//Variables para los errores
static volatile int valor_error,hay_error=0;
//Buffers para las strings provenientes de la UART (funcion mostrar tablero)
char buffer[25];
char total[200];

static volatile CELDA
//Cuadricula para cuando reseteamos.
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
				//No quitar candidato
				if(hay_error==0){
					celda_eliminar_candidato(&cuadricula[fila][j],valor);
				}	
				//Si corrijes el error quitamos error
				if(hay_error==0 && j==columna){
					celda_quitar_bit_error(&cuadricula[fila][j]);
				}
				//SI hay error ponemos error en la causa
				if(hay_error==1 && valor_error == celda_leer_valor(cuadricula[fila][j])){
					celda_introducir_bit_error(&cuadricula[fila][j]);
				}
				//Si se a corregido el valor quita error de la causa
				if(hay_error==0 && error ==0x00000020 && antiguo_valor==celda_leer_valor(cuadricula[fila][j])){
					celda_quitar_bit_error(&cuadricula[fila][j]);
				}
				//Si introduces un nuevo error que lo causa una casilla diferente cambiar los bits de error de las causantes
				if(hay_error==1 && error ==0x00000020 && valor_error!=celda_leer_valor(cuadricula[fila][j]) && antiguo_valor==celda_leer_valor(cuadricula[fila][j]) && j!=columna){
						celda_quitar_bit_error(&cuadricula[fila][j]);
				}
		}

    /* recorrer columna descartando valor de listas candidatos */
    for (i=0;i<NUM_FILAS;i++){
				error=cuadricula[i][columna]&0x00000020;
				//No quitar candidato	
				if(hay_error==0){
					celda_eliminar_candidato(&cuadricula[i][columna],valor);
				}	
				//Si corriges el error quitamos error
				if(hay_error==0 && i==fila){
					celda_quitar_bit_error(&cuadricula[i][columna]);
				}
				//Si hay error ponemos error en la causa
				if(hay_error==1 && valor_error == celda_leer_valor(cuadricula[i][columna])){
					celda_introducir_bit_error(&cuadricula[i][columna]);
				}
				//Si se a corregido el valor quita error de la causa
				if(hay_error==0 && error ==0x00000020 && antiguo_valor==celda_leer_valor(cuadricula[i][columna])){
					celda_quitar_bit_error(&cuadricula[i][columna]);
				}
				if(hay_error==1 && error ==0x00000020 && valor_error!=celda_leer_valor(cuadricula[i][columna]) && antiguo_valor==celda_leer_valor(cuadricula[i][columna]) && i!=fila){
						celda_quitar_bit_error(&cuadricula[i][columna]);
				}
		}

    //Se determinan las fronteras región 
    init_i = init_region[fila];
    init_j = init_region[columna];
    end_i = init_i + 3;
    end_j = init_j + 3;

    // Se recorre la region descartando valores de listas candidatos
    for (i=init_i; i<end_i; i++) {
      for(j=init_j; j<end_j; j++) {
					error=cuadricula[i][j]&0x00000020;
					//No quitar candidato
					if(hay_error==0){
						celda_eliminar_candidato(&cuadricula[i][j],valor);
					}	
					//Si se corrige el error se quita el error
					if(hay_error==0 && i==fila && j==columna){
						celda_quitar_bit_error(&cuadricula[i][j]);
					}
					//Si hay error se pone error el la celda afectada
					if(hay_error==1 && valor_error == celda_leer_valor(cuadricula[i][j])){
						celda_introducir_bit_error(&cuadricula[i][j]);
					}
					//Si se ha corregido el valor quita error de la causa
					if(hay_error==0 && error ==0x00000020 && antiguo_valor==celda_leer_valor(cuadricula[i][j])){
						celda_quitar_bit_error(&cuadricula[i][j]);
					}
					if(hay_error==1 && error ==0x00000020 && valor_error!=celda_leer_valor(cuadricula[i][j]) && antiguo_valor==celda_leer_valor(cuadricula[i][j]) && j!=columna && i!=fila){
						celda_quitar_bit_error(&cuadricula[i][j]);
					}
			}
    }
}

//Funcion auxiliar que calcula cuantos candidatos tiene una celda (NO SE USA)
int sudoku_numero_candidatos(int fila, int columna){
	int candidatos;		
	//Se desplaza la cuadricula para dejar los candiatos al principio
	candidatos = ((cuadricula_C_C[fila][columna] >> 7));	
	//Se calculan los candidatos con una AND de los primeros 9 bits
	candidatos = candidatos & 0x1FF;
	return candidatos;
}

/*
* sudoku_introducir_candidatos se encarga de actualizar los candidatos 
* de las celdas tras introducir un nuevo valor en la misma
*/
void sudoku_introducir_candidatos(int valor,int fila,int columna){
		uint8_t j, i , init_i, init_j, end_i, end_j;
		const uint8_t init_region[9] = {0, 0, 0, 3, 3, 3, 6, 6, 6};
   

    //Se recorre la fila actualizando los candidatos
    for (j=0;j<9;j++){
			celda_introducir_candidatos(&cuadricula_C_C[fila][j],valor);
		}

    //Se recorre la columna actualizando los candidatos
    for (i=0;i<9;i++){
				celda_introducir_candidatos(&cuadricula_C_C[i][columna],valor);
		}

    // Se determinan las fronteras de la región
    init_i = init_region[fila];
    init_j = init_region[columna];
    end_i = init_i + 3;
    end_j = init_j + 3;

    //Se recorre la region actualizando los candidatos
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
int candidatos_actualizar_c(CELDA cuadricula[NUM_FILAS][NUM_COLUMNAS])
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

/*
* cuadricula_candidatos_verificar al acabar el sudoku, comprueba 
* que el resultado coincide con el tablero solucion
*/
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

/*Funcion que transforma un entero de una cifra (numero) en un char [] (letra)*/
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

/*Funcion que transforma un entero de más de una cifra (numero) en un char [] (letra)*/
void itoa_varios(int numero,char letra[]){
	if(numero==0){letra[0]='0';letra[1]='\0';}
	else{
		int resto = numero;
		int num = numero;
		int len = 0;
		//Se calcula el numero de digitos del numero
		while (num > 0){
			num = num/10;
			len++;
		}
		num = numero;
		letra[len] = '\0';
		len--;
		//Se almacena el numero de menor a mayor peso
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
/*
* get_candidatos se encarga de escribir por la UART los
* candidatos la celda que se pasa por parametro. Recibe por parámetros la fila y columna de la celda a tratar
*/
void get_candidatos(int fila, int columna,char can[]){
		int candidatos;
		//Se desplaza la cuadricula para dejar los candiatos al principio
		candidatos = ((cuadricula_C_C[fila][columna] >> 7));					
		//Se calculan los candidatos
		candidatos = candidatos & 0x1FF;
		//Se escriben los candidatos
		if((candidatos & 0x001) == 0){
			escritura_comenzar("1");
		}
		if((candidatos & 0x002) == 0){
			escritura_comenzar("2");
		}
		if((candidatos & 0x004) == 0){
			escritura_comenzar("3");
		}
		if((candidatos & 0x008) == 0){
			escritura_comenzar("4");
		}
		if((candidatos & 0x010) == 0){
			escritura_comenzar("5");
		}
		if((candidatos & 0x020) == 0){
			escritura_comenzar("6");
		}
		if((candidatos & 0x040) == 0){
			escritura_comenzar("7");
		}
		if((candidatos & 0x080) == 0){
			escritura_comenzar("8");
		}
		if((candidatos & 0x100) == 0){
			escritura_comenzar("9");
		}
		escritura_comenzar("\n");
}
//Función que escribe en la UART los candidatos de cada celda
void mostrar_candidatos(void){
	int fila, columna;
	char can[]="";
	char fila2 []= "";
	char columna2 [] = "";
	escritura_comenzar("Candidatos:\n");
	for(fila=0;fila<9;fila++){
		for(columna=0;columna<9;columna++){\
			//Se escribe fila y columna
			itoa(fila,fila2);
			itoa(columna,columna2);
			escritura_comenzar(fila2);
			escritura_comenzar(columna2);
			escritura_comenzar(":");
			//Se escriben los candidaros de la celda[fila][columna]
			get_candidatos(fila,columna,can);
		}
	}
		
}
/*
* sudoku_reset se ejecuta cuando llega al planificador la orden de reset la cual puede llegar 
* cuando se introduce #RST! por la UART o cuando finaliza la partida de forma exitosa
*/
void sudoku_reset (void){
	int j,k,minutos,segundos;
	char minutos_letra []= "";
	char segundos_letra []= "";
	char total_actualizar_letra []= "";
	//Se reincia el tablero
	for(j=0;j<9;j++){
		for(k=0;k<16;k++){
				cuadricula_C_C[j][k] = cuadricula[j][k];
		}
	}
	//Actualizamos los candidatos de nuevo
	candidatos_actualizar_c(cuadricula_C_C);
	//Reseteamos variable globales
	hay_error=0;
	tot=0;
	//convertimos los enteros a char
	minutos = RTC_leer_minutos();
	segundos= RTC_leer_segundos();
	itoa_varios(minutos,minutos_letra);
	itoa_varios(segundos,segundos_letra);
	itoa_varios(tiempo_actualizar,total_actualizar_letra);
	tiempo_actualizar = 0;
	//Mostramos por la UART las estadisticas de la partida
	escritura_comenzar("Se han jugado ");
	escritura_comenzar(minutos_letra);
	escritura_comenzar(" minutos y ");
	escritura_comenzar(segundos_letra);
	escritura_comenzar(" segundos.\n");
	escritura_comenzar("Tiempo de Actualizar:");
	escritura_comenzar(total_actualizar_letra);
	escritura_comenzar("\n");
	escritura_comenzar("Nueva Partida\n----------------------------\n");
	//Dejamos al jugador iniciar una nueva partida
}


/*
* sudoku_jugada_principal almacena una jugada en el sudoku. Recibe como parámetros la fila, la columna y el nuevo valor.
*/
void sudoku_jugada_principal (int fila, int columna, int nuevo_valor){
		int guarda,fail;
		// La casilla introducida no es pista o se ha introducido fila = 0, columna = 0, valor = 0 (reset)
		if((((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) != 0x00000001) || (fila == 0 && columna == 0 && nuevo_valor ==0)){	
			if(fila == 0 && columna == 0 && nuevo_valor ==0){
				//Reiniciamos el juego
				sudoku_reset();
			}else{	
				//Si no se reinicia la partida ni es una casilla de las que se encuentras rellenas como pista
				guarda = (cuadricula_C_C[fila][columna] >> 6);	
				//Se comprueba que el valor a introducir esta como candidato
				guarda = guarda >> nuevo_valor;	
				//Se comprueba error anterior 
				fail=cuadricula_C_C[fila][columna]&0x00000020;
				//Si no hay error y la celda ya tenia un valor y ese valor no generaba error
				if((hay_error==0) && (antiguo_valor  != 0) && (fail == 0x00000000)){
					//Se vuelve a añadir el valor anterior a los candidatos de la celda y las de su fila, columna y sector
					sudoku_introducir_candidatos(antiguo_valor,fila,columna);
				}
				// Si el valor es candidato se añade y se almacena que no hay error 
				if( (guarda & 0x00000001) == 0 ){	
					celda_quitar_bit_error(&cuadricula_C_C[fila][columna]);
					hay_error=0;
				}else{
					//En caso de no ser candidato se almacena la jugada y se devuelve error
					celda_introducir_bit_error(&cuadricula_C_C[fila][columna]);
					hay_error = 1;
					//Se almacena el valor que provoca el error
					valor_error = celda_leer_valor(cuadricula_C_C[fila][columna]);
				}
				//Si hay error y la celda ya tenia un valor y ese valor no generaba error
				if((hay_error==1) && (antiguo_valor  != 0) && (fail== 0x00000000)){
					//Se vuelve a añadir el valor anterior a los candidatos de la celda y las de su fila, columna y sector
					sudoku_introducir_candidatos(antiguo_valor,fila,columna);
				}
				//Se escribe almacena la jugada
				celda_poner_valor(&cuadricula_C_C[fila][columna],nuevo_valor);	
				valor_error = nuevo_valor;
				//Propagas el nuevo valor
				candidatos_propagar_c(cuadricula_C_C,fila,columna);
				//Se comprueba si se ha acabado la partida
				if(cuadricula_candidatos_verificar(cuadricula_C_C,solucion) == 1){
					cola_guardar_eventos(ID_FINAL_JUEGO,0);
				}
				sudoku_jugar();
				sudoku_validacion_1s();
			}
		}else{
			sudoku_restaurar_estado();
			sudoku_jugar();
		}
}

/*
* sudoku_mostrar_vista_previa muestra una vista previa de la jugada
* antes de que esta sea aceptada o no. Recibe como parámertros la fila, columna y nuevo valor introducidos por la UART
*/
void sudoku_mostrar_vista_previa(int buffer){
	int fila,val;
	int columna;
	int nuevo_valor;
	//Se calcula la fila y la columna
	fila = buffer/100;
	fila_cancelar=fila;
	columna = (buffer/10)%((buffer/100)*10);
	columna_cancelar=columna;
	nuevo_valor = buffer%((buffer/10)*10);
	val = cuadricula_C_C[fila][columna] & 0x0000000F;
	antiguo_valor = val;
	cuadricula_C_C[fila][columna]&= 0xFFFFFFF0;
	//Escribes el nuevo valor
	cuadricula_C_C[fila][columna] += nuevo_valor;	
	escritura_comenzar("Vista Previa de la jugada\n");
	mostrar_tablero();
	introducir_alarma_aceptar();//Introducimos la alarma del tiempo que nos queda para aceptar
	introducir_alarma_parpadeo_aceptar();//Introducimos la alarma del parpadeo durante el periodo en el que se puede aceptar
}

/*
* sudoku_aceptar_jugada acepta una jugada
*/
void sudoku_aceptar_jugada(void){
	//Se elimina la alarma de timeout jugada
	quitar_alarma_aceptar();
	//Se desactiva el parpadeo del bit 13
	quitar_alarma_parpadeo_aceptar();
	disable_isr();
	disable_isr_fiq();
	//Se encola un evento de fin de periodo de aceptar jugada
	cola_guardar_eventos(ID_FIN_ACEPTAR,0);
	enable_isr();
	enable_isr_fiq();
}

/*
* sudoku_restaurar_estado restaura el estado de la juagda anterior
*/
void sudoku_restaurar_estado(void){
	cuadricula_C_C[fila_cancelar][columna_cancelar]&= 0xFFFFFFF0;
	cuadricula_C_C[fila_cancelar][columna_cancelar] += antiguo_valor;	// Escribes el valor antiguo
}

/*
* sudoku_cancelar_jugada cancela la jugada introducida
* restaurando el valor que tenia la celda anteriormente
*/
void sudoku_cancelar_jugada(void){
	//Se elimina la alarma de timeout jugadav y el parpadeo
	quitar_alarma_parpadeo_aceptar();
	quitar_alarma_aceptar();
	//Se restaura el valor anterior de la celda
	sudoku_restaurar_estado();
	sudoku_jugar();
}

/*
* sudoku_jugada guarda la jugada (GPIO)
*/
void sudoku_jugada (void){
		int fila;
		int columna;
		int nuevo_valor;
		fila = gestor_IO_leer_de_gpio(16,4);
		columna = gestor_IO_leer_de_gpio(20,4);
		nuevo_valor = gestor_IO_leer_de_gpio(24,4);
		sudoku_jugada_principal (fila,columna,nuevo_valor);
}

/*
* sudoku_jugada_UART la jugada (GPIO). Recibe como parámertros la fila, columna y nuevo valor introducidos por la UART
*/
void sudoku_jugada_UART (int auxdata){
		int fila;
		int columna;
		int nuevo_valor;
		//Se calcula la fila, columna y valor
		fila = auxdata/100;
		columna = (auxdata/10)%((auxdata/100)*10);
		nuevo_valor = auxdata%((auxdata/10)*10);
		//Se almacena la jugada
		sudoku_jugada_principal (fila,columna,nuevo_valor);
}

/*
* sudoku_jugada_borrar elimina una jugada de una celda (GPIO)
*/
void sudoku_jugada_borrar(void){
	int fila;
	int columna;
	fila = gestor_IO_leer_de_gpio(16,4);
	columna = gestor_IO_leer_de_gpio(20,4);
	//Comprobamos que la casilla no es pista por lo que no se podría borrar
	if(((cuadricula_C_C[fila][columna] >> 4) & 0x00000001) != 0x00000001){	
		//Eliminamos el valor de la casilla
		cuadricula_C_C[fila][columna] = 0x00000000;							
		t1 =temporizador_leer();
		//Actualizamos los candidatos
		candidatos_actualizar_c(cuadricula_C_C);							
		t2 =temporizador_leer();
		tot = t2-t1;
		tiempo_actualizar+= tot;
		sudoku_validacion_1s();
	}
}

/*
* sudoku_programar_visualizacion visualizar candidatos (GPIO)
*/
void sudoku_programar_visualizacion(void){
	//Evento que se genera para mostrarla visualización una vez a habido cambios en la entrada
	struct EventInfo Most_Vis;	
	//Lees las entradas de la GPIO
	entradas_nuevo = GPIO_leer(16,12);		
	//Si la entrada ha cambiado	
	if (entradas_anterior != entradas_nuevo){	
		//Reseteas la alarma de power_down
		introducir_alarma_power();		
		//Generamos evento de visualización si ha cambiado la entrada para actualizar los candidatos y el valor		
		Most_Vis.idEvento = ID_mostrar_vis;					
		Most_Vis.auxData = entradas_nuevo;	
		//Antes de encolar habría que inhabilitar las interrupciones pero lo haremos en la 3 con softirq
		disable_isr();
		disable_isr_fiq();
		cola_guardar_eventos(Most_Vis.idEvento,Most_Vis.auxData);		
		enable_isr();
		enable_isr_fiq();
	}	//Sino no haces nada
}

/*
* mostrar_tablero se muestra el tablero con todas las 
* jugadas realizadas sobre el
*/
void mostrar_tablero(void){
	int k,s,val,col,fil;
	char linea[] ="+-+-+-+-+-+-+-+-+-+-+-+-+-+-\n";
	char letra[] = "";
	escritura_comenzar("\n\n");
	//Se iteran filas
	for(k=0;k<19;k++){
		if(k%2==0){
			escritura_comenzar(linea);
		}else{
			//Se iteran columnas
			for(s=0;s<19;s++){
				if(s==0||s==6||s==12||s==18){
					escritura_comenzar("|");
				}else if(s==2||s==4||s==8||s==10||s==14||s==16){
					escritura_comenzar("!");
				}else{
					fil=k/2;
					col=s/2;
					//Si hay error y es pista
					if ((((cuadricula_C_C[fil][col] >> 5) & 0x00000001) == 0x00000001)&&(((cuadricula_C_C[fil][col] >> 4) & 0x00000001) == 0x00000001)){
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						//Se escribe una X en pista
						escritura_comenzar("X");
						escritura_comenzar(letra);
					}else if(((cuadricula_C_C[fil][col] >> 4) & 0x00000001) == 0x00000001){
						//Si es pista
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						//Se escribe una P
						escritura_comenzar("P");
						escritura_comenzar(letra);
					}else if (((cuadricula_C_C[fil][col] >> 5) & 0x00000001) == 0x00000001){
						//Si solo es error 
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						//Se escribe una E
						escritura_comenzar("E");
						escritura_comenzar(letra);
					}else if((cuadricula_C_C[fil][col] & 0x0000000F) != 0x00000000){
						//Si hay valor 
						val = cuadricula_C_C[fil][col] & 0x0000000F;
						itoa(val,letra);
						//Se escribe el valor 
						escritura_comenzar(letra);
						escritura_comenzar(" ");
					}else{
					//Sino espacio
						escritura_comenzar("  ");
					}
				}
			}
			escritura_comenzar("\n");
		}
	}
}


/*
* sudoku gestor de eventos de sudoku. Recibe el id del evento tratado
*/
void sudoku (int Evento){
    switch(Evento){
					case ID_Alarma:	
						//Alarma de pulsacion para comprobar si el boton sigue pulsado 
						Gestor_Pulsacion_Control(Evento);
					break;
					case ID_EINT1:	
						//Alarma de pulsacion del boton 1					
						sudoku_aceptar_jugada();
					break;
					case ID_EINT2:	
						//Alarma de pulsacion del boton 2
						sudoku_cancelar_jugada();
					break;
					case ID_Alarma_visualizacion:	
						//Alarma de visualizacion
						sudoku_programar_visualizacion();
					break;
					case ID_bit_val:	
						//Poner el bit de validación a 1 durante 1 segundo; Jugada exitosa; Practica2
						sudoku_validacion_1s();
					break;
					case ID_fin_val:	
						//Acaba la alarma de 1s y ponemos validar a 0 otra vez; Fin bit; Practica2
						gestor_IO_escribir_bit_validar();
					break;
		}		
}

/*
* sudoku_mostrar_visualizacion muestra los candidatos (GPIO). 
*/
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

/*
* sudoku_iniciar_tablero establece los candidatos del tablero incial
*/
void sudoku_iniciar_tablero(void){
	//Se inicia el temporizador
	t1=temporizador_leer();
	//Se inicializan los candidatos
	candidatos_actualizar_c(cuadricula_C_C);
	//Se mide el tiempo
	t2 =temporizador_leer();
	tot = t2-t1;
	//Tiempo total de actualizar
	tiempo_actualizar += tot;
}

/*
* sudoku_inicio se muestra la leyenda y el mensaje de incio
*/
void sudoku_inicio(void){
	escritura_comenzar("Leyenda\n#RST!: Parar juego.\n#NEW!: Nueva partida.\n#fcvs!: f(fila), c(columna), v(nuevo valor)\n");
	escritura_comenzar("Nueva Partida\n----------------------------\n");
	escritura_comenzar("Comando:");
}

/*
* sudoku_jugar se muestran por la uart el tablero y los candidatos 
*/
void sudoku_jugar(void){
	mostrar_tablero();
	mostrar_candidatos();
	escritura_comenzar("Comando:");
}

/*
* sudoku_nueva_partida reincia la partida
*/
void sudoku_nueva_partida(void){
	t1=temporizador_leer();
	candidatos_actualizar_c(cuadricula_C_C);
	t2 =temporizador_leer();
	tot = t2-t1;
}
