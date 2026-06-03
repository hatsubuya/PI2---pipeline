#include <stdio.h>
#include <stdlib.h>
#include "Head.h"


int ulamx(int Sinais[16], signed char A, signed char B, signed char immx)
{

    int b0 = Sinais[ULA_FONTE_B0];

    int b1 = Sinais[ULA_FONTE_B1];

    if (b1 == 0 && b0 == 0)
    {

        return (int)B;

    }

    if (b1 == 0 && b0 == 1)
    {

        return 1;

    }

    if (b1 == 1 && b0 == 0)
    {

        return (int)immx;

    }

    return 0;
}

int ula(int ULAop, signed char A, signed char B, int *overflow, int *zero)
{
    int result = 0;

    *overflow  = 0;

    *zero      = 0;

    switch (ULAop)
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

            break;

    }

    if (result > 127 || result < -128)
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

    if (opcode == 0x4 || opcode == 0xB || opcode == 0xF)
    {

        *ULAop = 0;

    }

    else if (opcode == 0x8)
    {

        *ULAop = 2;

    }

}
