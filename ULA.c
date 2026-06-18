#include <stdio.h>
#include <stdlib.h>
#include "head.h"

int ulamx(int Sinal, signed char A, signed char B, signed char immx)
{

    return (Sinal==0) ? B : immx;

}

int ula(int ULAop, signed char A, signed char B, int *overflow, int *zero)
{

    int result = 0;

    *overflow = 0;

    *zero = 0;

    switch(ULAop)
    {

        case 0:

            result = A + B;


            break;

        case 2:

            result = A - B;

            break;

        case 4:

            result = A & B;

            break;

        case 5:

            result = A | B;

            break;

        default:

            result = 0;
    }

    if(result > 127 || result < -128)
    {

        *overflow = 1;

    }

    *zero = (result == 0);


    return result;

}

void Estender(unsigned char imm, signed char *immx)
{

    *immx = (signed char)((imm & 0x20) ? (imm | 0xC0) : imm);

}

int wrmux(int Sinais[16], int result)
{

    return result;

}

void tipo(int ULAop)
{
    switch (ULAop)
    {

        case 0:

             printf("  ULA: add\n");

             break;

        case 2:

             printf("  ULA: sub\n");

             break;

        case 4:

             printf("  ULA: and\n");

             break;

        case 5:

             printf("  ULA: or\n");

             break;

        default:

             printf("  ULA: op %d\n", ULAop);

            break;

    }

}

void Tipo2(unsigned char opcode, int *ULAop)
{
    if(opcode == 0x4 || opcode == 0xB||opcode == 0xF)
    {

        // type I
        *ULAop = 0;

    }


    else if(opcode == 0x8)
    {


        // branch
        *ULAop = 2;

    }

}
