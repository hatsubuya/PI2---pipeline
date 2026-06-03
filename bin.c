
#include <stdio.h>
#include "Head.h"

//impressor de binario par 16 bits - não mudou nada
void print_bin(unsigned short x)
{
    for (int i = 15; i >= 0; i--)
    {

        printf("%d", (x >> i) & 1);

    }

}

//impressor de binario par 8 bits - não mudou nada
void print_bin8(unsigned char x)
{
    for (int i = 7; i >= 0; i--)
    {

        printf("%d", (x >> i) & 1);

    }

}
