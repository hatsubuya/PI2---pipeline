
#include <stdio.h>
#include <stdlib.h>
#include "head.h"

void save_program_asm(instro *l, int tamanho, const char *nome_arquivo)
{

    FILE *f = fopen(nome_arquivo, "w");


    if (!f)
    {

        printf("\n erro ao criar arquivo");

        return;

    }



    for(int i = 0; i < tamanho; i++)
    {

        unsigned short instr = l->instc[i];

        //fprintf(f, "%3d: ", i);


        unsigned char opcode = (instr >> 12) & 0xF;

        unsigned char rs = (instr >> 9) & 0x7;

        unsigned char rt = (instr >> 6) & 0x7;

        unsigned char rd = (instr >> 3) & 0x7;

        unsigned char imm = instr & 0x3F;

        unsigned char funct = instr & 0x7;

        unsigned char addr = instr & 0xFF;

        signed char immx = (imm & 0x20) ? (imm | 0xC0) : imm;

        if (opcode == 0x0)
        {

            if (funct == 0x0)
            {
                fprintf(f, "add $r%d, $r%d, $r%d", rd, rs, rt);
            }


            else if (funct == 0x2)
            {
                fprintf(f, "sub $r%d, $r%d, $r%d", rd, rs, rt);
            }

            else if (funct == 0x4)
            {
                fprintf(f, "and $r%d, $r%d, $r%d", rd, rs, rt);
            }

            else if (funct == 0x5)
            {
                fprintf(f, "or $r%d, $r%d, $r%d", rd, rs, rt);
            }

            else
            {
                fprintf(f, "tipo r%d $r%d, $r%d, $r%d", funct, rd, rs, rt);
            }

        }

        else if (opcode == 0x4)
        {

            fprintf(f, "addi $r%d, $r%d, %d", rt, rs, immx);

        }

        else if (opcode == 0xB)
        {

            fprintf(f, "lw $r%d, %d($r%d)", rt, immx, rs);

        }

        else if (opcode == 0xF)
        {

            fprintf(f, "sw $r%d, %d($r%d)", rt, immx, rs);

        }

        else if (opcode == 0x8)
        {

            fprintf(f, "beq $r%d, $r%d, %d", rt, rs, immx * 2);

        }

        else if (opcode == 0x2)
        {

            fprintf(f, "j %d", addr * 2);

        }

        else
        {

            fprintf(f, "op_%d $r%d, $r%d, %d", opcode, rs, rt, immx);

        }

        fprintf(f, "\n");
    }

    fclose(f);

    printf("\n arquivo salvo com sucesso: %s\n", nome_arquivo);

}

void print_program(instro *l, int tamanho)
{

    printf("\n--- programa em assembly ---\n");

    for(int i = 0; i < tamanho; i++)
    {

        unsigned short instr = l->instc[i];

        printf("%3d: ", i);
        print_asm(instr);

    }

    printf("---------------------------\n");

}


//print da instru��o atual inicialmente para teste, mas adptado para ser usado em loop

void print_asm(unsigned short instr)
{
    unsigned char opcode = (instr >> 12) & 0xF;

    unsigned char rs = (instr >> 9) & 0x7;

    unsigned char rt = (instr >> 6) & 0x7;

    unsigned char rd = (instr >> 3) & 0x7;

    unsigned char imm = instr & 0x3F;

    unsigned char funct = instr & 0x7;

    unsigned char addr = instr & 0xFF;

    signed char immx = (imm & 0x20) ? (imm | 0xC0) : imm;



    // para o tipo R
    if (opcode == 0x0)
    {
        if (funct == 0x0)
        {

            printf("add $r%d, $r%d, $r%d", rd, rs, rt);

        }

        else if (funct == 0x2)
        {
            printf("sub $r%d, $r%d, $r%d", rd, rs, rt);
        }


        else if (funct == 0x4)
        {
            printf("and $r%d, $r%d, $r%d", rd, rs, rt);
        }


        else if (funct == 0x5)
        {
            printf("or $r%d, $r%d, $r%d", rd, rs, rt);
        }


        else
        {
            printf("tipo r %d $r%d, $r%d, $r%d", funct, rd, rs, rt);
        }

    }

    // para o tipo I
    else if (opcode == 0x4)
    {

        printf("addi $r%d, $r%d, %d", rt, rs, immx);

    }

    else if (opcode == 0xB)
    {

        printf("lw $r%d, %d($r%d)", rt, immx, rs);

    }

    else if (opcode == 0xF)
    {

        printf("sw $r%d, %d($r%d)", rt, immx, rs);

    }

    else if (opcode == 0x8)
    {

        printf("beq $r%d, $r%d, %d", rt, rs, immx * 2);

    }

    //para o tipo j
    else if (opcode == 0x2)
    {

        printf("j %d", addr * 2);

    }

    else
    {

        printf("op_%d $r%d, $r%d, %d", opcode, rs, rt, immx);

    }

    printf("\n");

}
