#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "head.h"

extern WINDOW *saida_pad;
#define saida_pad saida_pad

void print_mem(instro *dat) {
    escrever_no_pad("--- MEMORIA DE DADOS ---");
    for (int i = 0; i < 256; i++) {
        escrever_no_pad("M[%3d] = %d", i, (signed char)dat->instc[i]);
    }
}

int carregar_dados(instro *dat, const char *nome_arquivo)
{
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) return -1;

    char buffer[10];
    int i = 0;
    while (i < 256 && fgets(buffer, sizeof(buffer), arquivo))
    {
        dat->instc[i] = (unsigned short)strtol(buffer, NULL, 2);
        i++;
    }
    fclose(arquivo);
    return 0;
}

void save_mem_dat(instro *dat, const char *nome_arquivo)
{
    FILE *f = fopen(nome_arquivo, "w");
    if (!f)
    {
        if (saida_pad) wprintw(saida_pad, "\nErro ao criar arquivo .dat\n");
        else           printf("\nErro ao criar arquivo .dat\n");
        return;
    }
    for (int i = 0; i < 256; i++)
    {
        unsigned char valor = (unsigned char)dat->instc[i];
        for (int j = 7; j >= 0; j--)
            fprintf(f, "%d", (valor >> j) & 1);
        fprintf(f, "\n");
    }
    fclose(f);
}

void init_mem_incremental(instro *dat)
{
    for (int i = 0; i < 256; i++) dat->instc[i] = 0;
}

unsigned short load(instro *mem, unsigned char endereco)
{
    return mem->instc[endereco];
}

int temp(signed char A, signed char *mem)
{
    *mem = A;
    return 0;
}

int Store(instro *dat, signed char B, signed char mem)
{
    dat->instc[(unsigned char)B] = mem;
    return 0;
}
