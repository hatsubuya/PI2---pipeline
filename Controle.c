#include <stdio.h>
#include <stdlib.h>
#include "Head.h"

void print_sinais(int Sinais[16])
{
    printf("Sinais:\n");

    printf("REG_DST=%d\n", Sinais[REG_DST]);

    printf("JUMP=%d\n", Sinais[JUMP]);

    printf("MEM_READ=%d\n", Sinais[MEM_READ]);

    printf("MEM_WRITE=%d\n", Sinais[MEM_WRITE]);

    printf("BRANCH=%d\n", Sinais[BRANCH]);

    printf("ALU_SRC=%d\n", Sinais[ALU_SRC]);

    printf("MEM_TO_REG=%d\n", Sinais[MEM_TO_REG]);

    printf("REG_WRITE=%d\n", Sinais[REG_WRITE]);

    printf("IorD=%d\n", Sinais[IorD]);

    printf("IR_ESC=%d\n", Sinais[IR_ESC]);

    printf("PC_ESC=%d\n", Sinais[PC_ESC]);

    printf("PC_FONTE0=%d\n", Sinais[PC_FONTE0]);

    printf("PC_FONTE1=%d\n", Sinais[PC_FONTE1]);

    printf("ULA_FONTE_A=%d\n", Sinais[ULA_FONTE_A]);

    printf("ULA_FONTE_B0=%d\n", Sinais[ULA_FONTE_B0]);

    printf("ULA_FONTE_B1=%d\n", Sinais[ULA_FONTE_B1]);

}




void Decodifica_estado(int estado, int Sinais[16])
{



    switch (estado)
    {


        case 0:

            Sinais[PC_ESC] = 1;

            Sinais[IorD] = 0;

            Sinais[IR_ESC] = 1;

            Sinais[ULA_FONTE_A] = 0;

            Sinais[ULA_FONTE_B0]= 1;

            Sinais[ULA_FONTE_B1]= 0;

            Sinais[PC_FONTE0] = 0;

            Sinais[PC_FONTE1] = 0;

            Sinais[REG_DST] = 1;

            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[REG_WRITE] = 0;

            print_sinais(Sinais);


            break;


        case 1:

            Sinais[ULA_FONTE_A] = 0;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[REG_DST]= 1;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[REG_WRITE] = 0;

            print_sinais(Sinais);

            break;


        case 2:

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[REG_DST]     = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[REG_WRITE] = 0;

            print_sinais(Sinais);

            break;

        case 3:

            Sinais[IorD] = 1;

            Sinais[MEM_READ] = 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[MEM_WRITE] = 0;

            Sinais[REG_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            print_sinais(Sinais);

            break;

        case 4:

            Sinais[REG_WRITE] = 1;

            Sinais[MEM_TO_REG] = 1;

            Sinais[REG_DST] = 0;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            print_sinais(Sinais);

            break;

        case 5:

            Sinais[IorD] = 1;

            Sinais[MEM_WRITE] = 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[MEM_READ] = 0;

            Sinais[REG_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            print_sinais(Sinais);

            break;

        case 6:

            Sinais[REG_WRITE] = 1;

            Sinais[MEM_TO_REG]= 0;

            Sinais[REG_DST]= 0;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 1;

            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            print_sinais(Sinais);

            break;


        case 7:

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 0;

            Sinais[REG_DST]= 1;

            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[REG_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            print_sinais(Sinais);

            break;


        case 8:

            Sinais[REG_WRITE] = 1;

            Sinais[MEM_TO_REG] = 0;

            Sinais[REG_DST]= 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 0;

            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;

            print_sinais(Sinais);

            break;


        case 9:

            Sinais[BRANCH] = 1;

            Sinais[ULA_FONTE_A] = 1;

            Sinais[ULA_FONTE_B0]= 0;

            Sinais[ULA_FONTE_B1]= 0;

            Sinais[PC_FONTE0] = 1;

            Sinais[PC_FONTE1] = 0;


            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[REG_WRITE] = 0;

            Sinais[PC_ESC] = 0;

            Sinais[IR_ESC] = 0;


            print_sinais(Sinais);

            break;


        case 10:

            Sinais[PC_ESC]= 1;

            Sinais[PC_FONTE0] = 0;

            Sinais[PC_FONTE1]= 1;

            Sinais[REG_DST]= 1;


            Sinais[MEM_READ] = 0;

            Sinais[MEM_WRITE] = 0;

            Sinais[REG_WRITE] = 0;

            Sinais[BRANCH] = 0;

            Sinais[IR_ESC] = 0;

            print_sinais(Sinais);

            break;

        default:

            printf(" Erro de estado", estado);

            break;

    }
}

int proximo_estado(int estado_atual, unsigned char opcode)
{

    switch (estado_atual)
    {

        case 0:

            return 1;

        case 1:

            if (opcode == 0xB || opcode == 0xF || opcode == 0x4)
            {

                return 2;

            }

            else if (opcode == 0x0)
            {

                return 7;

            }

            else if (opcode == 0x8)
            {

                return 9;

            }

            else if (opcode == 0x2)
            {

                return 10;

            }


            else
            {

                printf(" Opcode desconhecido 0x%X no estado 1\n", opcode);

                return 0;
            }

        case 2:

            if (opcode == 0xB)
            {

                return 3;

            }

            else if (opcode == 0xF)
            {

                return 5;

            }


            else if (opcode == 0x4)
            {

                return 6;

            }

            else
            {
                printf(" Opcode inesperado 0x%X no estado 2\n", opcode);

                return 0;

            }

        case 3:

            return 4;

        case 4:

            return 0;

        case 5:

            return 0;

        case 6:

            return 0;

        case 7:

            return 8;

        case 8:

            return 0;

        case 9:

            return 0;

        case 10:

             return 0;

        default:

            printf("  Erro de estado \n", estado_atual);

            return 0;

    }

}
