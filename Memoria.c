#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Head.h"

// mostrar registratadores não visiveis ao programador pc, ab, alout, , memoria

int carregar_unificado(instro *mem, const char *nome_arquivo)
{

    FILE *arquivo = fopen(nome_arquivo, "r");

    if (!arquivo)
    {

        printf(" Erro ao abrir: %s\n", nome_arquivo);

        return -1;

    }


    for (int i = 0; i < 256; i++)
    {

        mem->instc[i] = 0;

    }

    mem->n = 0;

    char buffer[32];

    //para instruções
    int pos_inst = 0;

    //para dados
    int pos_dado = 128;

    //mudou de memoria
    int na_area_dados = 0;

    while (fgets(buffer, sizeof(buffer), arquivo))
    {

        buffer[strcspn(buffer, "\r\n")] = '\0';


        if (strlen(buffer) == 0)
        {

            continue;

        }



        if (strcmp(buffer, ".data") == 0)
        {
            na_area_dados = 1;

            printf("  [MEMORIA] Marcador .data encontrado — dados a partir de %d\n",pos_dado);

            continue;

        }

        if (!na_area_dados)
        {


            if (pos_inst >= 128)
            {

                printf("  [MEMORIA] Aviso: mais de 128 instrucoes — ignorando excesso\n");

                continue;

            }

            mem->instc[pos_inst] = (unsigned short)strtol(buffer, NULL, 2);

            pos_inst++;

            mem->n = pos_inst;

        }

        else
        {

            char *sep = strchr(buffer, ':');

            if (sep != NULL)
            {

                *sep = '\0';

                int endereco = atoi(buffer);

                char *valor_str = sep + 1;

                if (endereco < 128 || endereco > 255)
                {
                    printf("  [MEMORIA] Aviso: endereco de dado invalido %d — ignorando\n",endereco);

                    continue;
                }

                mem->instc[endereco] = (unsigned short)strtol(valor_str, NULL, 2);
                printf("  [MEMORIA] Dado em mem[%d] = %d\n",
                       endereco, (signed char)mem->instc[endereco]);
            }
            else
            {

                if (pos_dado > 255)
                {
                    printf("  [MEMORIA] Aviso: area de dados cheia — ignorando excesso\n");

                    continue;

                }


                mem->instc[pos_dado] = (unsigned short)strtol(buffer, NULL, 2);

                printf("  [MEMORIA] Dado em mem[%d] = %d\n",pos_dado, (signed char)mem->instc[pos_dado]);

                pos_dado++;

            }

        }

    }


    fclose(arquivo);

    printf("  [MEMORIA] Carregadas %d instrucoes.\n", mem->n);

    return 0;

}


unsigned short ler_unificada(instro *mem, unsigned char endereco)
{



    return mem->instc[endereco];

}


int Store(instro *mem, signed char endereco, signed char valor)
{

    unsigned char end = (unsigned char)endereco;

    /*

    if (end < 128)
    {
        printf("tentativa de escrita em area de instrucoes !!!(end=%d)\n",end);

        return -1;

    }
    */

    mem->instc[end] = (unsigned short)(unsigned char)valor;

    return 0;

}


void print_mem_unificada(instro *mem)
{

    printf("\n");
    printf("====================================================\n");
    printf("              MEMORIA DE INSTRUCOES                 \n");
    printf("====================================================\n");
    printf(" END │ BINARIO                         │ HEXADECIMAL\n");
    printf("----------------------------------------------------\n");



    for (int i = 0; i < 128; i++)
    {




            printf("  [%3d] ", i);

            for (int b = 15; b >= 0; b--)
            {

                printf("%d", (mem->instc[i] >> b) & 1);

            }


            printf(" (0x%04X)\n", mem->instc[i]);



    }

    printf("====================================================\n");


    printf("\n");
    printf("====================================================\n");
    printf("                 MEMORIA DE DADOS                   \n");
    printf("====================================================\n");
    printf(" END │ BINARIO          │ DECIMAL\n");
    printf("----------------------------------------------------\n");

    for (int i = 128; i < 256; i++)
    {


            unsigned char val8 = (unsigned char)mem->instc[i];

            printf("  [%3d] ", i);

            for (int b = 7; b >= 0; b--)
            {

                printf("%d", (val8 >> b) & 1);

                if (b == 4)
                {

                    printf(" ");

                }

            }


            printf(" (%d)\n", (signed char)val8);



    }

    printf("=====================================\n");

}


void save_mem_dat(instro *mem, const char *nome_arquivo)
{

    FILE *f = fopen(nome_arquivo, "w");

    if (!f)
    {

        printf("Erro ao criar arquivo %s\n", nome_arquivo);

        return;

    }

    for (int i = 128; i < 256; i++)
    {

        unsigned char val = (unsigned char)mem->instc[i];

        for (int b = 7; b >= 0; b--)
        {

            fprintf(f, "%d", (val >> b) & 1);

        }


        fprintf(f, "\n");

    }

    fclose(f);

    printf("Dados salvos em: %s\n", nome_arquivo);

}
