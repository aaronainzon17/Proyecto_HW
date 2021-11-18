#include "gestor_IO.h"
#include "gpio.h"

void gestor_IO_init(void){
	GPIO_iniciar();
	GPIO_marcar_entrada(14,14);
}


