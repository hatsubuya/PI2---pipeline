
#include <stdio.h>
#include <stdlib.h>
#include "head.h"

void save_mem_dat(instro *dat, const char *nome_arquivo)
{
    FILE *f = fopen(nome_arquivo, "w");

    if (!f)
    {

        printf("\n erro ao criar arquivo .dat");

        return;

    }

    for(int i = 0; i < 256; i++)
    {

        unsigned char valor = (unsigned char) dat->instc[i];

        // imprime em binário (8 bits)
        for(int j = 7; j >= 0; j--)
        {

            fprintf(f, "%d", (valor >> j) & 1);

        }

        fprintf(f, "\n");

    }

    fclose(f);

    printf("\n memoria de dados salva em: %s\n", nome_arquivo);

}

void print_mem(instro *dat)
{
    printf("\n--- MEMORIA DE DADOS ---\n");

    for(int i = 0; i < 256; i++)
    {

        printf("M[%3d] = ", i);

        imprime_bits(dat->instc[i]);

        //testar primeiro bit para verificar se é negativo caso 1(fazer complemento de 2) - erro corrigido! era pontual mas crucial
        printf(" (%d)", (signed char) dat->instc[i]);

        printf("\n");

    }

    printf("------------------------\n");

}


int carregar_dados(instro *dat, const char *nome_arquivo)
{
    FILE *arquivo = fopen(nome_arquivo, "r");

    if (!arquivo)
    {

        return -1;

    }

    //8 bits mas os mesmo 2 chacteres extras
    char buffer[10];

    int i = 0;

    while (i < 256 && fgets(buffer, sizeof(buffer), arquivo))
    {

        dat->instc[i] = (unsigned short) strtol(buffer, NULL, 2);

        i++;

    }

    fclose(arquivo);

    return 0;
}

void init_mem_incremental(instro *dat)
{

    for(int i = 0; i < 256; i++)
    {

        dat->instc[i] = 0;

    }

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
