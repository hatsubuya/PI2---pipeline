#include <stdio.h>
#include <stdlib.h>
#include "Head.h"

#include <stdio.h>
#include <stdlib.h>
#include "Head.h"

void Decodifica_controle(unsigned char opcode,unsigned char *RegDst,unsigned char *ULAOp,unsigned char *ULAFonte,unsigned char *beq,unsigned char *jump,unsigned char *EscMem,unsigned char *EscReg,unsigned char *MemParaReg)
{
    switch (opcode)
    {
        case 0x0:

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

            printf("Opcode desconhecido: 0x%X\n", opcode);

            break;
    }
}
