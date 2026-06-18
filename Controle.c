#include <stdio.h>
#include <stdlib.h>
#include "head.h"


void print_sinais(int RegDst,int ULAOp,int ULAFonte,int beq,int jump,int EscMem,int EscReg,int MemParaReg)
{
    printf("\n--- SINAIS DE CONTROLE ---\n");

    printf("RegDst     = %d\n", RegDst);
    printf("ULAOp      = %d\n", ULAOp);
    printf("ULAFonte   = %d\n", ULAFonte);
    printf("beq        = %d\n", beq);
    printf("jump       = %d\n", jump);
    printf("EscMem     = %d\n", EscMem);
    printf("EscReg     = %d\n", EscReg);
    printf("MemParaReg = %d\n", MemParaReg);

    printf("--------------------------\n");
}

void Decodifica_controle(unsigned char opcode,int *RegDst,int *ULAOp,int *ULAFonte,int *beq,int *jump,int *EscMem,int *EscReg,int *MemParaReg)
{
    switch (opcode)
    {
        case 0x0:

            printf("Type R");

            *RegDst    = 1;

            *ULAOp     = 0;

            *ULAFonte  = 0;

            *beq       = 0;

            *jump       = 0;

            *EscMem    = 0;

            *EscReg    = 1;

            *MemParaReg = 0;

            break;

        case 0xB:

            printf("lw");

            *RegDst    = 0;

            *ULAOp     = 0;

            *ULAFonte  = 1;

            *beq       = 0;

            *jump       = 0;

            *EscMem    = 0;

            *EscReg    = 1;

            *MemParaReg = 1;

            break;

        case 0xF:

            printf("sw");

            *RegDst    = 0;

            *ULAOp     = 0;

            *ULAFonte  = 1;

            *beq       = 0;

            *jump       = 0;

            *EscMem    = 1;

            *EscReg    = 0;

            *MemParaReg = 0;

            break;

        case 0x8:

            printf("beq");

            *RegDst    = 0;

            *ULAOp     = 2;

            *ULAFonte  = 0;

            *beq       = 1;

            *jump       = 0;

            *EscMem    = 0;

            *EscReg    = 0;

            *MemParaReg = 0;

            break;

        case 0x4:

            printf("addi \n");

            *RegDst    = 0;

            *ULAOp     = 0;

            *ULAFonte  = 1;

            *beq       = 0;

            *jump       = 0;

            *EscMem    = 0;

            *EscReg    = 1;

            *MemParaReg = 0;

            break;

        case 0x2:

            printf("jump");

            *RegDst    = 0;

            *ULAOp     = 0;

            *ULAFonte  = 0;

            *beq       = 0;

            *jump       = 1;

            *EscMem    = 0;

            *EscReg    = 0;

            *MemParaReg = 0;

            break;

        default:

            printf("Opcode desconhecido: 0x%X\n", opcode);

            break;
    }
}
