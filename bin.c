#include <stdio.h>
#include <stdlib.h>
#include "head.h"


//imprimir valores binarios de 8 bits
void imprime_bits(unsigned char valor)
{
    for (int i = 7; i >= 0; i--)
    {

        printf("%d", (valor >> i) & 1);

    }

    printf("\n");

}

//imprimir valores binarios de 16 bits
void print_bin(unsigned short x)
{

    for(int i=15;i>=0;i--)
    {

        printf("%d", (x>>i)&1);

    }

}
