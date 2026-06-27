#include <stdio.h>
#include <stdlib.h>
#include "head.h"
#include <ncurses.h>

extern WINDOW *saida_pad;
#define saida_pad saida_pad



void print_sinais(int RegDst, int ULAOp, int ULAFonte, int beq, int jump,
                  int EscMem, int EscReg, int MemParaReg)
{
    if (saida_pad)
    {
        escrever_no_pad("--- SINAIS DE CONTROLE ---");

        escrever_no_pad("RegDst=%d | ULAOp=%d | ULAFonte=%d", RegDst, ULAOp, ULAFonte);
        escrever_no_pad("beq=%d | jump=%d | EscMem=%d", beq, jump, EscMem);
        escrever_no_pad("EscReg=%d | MemParaReg=%d", EscReg, MemParaReg);
        escrever_no_pad("--------------------------");
    }
}

void Decodifica_controle(unsigned char opcode,int *RegDst,int *ULAOp,int *ULAFonte,int *beq,int *jump,int *EscMem,int *EscReg,int *MemParaReg,int funct)
{
    switch (opcode)
    {
        case 0x0:


            switch(funct)
            {
                case 0:


                *RegDst    = 1;

                *ULAOp     = 0;

                *ULAFonte  = 0;

                *beq       = 0;

                *jump       = 0;

                *EscMem    = 0;

                *EscReg    = 1;

                *MemParaReg = 0;


                break;

                case 2:


                *RegDst    = 1;

                *ULAOp     = 2;

                *ULAFonte  = 0;

                *beq       = 0;

                *jump       = 0;

                *EscMem    = 0;

                *EscReg    = 1;

                *MemParaReg = 0;


                break;
            }



            break;

        case 0xB:


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


            break;
    }
}
