#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include "head.h"


extern WINDOW *saida_pad;
#define saida_pad saida_pad

/* ---- instrucoes ---- */

void print_mem_inst(instro *l, int tamanho)
{
    if (saida_pad)
    {
        escrever_no_pad("--- MEMORIA DE INSTRUCOES ---");
        for (int j = 0; j < tamanho; j++)
        {

            char buffer[100];

            sprintf(buffer, "Pos %3d: ", j);


            mvwprintw(saida_pad, pad_linha, 0, "%s", buffer);


            wmove(saida_pad, pad_linha, 12);
            print_bin(l->instc[j]);


            pad_linha++;
        }
        escrever_no_pad("-----------------------------");
    }
}

int carregar(instro *l, const char *nome_arquivo, int *i)
{
    FILE *arquivo = fopen(nome_arquivo, "r");
    if (!arquivo) return -1;

    char buffer[32]; // Aumentei um pouco por segurança
    *i = 0;

    while (*i < 256 && fgets(buffer, sizeof(buffer), arquivo))
    {
        // 1. Remove o \n e \r (quebra de linha) se existirem
        buffer[strcspn(buffer, "\r\n")] = 0;

        // 2. Verifica se a linha não está vazia após a limpeza
        if (strlen(buffer) > 0)
        {
            l->instc[*i] = (unsigned short)strtol(buffer, NULL, 2);
            (*i)++;
        }
        // Se a linha estiver vazia, o loop continua e NÃO incrementa o *i,
        // evitando os "zeros" indesejados entre instruções.
    }

    fclose(arquivo);
    return 0;
}

unsigned short ler(instro *l, unsigned char instruct)
{
    return l->instc[instruct];
}
