#include <stdio.h>
#include <stdlib.h>
#include "head.h"


//imprime o valore de todos os registradores com o auxilio das fuções de impressões binarias
void print_regs(signed char reg[8])
{
    printf("\n--- REGISTRADORES ---\n");

    for(int i = 0; i < 8; i++)
    {

        printf("R%d = ", i);

        imprime_bits((unsigned char)reg[i]);

        printf(" (%d)", reg[i]);

        printf("\n");

    }

    printf("---------------------\n");
}


// zera registradores, embora em teoria não seja necessario, e melhor se precaver com lixo eletronico
int iniat(signed char reg[8])
{
    reg[0]=0; reg[1]=0; reg[2]=2; reg[3]=3;
    reg[4]=0; reg[5]=0; reg[6]=0; reg[7]=0;

    return 0;
}


// le os registradores a e b
int read(signed char reg[8], signed char A, signed char B, signed char *outA, signed char *outB)
{

    //le regitradore e atualiza valores com base neles
    *outA = reg[A];

    *outB = reg[B];

    return 0;

}


// le e retorna o valor de destino com base no sinal reg_dst
int Rdest(int Sinal, signed char A, signed char B)
{

    return (Sinal) ? A : B;

}


//escreve no registradores destino, caso o sinal de escrita esteja ligado
int esc(signed char reg[8], int dest, signed char A,int RegWrite)
{

    if(dest != 0 && RegWrite==1)
    {

        reg[dest] = A;

    }

    return 0;

}
