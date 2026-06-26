#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "head.h"

extern WINDOW *saida_pad;
#define saida_pad saida_pad

void print_regs(signed char reg[8])
{

    extern int pad_linha;
    pad_linha = 0;
    werase(saida_pad);

    escrever_no_pad("--- BANCO DE REGISTRADORES ---");
    for (int i = 0; i < 8; i++)
    {
        escrever_no_pad("R%d = %d", i, (int)reg[i]);
    }
    escrever_no_pad("-----------------------------");
}

int iniat(signed char reg[8])
{
    for (int i = 0; i < 8; i++) reg[i] = 0;
    return 0;
}

int ler_regs(signed char reg[8], signed char A, signed char B,
             signed char *outA, signed char *outB)
{
    *outA = reg[(unsigned char)A];
    *outB = reg[(unsigned char)B];
    return 0;
}

int Rdest(int Sinal, signed char A, signed char B)
{
    return Sinal ? A : B;
}

int esc(signed char reg[8], int dest, signed char A, int RegWrite)
{
    if (dest != 0 && RegWrite == 1)
        reg[dest] = A;
    return 0;
}
