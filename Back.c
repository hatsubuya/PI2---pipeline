#include <stdio.h>
#include <stdlib.h>
#include "Head.h"


void push_multi(Snapshot *pilha,int *sp,signed char reg[8],int Sinais[16],int ULAop,int overflow, RegIF_ID *if_id, RegID_EX  *id_ex, RegEX_MEM *ex_mem, RegMEM_WB *mem_wb, unsigned char  PC,int estado,int n_ciclo,int n_instr, instro *mem, instrucoes contaInstrucoes)
{
    if (*sp >= 499)
    {

        printf("  [BACK] Pilha cheia (500 ciclos). O estado mais antigo sera perdido\n");

        for (int i = 0; i < 499; i++)
        {

            pilha[i] = pilha[i + 1];

        }


    }

    else
    {

        (*sp)++;

    }

    Snapshot *s = &pilha[*sp];

    for (int i = 0; i < 8; i++)
    {

        s->reg[i] = reg[i];

    }


    for (int i = 0; i < 16; i++)
    {

        s->Sinais[i] = Sinais[i];

    }

    for(int i = 0; i < 256; i++)
    {
        s->memoria[i] = mem->instc[i];
    }



    s->ULAop    = ULAop;

    s->overflow = overflow;

    s->ULASaida = ULA_saida;

    s->RDM      = RDM;

    s->regA     = regA;

    s->regB     = regB;

    s->RI       = RI;

    s->PC       = PC;

    s->estado   = estado;

    s->n_ciclo  = n_ciclo;

    s->n_instr  = n_instr;



    s->tipoR = contaInstrucoes.tipoR;
    s->addi = contaInstrucoes.addi;
    s->lw = contaInstrucoes.lw;
    s->sw = contaInstrucoes.sw;
    s->beq = contaInstrucoes.beq;
    s->jump = contaInstrucoes.jump;

}

void pop_multi(Snapshot *pilha,int *sp,signed char reg[8],int Sinais[16],int *ULAop,int *overflow, RegIF_ID *if_id, RegID_EX  *id_ex, RegEX_MEM *ex_mem, RegMEM_WB *mem_wb, unsigned char *PC,int *estado,int *n_ciclo,int *n_instr, instro *mem, instrucoes *contaInstrucoes)
{
    if (*sp < 0)
    {

        printf("Nada para desfazer - pilha vazia\n");

        return;

    }


    unsigned char pc_antes   = *PC;

    int est_antes  = *estado;

    int ciclo_antes = *n_ciclo;

    Snapshot *s = &pilha[*sp];

    (*sp)--;

    for (int i = 0; i < 8; i++)
    {

        reg[i] = s->reg[i];

    }


    for (int i = 0; i < 16; i++)
    {

        Sinais[i] = s->Sinais[i];

    }

    for(int i = 0; i < 256; i++)
    {
        mem->instc[i] = s->memoria[i];
    }


    *ULAop    = s->ULAop;

    *overflow = s->overflow;

   *ULA_saida = s->ULASaida;

    *RDM      = s->RDM;

    *regA     = s->regA;

    *regB     = s->regB;

    *RI       = s->RI;

    *PC       = s->PC;

    *estado   = s->estado;

    *n_ciclo  = s->n_ciclo;

    *n_instr  = s->n_instr;


    contaInstrucoes->tipoR = s->tipoR;
    contaInstrucoes->addi = s->addi;
    contaInstrucoes->lw = s->lw;
    contaInstrucoes->sw = s->sw;
    contaInstrucoes->beq = s->beq;
    contaInstrucoes->jump = s->jump;


    printf("  [BACK] Ciclo %d | PC=%d | Estado=%d  ->  Ciclo %d | PC=%d | Estado=%d\n",ciclo_antes, pc_antes, est_antes,*n_ciclo, *PC, *estado);
}
