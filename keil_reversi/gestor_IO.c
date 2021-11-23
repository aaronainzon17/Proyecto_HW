#include "gestor_IO.h"
#include "gpio.h"

void gestor_IO_init(void){	// Inicia la GPIO y marca los pines que van a ser entradas 
	GPIO_iniciar();
	GPIO_marcar_entrada(14,14);
}


