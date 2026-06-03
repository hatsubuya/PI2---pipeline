#include <stdio.h>
#include <stdlib.h>
#include "Head.h"


void print_asm(unsigned short instr)
{
    unsigned char opcode = (instr >> 12) & 0xF;

    unsigned char rs     = (instr >> 9)  & 0x7;

    unsigned char rt     = (instr >> 6)  & 0x7;

    unsigned char rd     = (instr >> 3)  & 0x7;

    unsigned char imm    = instr & 0x3F;

    unsigned char funct  = instr & 0x7;

    unsigned char addr   = instr & 0xFF;

    signed char   immx   = (imm & 0x20) ? (imm | 0xC0) : imm;


    if (opcode == 0x0)
    {

        if(funct == 0x0)
        {

            //printf("add $r%d, $r%d, $r%d", rd, rs, rt);

            printf("  add $r%d, $r%d, $r%d\t\t| R%d = R%d + R%d\n",rd, rs, rt, rd, rs, rt);

        }

        else if (funct == 0x2)
        {

             printf("  sub $r%d, $r%d, $r%d\t\t| R%d = R%d - R%d\n",rd, rs, rt, rd, rs, rt);

        }

        else if(funct == 0x4)
        {

             //printf("and $r%d, $r%d, $r%d", rd, rs, rt);

             printf("  and $r%d, $r%d, $r%d\t\t| R%d = R%d AND R%d\n",rd, rs, rt, rd, rs, rt);

        }

        else if(funct == 0x5)
        {

             printf("  or  $r%d, $r%d, $r%d\t\t| R%d = R%d OR R%d\n",rd, rs, rt, rd, rs, rt);

        }


        else
        {

            printf("tipo_r funct=%d $r%d, $r%d, $r%d", funct, rd, rs, rt);

        }

    }

    else if (opcode == 0x4)
    {

        printf("  addi $r%d, $r%d, %d\t\t| R%d = R%d + %d\n",rt, rs, immx, rt, rs, immx);

    }

    else if (opcode == 0xB)
    {

        printf("  lw   $r%d, %d($r%d)\t\t| R%d = Mem[R%d + %d]\n",rt, immx, rs, rt, rs, immx);

    }

    else if (opcode == 0xF)
    {

        printf("  sw   $r%d, %d($r%d)\t\t| Mem[R%d + %d] = R%d\n",rt, immx, rs, rs, immx, rt);

    }

    else if (opcode == 0x8)
    {

        printf("  beq  $r%d, $r%d, %d\t\t| se R%d == R%d entao PC = PC + 1 + %d\n",rs, rt, immx, rs, rt, immx);

    }

    else if (opcode == 0x2)
    {

       printf("  j    %d\t\t\t| PC = %d\n",addr, addr);

    }

    else
    {

        printf("  op_0x%X\t\t\t| instrucao desconhecida\n", opcode);

    }

    printf("\n");
}

//print na tela pode e usa algo mais informativo, ao inves do assembly "cru"

void print_program(instro *mem, int tamanho)
{
    printf("\n--- PROGRAMA EM ASSEMBLY ---\n");

    for (int i = 0; i < tamanho; i++)
    {



            printf("  %3d: ", i);

            print_asm(mem->instc[i]);



    }

    printf("----------------------------\n");

}


void save_program_asm(instro *mem, int tamanho, const char *nome_arquivo)
{

    FILE *f = fopen(nome_arquivo, "w");

    if (!f)
    {

        printf(" Erro ao criar arquivo %s\n", nome_arquivo);

        return;

    }

    for (int i = 0; i < tamanho; i++)
    {

        if (mem->instc[i] != 0 || i < mem->n)
        {

            unsigned short instr  = mem->instc[i];

            unsigned char  opcode = (instr >> 12) & 0xF;

            unsigned char  rs     = (instr >> 9)  & 0x7;

            unsigned char  rt     = (instr >> 6)  & 0x7;

            unsigned char  rd     = (instr >> 3)  & 0x7;

            unsigned char  imm    = instr & 0x3F;

            unsigned char  funct  = instr & 0x7;

            unsigned char  addr   = instr & 0xFF;

            signed char    immx   = (imm & 0x20) ? (imm | 0xC0) : imm;

            fprintf(f, "%3d: ", i);

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

                else if(funct == 0x5)
                {

                    fprintf(f, "or  $r%d, $r%d, $r%d", rd, rs, rt);

                }

                else
                {

                    fprintf(f, "tipo_r funct=%d $r%d, $r%d, $r%d", funct, rd, rs, rt);

                }

            }

            else if (opcode == 0x4)
            {

                fprintf(f, "addi $r%d, $r%d, %d",   rt, rs, immx);

            }

            else if (opcode == 0xB)
            {

                fprintf(f, "lw   $r%d, %d($r%d)",   rt, immx, rs);

            }

            else if (opcode == 0xF)
            {

                fprintf(f, "sw   $r%d, %d($r%d)",   rt, immx, rs);

            }

            else if (opcode == 0x8)
            {

                fprintf(f, "beq  $r%d, $r%d, %d",   rs, rt, immx);

            }

            else if (opcode == 0x2)
            {

                fprintf(f, "j %d",addr);

            }

            else if (instr  == 0x0)
            {

                fprintf(f, "--- (vazio)");

            }

            else
            {

                fprintf(f, "op_0x%X $r%d, $r%d, %d", opcode, rs, rt, immx);

            }

            fprintf(f, "\n");

        }

    }

    fclose(f);

    printf("  assembly salvo em: %s\n", nome_arquivo);

}
