#include <stdio.h>
#include <string.h>
#include "head2.h"

void push_pipeline(Snapshot *pilha, int *sp,signed char *reg, unsigned char PC,int n_ciclo, int n_instr,RegIF_ID *if_id, RegID_EX *id_ex,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,instro *mem, instrucoes contaInstrucoes)
{
    if (*sp >= 499)
    {

        printf("  [BACK] Pilha cheia (500 entradas). Estado mais antigo perdido.\n");

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

    for (int i = 0; i < 8;   i++)
    {
        s->reg[i]= reg[i];
    }

    for (int i = 0; i < 256; i++)
    {
        s->memoria[i]= mem->instc[i];
    }

    s->PC= PC;

    s->n_ciclo= n_ciclo;

    s->n_instr= n_instr;

    s->if_id= *if_id;

    s->id_ex= *id_ex;

    s->ex_mem= *ex_mem;

    s->mem_wb= *mem_wb;

    s->contaInstrucoes = contaInstrucoes;

}

void pop_pipeline(Snapshot *pilha, int *sp,signed char *reg, unsigned char *PC,int *n_ciclo, int *n_instr,RegIF_ID *if_id, RegID_EX *id_ex,RegEX_MEM *ex_mem, RegMEM_WB *mem_wb,instro *mem, instrucoes *contaInstrucoes)
{
    if (*sp < 0)
    {

        printf("  [BACK] Nada para desfazer � pilha vazia.\n");

        return;

    }

    unsigned char pc_antes= *PC;

    int ciclo_antes = *n_ciclo;

    Snapshot *s = &pilha[*sp];

    (*sp)--;

    for (int i = 0; i < 8;   i++)
    {

        reg[i]= s->reg[i];

    }

    for (int i = 0; i < 256; i++)
    {

        mem->instc[i]  = s->memoria[i];

    }


    *PC= s->PC;

    *n_ciclo= s->n_ciclo;

    *n_instr = s->n_instr;

    *if_id = s->if_id;

    *id_ex = s->id_ex;

    *ex_mem = s->ex_mem;

    *mem_wb = s->mem_wb;

    *contaInstrucoes = s->contaInstrucoes;


    printf("  [BACK] Ciclo %d | PC=%d  ->  Ciclo %d | PC=%d\n",ciclo_antes, pc_antes, *n_ciclo, *PC);
}
