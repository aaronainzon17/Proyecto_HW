/* guarda para evitar inclusiones múltiples (include guard) */
#ifndef CELDA_H
#define CELDA_H

#include <inttypes.h>



/* Cada celda se codifica en 16 bits
 * bits [15,7]: los 9 bits más significativos representan el vector de candidatos,
 * si el bit 7 + valor - 1 está a 0, valor es candidato, 1 en caso contrario
 * bit 6: no empleado
 * bit 5: error
 * bit 4: pista
 * bits [3,0]: valor de la celda
 */

enum { BIT_CANDIDATOS = 7 };

typedef uint16_t CELDA;

/* *****************************************************************************
 * elimina el candidato del valor almacenado en la celda indicada */
__inline static void celda_eliminar_candidato(CELDA *celdaptr, uint8_t valor)
{
    *celdaptr = *celdaptr | (0x0001 << (BIT_CANDIDATOS + valor - 1));
}

/* *****************************************************************************
 * modifica el valor almacenado en la celda indicada */
__inline static void
celda_poner_valor(CELDA *celdaptr, uint8_t val)
{
    *celdaptr = (*celdaptr & 0xFFF0) | (val & 0x000F);
}

/* *****************************************************************************
 * extrae el valor almacenado en los 16 bits de una celda */
__inline static uint8_t
celda_leer_valor(CELDA celda)
{
    return (celda & 0x000F);
}

// Funcion para eliminar candidatos
__inline static void celda_establecer_todos_candidatos(CELDA *celdaptr)
{
    //Se ponen todos los candidatos a 0 por la logica negada son candidatos
   *celdaptr = *celdaptr & (0x007F);
}

__inline static void celda_introducir_candidatos(CELDA *celdaptr, uint8_t candidato)
{
		int num;
		num = 1 << (7 + candidato -1);
		num = ~num;

		*celdaptr = *celdaptr & num;
}

__inline static void celda_introducir_bit_error(CELDA *celdaptr)
{
    //Se ponen todos los candidatos a 0 por la logica negada son candidatos
	 *celdaptr = *celdaptr&=0xFFFFFFDF;
   *celdaptr = *celdaptr+=0x00000020;
}

__inline static void celda_quitar_bit_error(CELDA *celdaptr)
{
    //Se ponen todos los candidatos a 0 por la logica negada son candidatos
   *celdaptr = *celdaptr&=0xFFFFFFDF;
}

#endif // CELDA_H
