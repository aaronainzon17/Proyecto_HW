/* guarda para evitar inclusiones m?ltiples (include guard) */
#ifndef GPIO_H
#define GPIO_H
#include <inttypes.h>
#include "eventos.h"

void GPIO_iniciar(void);
int GPIO_leer(int bit_inicial, int num_bits);
void GPIO_escribir(int bit_inicial, int num_bits, int valor);
void GPIO_marcar_entrada(int bit_inicial, int num_bits);
void GPIO_marcar_salida(int bit_inicial, int num_bits);


#endif // GPIO_H
