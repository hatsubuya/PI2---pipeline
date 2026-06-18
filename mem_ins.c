
#include <stdio.h>
#include <stdlib.h>
#include "head.h"


//imprime valores presentes na memoria de instruções
void print_mem_inst(instro *l, int tamanho)
{

    printf("\n--- memoria de Instrucoes ---\n");

    for (int j = 0; j < tamanho; j++)
    {

        printf("Pos %d: ", j);

        print_bin(l->instc[j]);

        printf("\n");

    }

    printf("------------------------\n");

}


//carrega programa e suas devidas instruções
int carregar(instro *l, const char *nome_arquivo,int *i)
{
    FILE *arquivo = fopen(nome_arquivo, "r");

    if (!arquivo)
    {

        return -1;

    }


    //são 16 mas tem 2 carateres invisiveis
    char buffer[18];

    *i = 0;

    while (*i < 256 && fgets(buffer, sizeof(buffer), arquivo))
    {
        //16 bits conferidos por debugg
        l->instc[*i] = (unsigned short) strtol(buffer, NULL, 2);

        (*i)++;

    }

    fclose(arquivo);

    return 0;

}

//le um instrução de um determinada poisção
unsigned short ler(instro *l, unsigned char instruct)
{
    //embora desse para ler por int...
    return l->instc[instruct];

}
