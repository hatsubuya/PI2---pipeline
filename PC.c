#include <stdio.h>
#include <stdlib.h>
#include "Head.h"


unsigned char busca(unsigned char PC)
{

    return PC + 1;

}


unsigned char jump(unsigned char addr)
{

    return addr;

}


unsigned char branch(unsigned char PC, unsigned char imm)
{

    return PC + (signed char)imm;

}

