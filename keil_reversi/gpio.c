#include <LPC210x.H>                      
#include <inttypes.h>
#include "gpio.h"

void GPIO_iniciar(void){
		IODIR = 0xFFFFFFFF;	//Poner los pines modo output
		IOCLR = 0xFFFFFFFF;	//Limpiar los valores
}
/*
* bit_inicial  indica  el  primer  bit  a  leer. 
* num_bits  indica  cu?ntos  bits  queremos  leer.  
* La  funci?n  devuelve un entero con el valor de los bits indicados. 
*/
int GPIO_leer(int bit_inicial, int num_bits){
	int res = 0;
	int value;
	int peso = 1;
	int max = bit_inicial + num_bits;
	int i = bit_inicial;
	while (i < max){
		value = (IOPIN & (1<<i));
		if (value != 0){
				res += peso;
		}
		peso*=2;
		i++;
	}
	return res;
}

/*
* bit_inicial  indica  el  primer  bit  a  escribir. 
* num_bits  indica  cu?ntos  bits  queremos  escribir.  
* La  funci?n  devuelve un entero con el valor de los bits indicados. 
*/
void GPIO_escribir(int bit_inicial, int num_bits, int valor){
	int resto;
	int i = bit_inicial;
	int max = bit_inicial + num_bits;
	while (i < max){
		resto = valor % 2;
		valor /= 2;
		if (resto == 1){
			//Para poner a 1 se pone en IOSET
			IOSET = IOSET | 1<<i;
		}else{
			// Para poner a 0 un bit como en IOSET o no hace nada 
			// se limpia el valor en IOCLR
			IOCLR = IOCLR | 1<<i;
		}
		i++;
	}
}

void GPIO_marcar_entrada(int bit_inicial, int num_bits){
	uint32_t ini = 0xFFFFFFFF;
	uint32_t bits = (ini << bit_inicial); //Se introducen ceros por detras hasta el bit inicial 
	int i = 31; //Max bits de la GPIO
	int limit = bit_inicial+num_bits;
	while (i>=limit){
		//desde el bit final pongo ceros hasta el ultimo bit
		bits = bits & ~(1<<i);
		i--;
	}
	bits = ~bits; //lo invierto todo
	IODIR = IODIR & bits; //Pongo cero los nuevos y el resto se quedan como antes
}

void GPIO_marcar_salida(int bit_inicial, int num_bits){
	uint32_t ini = 0xFFFFFFFF;
	uint32_t bits = (ini << bit_inicial); //Se introducen ceros por detras hasta el bit inicial 
	int i = 31; //Max bits de la GPIO
	int limit = bit_inicial+num_bits;
	while (i>=limit){
		//desde el bit final pongo ceros hasta el ultimo bit
		bits = bits & ~(1<<i);
		i--;
	}
	IODIR =  IODIR | bits; //Pongo a uno los nuevos
}
