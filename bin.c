#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "head.h"

extern WINDOW *saida_pad;
#define saida_pad saida_pad

void imprime_bits(unsigned char valor) {
    char bin[9];

    for (int i = 7; i >= 0; i--) {
        bin[7 - i] = (valor & (1 << i)) ? '1' : '0';
    }
    bin[8] = '\0';


    escrever_no_pad("%s", bin);
}

void print_bin(unsigned short x)
{
    for (int i = 15; i >= 0; i--)
    {
        if (saida_pad)
            wprintw(saida_pad, "%d", (x >> i) & 1);
        else
            printf("%d", (x >> i) & 1);
    }
}
